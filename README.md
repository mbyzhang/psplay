# PSPlay

## Prerequisites

### Debian/Ubuntu
```
$ sudo apt install libevent-dev pkg-config cmake build-essential libgtest-dev
```

### macOS

Make sure you have Homebrew and Xcode Command Line Tools installed.

```
$ brew install libevent pkg-config cmake googletest
```

## Building

```
$ git clone git@github.com:mbyzhang/psplay.git
$ cd psplay
$ mkdir build
$ cd build
$ cmake ..
$ make -j4
```
