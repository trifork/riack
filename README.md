#Riack
Is a C client library for Riak.

##Compilation
###Dependencies

####cmake
Riack uses cmake build system which means it can be compiled on most systems.
Make sure you have installed cmake if not find it here http://www.cmake.org/ or
if your fortunate enough to be an OS with a package manager just install it with that.

####wolfSSL (optional)
Riack uses wolfSSL to support TLS/SSL connections and authentication. Tested with wolfSSL 3.9.0. Other versions will most likely work fine. Find it here https://github.com/wolfSSL/wolfssl

ECC (Elliptic Curve Cryptography) is currently not supported.

###Ready
Get a prompt and move to Riack top folder and do
```
cmake src/
```
Or to build shared libs with tls/ssl support do
```
cmake -DBUILD_SHARED_LIBS=1 -DWITH_WOLFSSL=1 src/
```
This will generate make files, and you can run a make afterwards, unless your on windows
in which case I recommend generating a visual studio project this is done like this:

```
cmake src/ -G "Visual Studio 10"
```
Note on windows you might need to tell cmake where to find the Protobuf-C files
You can do this by passing some options to cmake which is hard to remember ;) I recommend 
to just edit src\cmake\Modules\FindProtoBufC.cmake lines 19 & 20.

##Examples

Connect to Riak and ping it
```c
#include <riack.h>

riack_init();
riack_client *client = riack_new_client(0);
riack_connect(client, "127.0.0.1", 8087, 0);

if (riack_ping(client) == RIACK_SUCCESS)
    printf("pong");
}

riack_free(client);
riack_cleanup();
```
Connect to Riak securely using username/password and ping it
```c
#include <riack.h>

riack_init();
riack_security_options security;
riack_init_security_options(&security);
riack_client *client = riack_new_client(0);
riack_connect(client, "127.0.0.1", 8087, 0);

riack_start_tls(client, &security);
riack_string user;
riack_string pw;
user.value = "riakuser";
user.len = strlen(user.value);
pw.value = "pass";
pw.len = strlen(pw.value);
riack_auth(client, &user, &pw);

if (riack_ping(client) == RIACK_SUCCESS)
    printf("pong");
}

riack_free(client);
riack_cleanup();
```
Connect to Riak securely using certificates and ping it
```c
#include <riack.h>

riack_init();
riack_security_options security;
riack_init_security_options(&security);
riack_client *client = riack_new_client(0);
riack_connect(client, "127.0.0.1", 8087, 0);

security.ca_file = "/path/to/cacert.crt";
security.cert_file = "/path/to/client.crt";
security.key_file = "/path/to/client.key";
riack_start_tls(client, &security);
riack_string user;
user.value = "riakuser";
user.len = strlen(user.value);
riack_auth(client, &user, 0);

if (riack_ping(client) == RIACK_SUCCESS)
    printf("pong");
}

riack_free(client);
riack_cleanup();
```
To see more examples of this look in the examples directory.
Before the examples can run you must place the compiled library files in the precompiled folder (see the precompiled/README.md file for details).

##Tests
To make all tests succeed you need a running riak server with eleveldb backend and riak search enabled in app.config.
You also need to have seach enabled on the ´testsearch´ bucket, this can be done using the riak search-cmd like this:
```
search-cmd install testsearch
```
Futhermore an active bucket type called ´riack_bt_test´ needs to exist, can be done like this:  
```
riak-admin bucket-type create riack_bt_test
riak-admin bucket-type activate riack_bt_test
```

If your server is running on localhost with port 8087 set as protocol buffer port, you can run it right away, if not you need to input the ip and port in src/CMakeLists.txt line 4 & 5 and rerun cmake
When ready you can simply do a make test.
(on windows just choose the correct build target in Visual Studio)

##Built with Riack

###php_riak
PHP extension featuring persistent connection, autoreconnect and a PHP session module.
https://github.com/TriKaspar/php_riak

###Riack++
A C++ wrapper can be found here https://github.com/TriKaspar/riack_cpp
It does not require anything but riack and a C++ compiler.

##Disclamer
This is a sparetime project, so if you so a silly bug don't blame my employer, instead 
make a pull request ;)  

