<img src="https://raw.githubusercontent.com/rijn/Aincrad/master/res/logo.gif" width="500px" height="250px">

# Aincrad
Application manage and monitor tool designed for distributed system.

## Diagram

![](https://raw.githubusercontent.com/rijn/Aincrad/master/res/diagram.jpg)

Aincrad could:

* Realtime surveillance on multiple servers. track many resources like CPU and memory usage.
* Remotely control server, run commends on bash, terminate processes, restart service, reboot, etc.
* Manage application running on clients, distribute files, easily integrate with version control system like Git.
* Available for many public service and OS.
* Install deamon on clients automaticlly.

## Dependency

```
autoconf >= stable 2.69
automake >= stable 1.15
Boost >= stable 1.6.1
```

## Install

```
# configure on OSX
$ brew install autoconf automake boost
$ autoreconf --install --force
$ mkdir build && cd build
$ ../configure
$ make

# execute main program
$ make main

# run unit text
$ make test
```

## License

MIT