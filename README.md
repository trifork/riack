#Riack
Is a C client library for Riak.

##Precompiled
The easiest thing to do is just use the precompiled libraries, from the precompiled folder.
All you need is the shared library and 2 headers and you are good to go, se examples folder
for examples of how to use it.

##Compilation
###Dependencies

####protobuf-c
Pure C bindings for google protocol buffers.
Download at http://code.google.com/p/protobuf-c/

to compile protobuf-c you first need to install Protocol Buffers it here
http://code.google.com/p/protobuf/

On linux and Mac just do a 
```
./configure
make
sudo make install
```
On both protobuf libraries.

On Windows
Protocol Buffers follow the vsprojects/readme.txt in the downloaded the package.
protobuf-c I recommend using cmake however at the time of writing this there is a bug
which will give you a compile error if this is still the case visit 
http://code.google.com/p/protobuf-c/issues/list to get the solution.

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

##Tests
To run the test first input an ip/port in src/CMakeLists.txt line 4 & 5 and rerun cmake
When ready you can simply do a make test.
(on windows just choose the correct build target in Visual Studio)

##Disclamer
This is a sparetime project, so if you so a silly bug don't blame my employer, instead 
make a pull request ;)
