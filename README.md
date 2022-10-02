# PSPlay

## Prerequisites

### Debian/Ubuntu
```
$ sudo apt install pkg-config cmake build-essential libgtest-dev libsndfile1-dev
```

### macOS

Make sure you have Homebrew and Xcode Command Line Tools installed.

```
$ brew install pkg-config cmake googletest libsndfile
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
