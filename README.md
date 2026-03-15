# PROBE

A low-level utility designed for base network scanning and port-service matching with accuracy calculation.

### OVERVIEW
Overall usage is perfomed via a simple CLI interface, the app does not implement any sort graphical interface.
Standard command synposis:

```
$ probe [DOMAIN/IP]
```

### COMPILATION
After cloning the repo on a local machine, ```$ mkdir bin/; make``` command combination in the root directory will
create a directory for the executable build and compile the code. Minimal build requirements include:
* Any standard C compiler, supporting C18
* Standard UNIX C libraries (*arpa/\*, netinet/\*, sys/\*, etc.*)

Please ignore any warnings that your compiler may throw. The code has been tested in multiple environments,
any potential errors are to be improved in the future

### USAGE
Current version of PROBE only supports a scan of a single machine. DNS lookup is supported if the connection to such server
is present. Utility synopsis may be viewed a few sections higher.
