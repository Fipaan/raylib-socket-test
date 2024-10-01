### Introduction

This is a simple example of using raylib, to communicate between users.
For communicating I'm using ZeroMQ library, because I couldn't figure out
how to use C library for manipulating sockets. Server using localhost, to
change this, just put your ip in "common.h"

---

### Dependecies

#### Raylib: [GitHub](https://github.com/raysan5/raylib "GitHub")

#### ZeroMQ: [GitHub](https://github.com/zeromq/libzmq "GitHub")

---

### Internal dependecies

For some reason I couldn't make fully statically linked executable,
so I have to link to standard libs, such as `libm` and `libc`, so it's mostly single executable.

### How to run?

#### Client:

```console
   ./client
```

###### Note: You can put l or r flag, to align window to the left or right

#### Server:

```console
   ./server
```

---

### Build:

#### Client:

```console
   make client-build
```

#### Server:

```console
   make server-build
```
