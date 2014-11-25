#Riack
Is a C client library for Riak.

##Compilation
###Dependencies

####cmake
Riack uses cmake build system which means it can be compiled on most systems.
Make sure you have installed cmake if not find it here http://www.cmake.org/ or
if your fortunate enough to be an OS with a package manager just install it with that.

###Ready
Get a prompt and move to Riack top folder and do
```
cmake src/
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
To se examples of this look in the examples directory.  
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

