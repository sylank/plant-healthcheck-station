# Debugging

Connect jumper cabels to the RX/TX and use a simple FTDI.

Connect to the FTD.

https://pbxbook.com/other/mac-tty.html
List the available devices:

```
ls /dev/cu.*
```

Use the `screen` to connect:

```
screen /dev/tty.usbserial-0001 19200
```
