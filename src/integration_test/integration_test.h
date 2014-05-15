/*
 * kinetic-cpp-client
 * Copyright (C) 2014 Seagate Technology.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef KINETIC_CPP_CLIENT_INTEGRATION_TEST_H_
#define KINETIC_CPP_CLIENT_INTEGRATION_TEST_H_

#include <signal.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <memory>

#include "kinetic/kinetic.h"
#include "mock_callbacks.h"
#include "matchers.h"

namespace kinetic {

using ::testing::StrictMock;
using ::testing::Assign;
using ::testing::Matcher;
using ::testing::MatcherInterface;
using palominolabs::protobufutil::MessageStreamFactory;

using std::shared_ptr;
using std::unique_ptr;
using std::string;

/*
 * IntegrationTest is a test fixture designed to provide functionality to
 * fixtures that derive from it. It facilitates the process of creating a
 * nonblocking connection and executing operations on it until the integration
 * test is complete.
 */
class IntegrationTest : public ::testing::Test {
    protected:
    IntegrationTest() : use_external_(false),
                        pid_(0), done_(false),
                        nonblocking_connection_(nullptr),
                        blocking_connection_(nullptr) {}

    void SetUp() {
        const char *kinetic_path = getenv("KINETIC_PATH");
        use_external_ = kinetic_path != NULL;
        if (use_external_) {
            pid_ = fork();
            ASSERT_NE(-1, pid_);
            if (pid_ == 0) {
                execl(kinetic_path, kinetic_path, NULL);
            }
        }
        ConnectionOptions options;
        options.host = "localhost";
        options.port = 8123;
        options.use_ssl = false;
        options.user_id = 1;
        options.hmac_key = "asdfasdf";

        // Poll the server until it is ready to accept connections
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 50000000;

        const size_t kMaxRetries = 20;
        bool connected = false;

        KineticConnectionFactory connection_factory = kinetic::NewKineticConnectionFactory();

        for (size_t i = 0; i < kMaxRetries; ++i) {
            ASSERT_EQ(0, nanosleep(&sleep_time, NULL));
            if (connection_factory.NewNonblockingConnection(options, nonblocking_connection_).ok()) {
                connected = true;
                blocking_connection_.reset(new BlockingKineticConnection(nonblocking_connection_, 10));
                break;
            }
        }
        ASSERT_TRUE(connected);

        shared_ptr<string> null_ptr(nullptr);
        blocking_connection_->InstantSecureErase(null_ptr);
    }

    void TearDown() {
        if (use_external_) {
            ASSERT_EQ(0, kill(pid_, SIGINT));
            ASSERT_EQ(pid_, waitpid(pid_, NULL, 0));
        }
    }

    // Expect that Success() will be called on the handler, and execute the select loop until the
    // handler completes
    // TODO(marshall) deprecate in favor of shared_ptr version
    template<typename T> void WaitForSuccess(StrictMock<T>* callback) {
        EXPECT_CALL(*callback, Success()).WillOnce(Assign(&done_, true));
        RunSelectLoop();
        done_ = false;
    }

    template<typename T> void WaitForSuccessSharedPtr(shared_ptr<StrictMock<T>> callback) {
        WaitForSuccess(callback.get());
    }


    template<typename T> void WaitForFailure(StrictMock<T>* callback, StatusCode error,
            string message) {
        EXPECT_CALL(*callback, Failure(KineticStatusEq(error, message)))
            .WillOnce(Assign(&done_, true));
        RunSelectLoop();
        done_ = false;
    }

    template<typename T> void WaitForFailure(shared_ptr<StrictMock<T>> callback,
        StatusCode error, string message) {
        WaitForFailure(callback.get(), error, message);
    }

    // Keep running the nonblocking client until *done becomes true.
    void RunSelectLoop() {
        fd_set read_fds, write_fds;
        int nfds;
        // Set a timeout so that failures do not cause the test
        // runner to hang
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        ASSERT_TRUE(nonblocking_connection_->Run(&read_fds, &write_fds, &nfds));
        while (!done_) {
            ASSERT_GT(select(nfds, &read_fds, &write_fds, NULL, &timeout), 0);
            ASSERT_TRUE(nonblocking_connection_->Run(&read_fds, &write_fds, &nfds));
        }
    }

    bool use_external_;
    pid_t pid_;
    bool done_;
    shared_ptr<kinetic::NonblockingKineticConnection> nonblocking_connection_;
    unique_ptr<kinetic::BlockingKineticConnection> blocking_connection_;
};

// when the cap is reached, set the flag bool to true
class Incrementor {
    public:
    Incrementor(uint32_t cap, bool *flag, uint32_t *count): cap_(cap), flag_(flag), count_(count) {}

    void operator()() {
        ASSERT_LT(*count_, cap_);
        (*count_)++;
        if (*count_ == cap_) {
            *flag_ = true;
        }
    }

    private:
    uint32_t cap_;
    bool *flag_;
    uint32_t *count_;
};

class KineticRecordMatcher : public MatcherInterface<KineticRecord*> {
    public:
    KineticRecordMatcher(const std::string& value,
            const std::string& version,
            const std::string& tag,
            Message_Algorithm algorithm)
    : value_(value), version_(version), tag_(tag), algorithm_(algorithm) {}

    virtual bool MatchAndExplain(KineticRecord* other,
            testing::MatchResultListener *listener) const {
        *listener << "value=" << *(other->value()) <<
                    " version=" << *(other->version()) <<
                    " tag=" << *(other->tag()) <<
                    " algorithm=" << other->algorithm();
        return *(other->value()) == value_ &&
                *(other->version()) == version_ &&
                *(other->tag()) == tag_ &&
                other->algorithm() == algorithm_;
    }

    virtual void DescribeTo(::std::ostream *os) const {
        *os << "value=" << value_ <<
                " version=" << version_ <<
                " tag=" << tag_ <<
                " algorithm=" << algorithm_;
    }

    private:
    const std::string value_;
    const std::string version_;
    const std::string tag_;
    Message_Algorithm algorithm_;
};


inline Matcher<KineticRecord*> KineticRecordEq(
        const std::string& value,
        const std::string& version,
        const std::string& tag,
        Message_Algorithm algorithm) {
    return MakeMatcher(new KineticRecordMatcher(value, version, tag, algorithm));
}

}  // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_INTEGRATION_TEST_H_
