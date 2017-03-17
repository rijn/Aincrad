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

```
->> / forward      # push all remaining commands to server
-> / to hostname   # push remaining commands to specific client
system             # run system and push result to vstack
broadcast except   # broadcast command to all clients except specific host
set_hostname name  # set host name
list_host          # list clients
print              # print vstack to standard out
this               # would be altered to hostname
< >                # scope operator. would be removed one level when parsing.
```

Some samples:

* List all clients connected to the server
    ```
    print$->$this$list_host$->>
    ```

* Set hostname
    ```
    set_hostname$clientA
    ```

* Ping client
    ```
    print$Connected$<$<$this$>$>$-$time$->$this$->>$->$clientA$->>$time
    # will return "clientA Connected !"
    ```

* List root dir on clientA
    ```
    print$->$this$->>$system$ls /$->$clientA$->>
    ```

## License

MIT