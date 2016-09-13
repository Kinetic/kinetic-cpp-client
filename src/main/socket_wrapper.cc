/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 * 
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without 
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public 
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <exception>
#include <stdexcept>
#include "glog/logging.h"
#include "socket_wrapper.h"

#ifdef USE_PTHREAD_LOCKS
#include <pthread.h>
namespace {
    pthread_mutex_t* mutex_buffer = NULL;

    void pthread_mutex_funs(int mode, int index, const char* file, int line)
    {
        if (mode & CRYPTO_LOCK) {
            pthread_mutex_lock(&mutex_buffer[index]);
        } else {
            pthread_mutex_unlock(&mutex_buffer[index]);
        }
    }

    unsigned long pthread_id_fun()
    {
        return (unsigned long) pthread_self();
    }

    void SSL_register_locks()
    {
        const int num_locks = CRYPTO_num_locks();
        mutex_buffer = (pthread_mutex_t*) malloc(num_locks * sizeof(pthread_mutex_t));
        if (!mutex_buffer) {
            LOG(ERROR) << "Failed allocating memory for OpenSSL pthread locks.";
            return;
        }
        for (int i = 0; i < num_locks; i++) {
            pthread_mutex_init(&mutex_buffer[i], NULL);
        }
        CRYPTO_set_id_callback(pthread_id_fun);
        CRYPTO_set_locking_callback(pthread_mutex_funs);
    }

    void SSL_free_locks()
    {
        if (mutex_buffer) {
            CRYPTO_set_id_callback(NULL);
            CRYPTO_set_locking_callback(NULL);
            for(int i=0; i<CRYPTO_num_locks(); i++){
                pthread_mutex_destroy(&mutex_buffer[i]);
            }
            free(mutex_buffer);
            mutex_buffer = NULL;
        }
    }
}
#else
namespace {
    void SSL_register_locks()
    {
        LOG(INFO) << "No locks configured for OpenSSL. Do so yourself if you require thread-safety.";
    }
    void SSL_free_locks() {}
}
#endif


namespace kinetic {

using std::string;

class OpenSSLInitializer
{
public:
  OpenSSLInitializer()
  {
    SSL_library_init();
    SSL_register_locks();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
  }

  ~OpenSSLInitializer()
  {
    SSL_free_locks();
  }
};

static OpenSSLInitializer init;

SocketWrapper::SocketWrapper(const std::string& host, int port, bool use_ssl, bool nonblocking)
        : ctx_(NULL), ssl_(NULL), host_(host), port_(port), nonblocking_(nonblocking), fd_(-1) {
    if(use_ssl) {
        ctx_ = SSL_CTX_new(SSLv23_client_method());
        ssl_ = SSL_new(ctx_);
        if(!ssl_ || !ctx_) {
            throw std::runtime_error("Failed Setting up SSL environment.");
        }
        SSL_set_mode(ssl_, SSL_MODE_AUTO_RETRY);
    }
}

SocketWrapper::~SocketWrapper() {
    if (fd_ == -1) {
        LOG(INFO) << "Not connected so no cleanup needed";
    } else {
        LOG(INFO) << "Closing socket with fd " << fd_;
        if (close(fd_)) {
            PLOG(ERROR) << "Error closing socket fd " << fd_;
        }
    }
    if(ssl_) SSL_free(ssl_);
    if(ctx_) SSL_CTX_free(ctx_);
}

bool SocketWrapper::Connect() {
    LOG(INFO) << "Connecting to " << host_ << ":" << port_;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));

    // could be inet or inet6
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICSERV;

    struct addrinfo* result;

    string port_str = std::to_string(static_cast<long long>(port_));

    if (int res = getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &result) != 0) {
        LOG(ERROR) << "Could not resolve host " << host_ << " port " << port_ << ": "
                << gai_strerror(res);
        return false;
    }

    struct addrinfo* ai;
    int socket_fd;
    for (ai = result; ai != NULL; ai = ai->ai_next) {
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        if (int res = getnameinfo(ai->ai_addr, ai->ai_addrlen, host, sizeof(host), service,
                sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
            LOG(ERROR) << "Could not get name info: " << gai_strerror(res);
            continue;
        } else {
            LOG(INFO) << "Trying to connect to " << string(host) << " on " << string(service);
        }

        socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (socket_fd == -1) {
            LOG(WARNING) << "Could not create socket";
            continue;
        }

        // os x won't let us set close-on-exec when creating the socket, so set it separately
        int current_fd_flags = fcntl(socket_fd, F_GETFD);
        if (current_fd_flags == -1) {
            PLOG(ERROR) << "Failed to get socket fd flags";
            close(socket_fd);
            continue;
        }
        if (fcntl(socket_fd, F_SETFD, current_fd_flags | FD_CLOEXEC) == -1) {
            PLOG(ERROR) << "Failed to set socket close-on-exit";
            close(socket_fd);
            continue;
        }

        // On BSD-like systems we can set SO_NOSIGPIPE on the socket to prevent it from sending a
        // PIPE signal and bringing down the whole application if the server closes the socket
        // forcibly
#ifdef SO_NOSIGPIPE
        int set = 1;
        int setsockopt_result = setsockopt(socket_fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
        // Allow ENOTSOCK because it allows tests to use pipes instead of real sockets
        if (setsockopt_result != 0 && setsockopt_result != ENOTSOCK) {
            PLOG(ERROR) << "Failed to set SO_NOSIGPIPE on socket";
            close(socket_fd);
            continue;
        }
#endif

        if (connect(socket_fd, ai->ai_addr, ai->ai_addrlen) == -1) {
            PLOG(WARNING) << "Unable to connect";
            close(socket_fd);
            continue;
        }

        if (nonblocking_ && fcntl(socket_fd, F_SETFL, O_NONBLOCK) != 0) {
            PLOG(ERROR) << "Failed to set socket nonblocking";
            close(socket_fd);
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ai == NULL) {
        // we went through all addresses without finding one we could bind to
        LOG(ERROR) << "Could not connect to " << host_ << " on port " << port_;
        return false;
    }

    fd_ = socket_fd;
    if(ssl_) return ConnectSSL();
    return true;
}

#include <openssl/err.h>

bool SocketWrapper::ConnectSSL()
{
    SSL_set_fd(ssl_,fd_);
    int rtn = SSL_connect(ssl_);
    if(rtn == 1)
        return true;

    int err = SSL_get_error(ssl_, rtn);
    if( err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE ){
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds); FD_ZERO(&write_fds);
        if(err == SSL_ERROR_WANT_READ)  FD_SET(fd_, &read_fds);
        if(err == SSL_ERROR_WANT_WRITE) FD_SET(fd_, &write_fds);
        struct timeval tv = {1,1};
        select(fd_+1, &read_fds, &write_fds, NULL, &tv);
        return ConnectSSL();
    }
    return false;
}

SSL * SocketWrapper::getSSL(){
    return ssl_;
}

int SocketWrapper::fd() {
    return fd_;
}

}  // namespace kinetic
