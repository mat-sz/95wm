# 95wm

Windows 95 inspired X11 window manager. Currently non-functional.

## Dependencies

Boost, Xlib (X11), XCB, Cairo and pthreads are mandatory for the project to build.

## Building

```
./configure
make all
```

## Running (in development)

```
make dev
```

This will open a new Xephyr display (size 1000x1000) with xterm and 95wm running inside of it.

## Special thanks to:

This project uses code from:

1. [NewProggie/Cpp-Project-Template](https://github.com/NewProggie/Cpp-Project-Template/)
2. Chuan Ji's excellent [X11 Window Manager guide](https://jichu4n.com/posts/how-x-window-managers-work-and-how-to-write-one-part-i/)
