# UNIX-like File OS Design

## **Overview**

This project simulates a **UNIX V6++ file system**, providing core file operations while helping users understand file system structure and logic.
A large file (e.g., `D:/myDisk.img`) is used to emulate a virtual disk, organized into 512-byte blocks.


## **Commands**

| Command                       | Description                                                 |
| ----------------------------- | ----------------------------------------------------------- |
| `fformat`                     | Format the file system                                      |
| `ls`                          | List current directory contents                             |
| `mkdir <dirname>`             | Create a directory                                          |
| `cd <dirname>`                | Change directory                                            |
| `fcreate <filename>`          | Create a file                                               |
| `fopen <fd>`                  | Open a file by descriptor                                   |
| `fwrite <fd> <infile> <size>` | Write `<size>` bytes from `<infile>` into file `<fd>`       |
| `fread <fd> <outfile> <size>` | Read `<size>` bytes from `<fd>` to `<outfile>`              |
| `fread <fd> std <size>`       | Read `<size>` bytes from `<fd>` to screen                   |
| `fseek <fd> <step> <i>`       | Move file pointer by `<step>` (relative to `begin/cur/end`) |
| `fclose <fd>`                 | Close the file                                              |
| `fdelete <filename>`          | Delete file or directory                                    |
| `exit`                        | Exit system and save cached data to disk                    |


## **System Calls**

* **UserCall**: Handles input validation and argument passing
* **SystemCall**: Executes system-level operations and manages resources



## **Requirements**

* **Performance**: Read/write operations ≤100ms
* **Data Integrity**: Ensure correct and complete storage
* **User Interface**: Simple CLI, output to screen or file
* **Development**: C++ in Visual Studio 2022, tested on Windows 11

