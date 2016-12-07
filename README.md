# plumber
Threaded TCP to named pipe

Connects to any TCP server and reads/writes data to and from named pipe (can be used with [sernet](https://github.com/eriknl/sernet)) to stream remote serial ports to VMware.

Set up VMware with a serial port and select `Use socket (named pipe)` input the path of the named pipe that plumber will connect to (eg /tmp/vmware-pipe) and pick `From: Server` and `To: An Application` in the dropdown boxes.
Then invoke plumber as following:

```
plumber /tmp/vmware-pipe <remote host> <remote port>
```
