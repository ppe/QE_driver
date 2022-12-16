## Driver template in C for Sinclair QL QDOS and Minerva

This is a simple sample or template for a device driver for [Sinclair QL](https://qlwiki.qlforum.co.uk/doku.php?id=qlwiki:sinclair_ql_home_computer) [QDOS operating system](http://qdosmsq.dunbar-it.co.uk/doku.php?id=qdosmsq:start&rev=1313651331#qdosmsq) written in C. I created this because I couldn't find material on the web about writing QDOS device drivers in C. And not much material about writing one in Assembler, either. There was a lot of trial and error and debugging wihh the [Q-Emulator](http://www.terdina.net/ql/q-emulator.html) QL Emulator involved in making even this simple example work properly both on QDOS and Minerva, which is an alternative implementation of the operating system for the QL.

The other reason for creating this template was that I wanted to switch to using qdos-gcc for compiling. Qdos-gcc is a special port of the GCC C compiler for QDOS with all the associated OS-specific libraries ported from C68. Unfortunately Qdos-gcc ws created for a really old release of GCC (2.95) and hasn't been updated since. It is hard (impossible?) to make it run on a modern Linux distro. Fortunately, XorA has created a [handy docker container](https://hub.docker.com/r/xora/qdos-gcc) that bundles the compiler into a usable package.

### Functionality
The driver is an echo generator. You can send a string into it and then read the same string back either a character or a line at a time. An example session in SuperBasic (QL's version of BASIC):
```
    open #3,echo_
    print #3,"Foo42"
    input #3,a$
    print a$: rem prints "Foo42"
    print #3,"Test"
    print inkey$(#3,0): rem prints "T"
    close #3
```

### Dependencies
* Docker
* qdos-gcc Docker container

There are instructions for the container on the QL Forum in [this thread.](https://qlforum.co.uk/viewtopic.php?t=2105)
To get started, pull the container image:
```bash
    docker pull xora/qdos-gcc
```

### Compiling
* Pull the code and change into the code directory
* Open a shell into the container:
```bash
    docker run -it -v $PWD:/build xora/qdos-gcc /bin/bash
```
* Currently, the container image does not have `/usr/local/qdos/bin` in default PATH so might need to add it manually.
  * Check that the `ld` command comes from the correct location by running `which ld` which should result in `/usr/local/qdos/bin/ld`
  * If `ld` comes from the wrong path it is the GNU ld which we **do not** want. You can add the correct path temporarily by: `export PATH=/usr/local/qdos/bin:$PATH`
* `make` builds the binary file `echodrv_bin`
* `make clean` cleans the output files

### Using on a QL (emulator or real)
```
a=alchp(3000):lbytes flp1_echodrv_bin:call a
```
or with Toolkit II
```
lrespr flp1_echodrv_bin
```

### Resources
[QL Technical Guide](http://www.dilwyn.me.uk/docs/manuals/qltm.pdf) by Tony Tebby and David Karlin available from [Dilwyn Jones's wonderful site](http://www.dilwyn.me.uk/)
Book: **The Sinclair QDOS Companion** by Andrew Pennell, with luck, you can find a copy on Ebay. _ISBN_ 0-946408-69-6
[Norman Dunbar's excellent QDOS/SMS internals wiki](http://qdosmsq.dunbar-it.co.uk/doku.php?id=qdosmsq:start&rev=1313651331#qdosmsq)
[GCC Extended ASM syntax](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
