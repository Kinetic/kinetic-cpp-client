Quickstart {#mainpage}
======================
This library allows building C++ programs that interact with a Kinetic API implementation. It provides blocking and non-blocking styles of interaction depending on application needs.

Including the library
---------------------
The library can be built as a static or shared library. For examples of how to include the library in a CMake-based project see the examples repo at [https://github.com/Seagate/kinetic-cpp-examples](https://github.com/Seagate/kinetic-cpp-examples). Applications also need to add the `include/kinetic` directory to the include path and include `kinetic/kinetic.h`.

Opening a connection
--------------------
Before creating a connection, first create a `KineticConnectionFactory` using the handy helper method:

    kinetic::KineticConnectionFactory kinetic_connection_factory = kinetic::NewKineticConnectionFactory();

A single `KineticConnectionFactory` can be used to create many connections. Deleting the connection factory won't affect existing connections.

Next, create a connection options object to indicate the IP, port, user identity, and so on:

    kinetic::ConnectionOptions options;
    options.host = my_host;
    options.port = my_port;
    options.user_id = my_user_id;
    options.hmac_key = my_hmac_key;

Finally, open the connection:

    std::unique_ptr<kinetic::NonblockingKineticConnection> connection;
    kinetic::KineticStatus status = kinetic_connection_factory.NewNonblockingConnection(options, timeout_in_seconds, connection);

*Note*: If the connection needs to be shared between threads, call `NewThreadsafeNonblockingConnection` instead.

To check whether the connection succeeded, check the value of `status.ok()`. If it's false, additional error information is available by calling `status.statusCode()` and `status.message()`.

The factory can create threadasafe and non-threadsafe variants of blocking and nonblocking clients. Blocking clients can also be created from an existing nonblocking client using the `BlockingKineticClient` constructor which takes a `NonblockingKineticConnection`.
The benefit of this approach is that both clients will use the same underyling connection. This is not necessarily threadsafe. If you use a nonblocking and blocking client which use the same underlying connection, then there are no guarantees about which thread is will process callbacks.

Performing blocking GET/PUT operations
--------------------------------------
PUT and GET calls make use of the `kientic::KineticRecord` class, which bundles the value/version/tag/algorithm fourpule allows construction via a series of `shared_ptr` instances (avoiding a copy) or a series of `string`s (avoiding extra typing).

To PUT a key:
    
    blocking_connection->Put(
        key,
        version,
        kinetic::IGNORE_VERSION,
        KineticRecord(value, version, tag, Message_Algorithm_SHA1));

Like most operations in the Kinetic C++ client, `Put` returns a `KineticStatus` object that indicates success and provides error information.

*Note*: The Kinetic C++ client makes extensive use of `shared_ptr` and `unique_ptr` to make memory transfer and ownership explicit while avoiding unnecessary copies. However, applications that prefer to avoid the complexity of `shared_ptr` and `unique_ptr` can just pass in `std::string` instances and accept the additional overhead.

GETting a key works similarly:

    std::unique_ptr<KineticRecord> record;
    blocking_connection->Get(key, record);

Performing non-blocking GET/PUT operations
------------------------------------------
The blocking API is convenient for simplicity or thread-per-connection style designs, but many applications use a non-blocking event-driven design. For these applications, it makes more sense to use a non-blocking API. The Kinetic C++ client's non-blocking API is very similar to the blocking API, with two important differences. First, all methods take an additional callback parameter that will be called once the operation succeeds or fails. Secondly, to actually perform IO the `Run` method needs to be called repeatedly.

As a simple example to perform several writes, first create a callback
class to receive success and failure messages:

    // PutCallback definition:
    class PutCallback : public PutCallbackInterface {
    public:
        void Success() {
            printf("Success!\n");
        }
        void Failure(KineticStatus error) {
            printf("Dismal failure!\n");
        }
    };
    //
    // ...
    //
    // PutCallback instantiation
    auto callback = make_shared<PutCallback>();

Then enqueue a series of operations:

    auto record = make_shared<KineticRecord>(value, version, tag, Message_Algorithm_SHA1);
        nonblocking_connection->Put(key1, version, kinetic::IGNORE_VERSION, record, callback);
    Message_Algorithm_SHA1);
        nonblocking_connection->Put(key2, version, kinetic::IGNORE_VERSION, record, callback);
    Message_Algorithm_SHA1);
        nonblocking_connection->Put(key3, version, kinetic::IGNORE_VERSION, record, callback);

Finally, call `Run` repeatedly to actually execute the operations:
    
    fd_set read_fds, write_fds;
    int nfd = 0;
    nonblocking_connection->Run(&read_fds, &write_fds, &nfd);
    while (there_is_work_to_do) {
        while (select(nfd + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
        nonblocking_connection->.Run(&read_fds, &write_fds, &num_fds);
    }

If desired, other fds can be added to the `fd_set`s so that the `select` call can wait for IO to be ready on fds controlled by the Kinetic API or other parts of the application.

GETs work very similarly. First, a callback implementation:

    class GetCallback : public GetCallbackInterface {
    public:
        void Success(const std::string &key, std::unique_ptr<KineticRecord> record) {
            printf("Good\n");
        }
        void Failure(KineticStatus error) {
            printf("Bad\n");
        }
    };
    //
    // ...
    //
    // GetCallback instantiation
    auto callback = make_shared<GetCallback>();

Next, enqueue some operations:

        nonblocking_connection->Get(key_1, callback);
        nonblocking_connection->Get(key_2, callback);
        nonblocking_connection->Get(key_3, callback);

The `Run` loop works exactly the same way as it does for the PUT case. Multiple GET/PUT/management operations can all be enqueued and they will be executed one at a time in order by repeated `Run` calls.
