# PSPlay

## Prerequisites

### Debian/Ubuntu
```
$ sudo apt install libevent-dev pkg-config cmake build-essential libgtest-dev libsndfile1-dev
```

### macOS

Make sure you have Homebrew and Xcode Command Line Tools installed.

```
$ brew install libevent pkg-config cmake googletest sndfile
```

### Any platform

`PSPlay` depends on `libcorrect` as well.

```
$ git clone git@github.com:quiet/libcorrect.git
$ cd libcorrect
$ mkdir build
$ cd build
$ cmake ..
$ make -j4
$ sudo make install
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
