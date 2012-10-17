Redis on Windows prototype
===
## What's new in this release

- Based on Redis 2.4.11
- Removed dependency on the pthreads library
- Improved the snapshotting (save on disk) algorithm. Implemented Copy-On-Write at the application level so snapshotting behavior is similar to the Linux version.

===
Special thanks to Dušan Majkic (https://github.com/dmajkic, https://github.com/dmajkic/redis/) for his project on GitHub that gave us the opportunity to quickly learn some on the intricacies of Redis code. His project also helped us to build our prototype quickly.

## Repo branches
- 2.4: save in foreground
- bksave: background save where we write the data to buffers first, then save to disk on a background thread. It is much faster than saving directly to disk, but it uses more memory. 
- bksavecow: Copy On Write at the application level

## How to build Redis using Visual Studio

You can use the free Express Edition available at http://www.microsoft.com/visualstudio/en-us/products/2010-editions/visual-cpp-express.

- The new application-level Copy On Write code is on a separate branch so before compiling you need to switch to the new branch:
<pre><code>git checkout bksavecow</code></pre>
    

- Open the solution file msvs\redisserver.sln in Visual Studio 10, and build.

    This should create the following executables in the msvs\$(Configuration) folder:

    - redis-server.exe
    - redis-benchmark.exe
    - redis-cli.exe
    - redis-check-dump.exe
    - redis-check-aof.exe


### Release Notes

This is a pre-release version of the software and is not yet fully tested, because of that we are keeping the Fork/COW code on a separate branch until testing is completed.  
This is intended to be a 32bit release only. No work has been done in order to produce a 64bit version of Redis on Windows.
To run the test suite requires some manual work:

- The tests assume that the binaries are in the src folder, so you need to copy the binaries from the msvs folder to src. 
- The tests make use of TCL. This must be installed separately.
- To run the tests you need to have a Unix command line shell installed. To run the test: `tclsh8.5.exe tests/test_helper.tcl`. If a Unix command shell is not installed you may see the following error: “couldn't execute "cat": no such file or directory”.

### Plan for the next release

- Improve test coverage
- Fix some performance issues on the Copy On Write code
- Add 64bit support


 