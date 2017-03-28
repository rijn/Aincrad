<img src="https://raw.githubusercontent.com/rijn/Aincrad/master/res/logo.gif" width="500px" height="250px">

# Aincrad

Application manage and monitor tool designed for distributed system.

## Diagram

![](https://raw.githubusercontent.com/rijn/Aincrad/master/res/diagram.jpg)

Aincrad could:

* Real-time surveillance on multiple servers. Track many resources like CPU and memory usage.
* Remotely control server, run commands on bash, terminate processes, restart service, reboot, etc.
* Manage application running on clients, distribute files, easily integrate with version control system like Git.
* Available for many public service and OS.
* Install daemon on clients automatically.

## Dependency

```
Boost >= stable 1.6.1
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
< >                    # scope operator. would be removed one level when parsing.

dup                    # duplicate the top element of vstack
swap                   # duplicate the top element of vstack
print                  # print vstack to standard out

-                      # minus
+                      # add
>                      # greater, if false will push a 0 into vstack
==                     # equal, if false will push a 0 into vstack

if (else, then)        # if

->> / forward          # push all remaining commands to server
-> / to hostname       # push remaining commands to specific client
system                 # run system and push result to vstack
time                   # push current time into vstack
broadcast except       # broadcast command to all clients except specific host
set_hostname name      # set host name
list_host              # list clients
push_host              # push clients into vstack

tree dir               # push relative path into vstack recursively
sf / sendfile path     # send file / send file to
sft path hostname
popfs path             # pop one file from fstack and save to path

this                   # would be altered to hostname
```

Some samples:

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
    set_hostname$clientA
    ```

* Broadcast message to all client except self
    ```
    print$message$broadcast$this$->>
    ```

* Ping client
    ```
    print$Connected$<$<$this$>$>$ns$-$time$->$this$->>$->$clientA$->>$time
    # will return "xxx ns clientA Connected !"
    ```

* List root dir on clientA
    ```
    print$->$this$->>$system$ls /$->$clientA$->>
    ```

* Transmit file to server and save to same relative path
    ```
    print$ns$-$time$->$this$popfs$->>$sf$dup$TESTFILE$time$finished in
    ```

* Transmit file to specific client
    ```
    print$ns$-$time$->$this$->>$popfs$->$sft$temp$dup$clientA$popfs$temp$->>$sf$dup$TESTFILE$time$finished in
    ```

## Syntactic sugar

All syntactic sugar starts with `@`. Remaining arguments after parsing a syntactic sugar will not be processed.

```
@list_host                  # list hosts connect to server
@ping client                # ping client
```

## License

MIT