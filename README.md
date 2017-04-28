<img src="https://raw.githubusercontent.com/rijn/Aincrad/master/res/logo.gif" width="500px" height="250px">

# Aincrad

Application manage and monitor tool designed for distributed system.

## Features

<!-- ![](https://raw.githubusercontent.com/rijn/Aincrad/master/res/diagram.jpg) -->

* Real-time surveillance on multiple servers. Track many resources like CPU and memory usage.
* Remotely control server, run commands on bash, terminate processes, restart service, reboot, etc.
* Manage application running on clients, distribute files, easily integrate with version control system like Git.
* Available for many public service and OS.
* Install daemon on clients automatically.

## Dependency

```
Boost >= stable 1.5.7
ncurses
```

## Install

```
# compile
$ make all
```

## Command

It's like stack programming language [Forth](https://en.wikipedia.org/wiki/Forth_(programming_language)), but it's a cross-host one, i.e., cstack and vstack will keep the same over multiple hosts. Everything you typed in to the cin will be pushed into call stack and pass to the interpreter.

Generally, your command will like

```
command_n(stack_bottom)$command_n-1$...$command1
```

And the interpreter will start with `command1`. If current command is not a build-in command, it would be thought as a variable and be pushed into vstack.

File stack `fstack` is special. Since files are often large, we cannot sync `fstack` over machines. `fstack` would like a temporary dictionary saving received data. We implemented some commands for you to access these files. For example, you can use `sf`/`sft` to save local file into `fstack` of remote machine, and run `popfs` command in remote machine to move files.

Here are list of build-in commands.

```
$                      # Delimiter
[ ]                    # scope operator. would be removed one level when parsing.

dup                    # duplicate the top element of vstack
swap                   # duplicate the top element of vstack
size                   # return current size of vstack
print                  # print vstack to standard out
print_limit n          # limit print
drop n                 # pop n elements from vstack
drop_one               # pop one elements

-                      # minus
+                      # add
>                      # greater, if false will push a 0 into vstack
==                     # equal, if false will push a 0 into vstack
lwc / upc              # lower and upper case
split                  # split(delim, target) split the string
parse                  # split the commands and push into command stack (astack)

if (else, then)        # if
begin (end)            # loop
exit                   # exit current loop

->> / forward          # push all remaining commands to server
-> / to hostname       # push remaining commands to specific client
system                 # run system and push result to vstack
time                   # push current time into vstack
broadcast except       # broadcast command to all clients except specific host
set_hostname name      # set host name
list_host              # list clients
push_host              # push clients into vstack
promise                # synchronize the commands flow on different ends

tree dir               # push relative path into vstack recursively
sf / sendfile path     # send file / send file to
sft path hostname
popfs path             # pop one file from fstack and save to path

this                   # would be altered to hostname
```

Some demo:

* List all clients connected to the server
    ```
    print$->$this$list_host$->>
    ```

    Interpreting process:

    * client replace every `this` which is not in scope operator to hostname.
    * client execute `->>`, send all remaining parts `print$->$this_hostname$list_host` to server.
    * server execute `list_host`, put host list to a string and push to vstack.
    * server execute `this_hostname`, treat as a variable, push to vstack.
    * server execute `->`, pop one element from vstack, send all the things to this host.
    * client execute `print`, print all the variables in vstack, which is list_host.

* Set hostname
    ```
    set_hostname clientA
    ```

* Broadcast message to all client except self
    ```
    print message broadcast this ->>
    ```

* Ping client
    ```
    print$Connected$[$[$this$]$]$ns$-$swap$time$->$this$->>$->$clientA$->>$time
    # will return "xxx ns clientA Connected !"
    ```

* List root dir on clientA
    ```
    print$->$this$->>$system$ls /$->$clientA$->>
    ```

* Transmit file to server and save to same relative path
    ```
    print$ns$-$swap$time$->$this$popfs$->>$sf$dup$TESTFILE$time$finished in
    ```

* Transmit file to specific client
    ```
    print$ns$-$swap$time$->$this$->>$popfs$->$sft$temp$dup$clientA$popfs$temp$->>$sf$dup$TESTFILE$time$finished in
    ```

* Transmit client src/ to server temp/ recursively
    ```
    print$ns$swap$finished in$-$swap$time$end$then$exit$else$->$this$popfs$++$temp/$->>$sf$++$src/$dup$if$>$1$size$begin$tree$src$time
    ```

* Transmit server /src to localhost /temp
    ```
    print$ns$swap$finished in$-$swap$time$->$this$end$then$exit$else$->>$popfs$++$temp/$->$this$sft$swap$this$++$src/$dup$if$>$1$size$begin$tree$src$->>$time
    ```

* Ping every clients
    ```
    end$then$exit$else$drop$3$print_limit$3$ns$-$swap$time$swap$->$this$->>$->$dup$swap$->>$time$if$>$0$size$begin$->$this$push_host$->>
    ```

## Script

Aincrad allows script running with parameters and return value. The script files are stored under `scripst` subdirectory and can be called using `run` command by filename directally. Here are some scripts we provided.

```
run all command           # get an command from vstack and run it at every clients
run ping [client|server]  # ping particular client or ping server
send client dir           # send dir to another client
```

## Syntactic sugar

All syntactic sugar starts with `@`. Remaining arguments after parsing a syntactic sugar will not be processed.

```
@list_host                  # list hosts connect to server
```

## Source code

```
.
├── LICENSE                         # license
├── Makefile
├── README.md
├── asio_patch                      # patch for Boost on osx
│   ├── fenced_block.hpp
│   └── std_fenced_block.hpp
├── build-scripts                   # Script for make
│   ├── tags.mk
│   ├── ycm_extra_conf_template.py
│   └── ycm_flags.mk
├── install                         # install script for prerequsites
│   ├── centos6.sh
│   └── ubuntu.sh
├── scripts                         # build-in scripts
│   ├── all
│   ├── ping
│   ├── pm2
│   └── send
└── src                             # source code
    ├── aincrad.cpp                 # main
    ├── aincrad.h
    └── lib
        ├── arguments.cpp           # arguments
        ├── arguments.h
        ├── client.cpp              # client
        ├── client.hpp
        ├── command.cpp             # interpreter
        ├── command.hpp
        ├── config.cpp              # config file
        ├── config.h
        ├── editor.cpp              # editor, ncurses part
        ├── editor.h
        ├── optionparser.h
        ├── package.cpp             # packet
        ├── package.hpp
        ├── server.cpp              # server
        ├── server.hpp
        ├── util.cpp                # utilities
        ├── util.h
        ├── window.cpp              # ncurses part
        └── window.h
```

## Presentation

[Slides](https://docs.google.com/presentation/d/1bVHTvuGpOiCniG19hEi2MQ-gNvQL2rrOo130Rr9W-so/edit?usp=sharing)

## License

MIT
