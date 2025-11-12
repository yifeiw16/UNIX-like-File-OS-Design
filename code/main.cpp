#define _CRT_SECURE_NO_WARNINGS
#include "OpenFileManager.h"
#include "SystemCall.h"
#include "UserCall.h"
#include "DiskDriver.h"
#include "CacheManager.h"
#include <iostream>
#include <unordered_map>
#include <sstream>

using namespace std;

DiskDriver myDiskDriver;
CacheManager myCacheManager;
wyfOpenFileTable mywyfOpenFileTable;
SuperBlock mySuperBlock;
FileSystem myFileSystem;
INodeTable myINodeTable;
SystemCall mySystemCall;
UserCall myUserCall;

int main() {
    UserCall& User = myUserCall;
    cout << "***************************************************************************************\n"
        "*                                                                                     *\n"
        "*                                    Unix-style File System                           *\n"
        "*                                                                                     *\n"
        "* [Instructions]:                                                                     *\n"
        "* [Command]:help <op_name>     [Description]:Display command help                      *\n"
        "* [Command]:test              [Description]:Run automatic tests                        *\n"
        "* [Command]:fformat           [Description]:Format the file system                     *\n"
        "* [Command]:ls                [Description]:List contents of the current directory     *\n"
        "* [Command]:mkdir <dirname>   [Description]:Create a new directory                     *\n"
        "* [Command]:cd <dirname>      [Description]:Change directory                           *\n"
        "* [Command]:fcreate <filename>[Description]:Create a new file named filename           *\n"
        "* [Command]:fopen <filename>  [Description]:Open a file named filename                 *\n"
        "* [Command]:fwrite <fd> <infile> <size>  [Description]:Write size bytes from infile to fd *\n"
        "* [Command]:fread <fd> <outfile> <size>  [Description]:Read size bytes from fd to outfile *\n"
        "* [Command]:fread <fd> std <size>         [Description]:Read size bytes from fd to screen *\n"
        "* [Command]:fseek <fd> <step> begin       [Description]:Move fd file pointer by step in begin mode *\n"
        "* [Command]:fseek <fd> <step> cur         [Description]:Move fd file pointer by step in current mode *\n"
        "* [Command]:fseek <fd> <step> end         [Description]:Move fd file pointer by step in end mode *\n"
        "* [Command]:fclose <fd>                  [Description]:Close the file with handle fd   *\n"
        "* [Command]:fdelete <filename>           [Description]:Delete the file or folder named filename *\n"
        "* [Command]:exit                         [Description]:Exit system and save cache to disk *\n"
        "***************************************************************************************\n";

    string line, cmd, args[3];
    while (true) {
        cout << "[20514540WYF@root " << User.curDirPath << " ]$ ";
        getline(cin, line);
        if (line.empty())
            continue;

        stringstream ss(line);
        ss >> cmd;
        args[0] = args[1] = args[2] = "";

        if (cmd == "fformat") {
            mywyfOpenFileTable.Reset();
            myINodeTable.Reset();
            myCacheManager.FormatBuffer();
            myFileSystem.FormatDevice();
            cout << "File system formatted. Please restart!" << endl;
            return 0;
        }
        else if (cmd == "ls") {
            User.userLs();
        }
        else if (cmd == "mkdir") {
            ss >> args[0];
            if (args[0][0] != '/')
                args[0] = User.curDirPath + args[0];
            User.userMkDir(args[0]);
        }
        else if (cmd == "cd") {
            ss >> args[0];
            User.userCd(args[0]);
        }
        else if (cmd == "fcreate") {
            ss >> args[0];
            if (args[0][0] != '/')
                args[0] = User.curDirPath + args[0];
            User.userCreate(args[0]);
        }
        else if (cmd == "fopen") {
            ss >> args[0];
            if (args[0][0] != '/')
                args[0] = User.curDirPath + args[0];
            User.userOpen(args[0]);
        }
        else if (cmd == "exit") {
            return 0;
        }
        else if (cmd == "fclose") {
            ss >> args[0];
            User.userClose(args[0]);
        }
        else if (cmd == "fseek") {
            ss >> args[0] >> args[1] >> args[2];
            User.userSeek(args[0], args[1], args[2]);
        }
        else if (cmd == "fread") {
            ss >> args[0] >> args[1] >> args[2];
            User.userRead(args[0], args[1], args[2]);
        }
        else if (cmd == "fwrite") {
            ss >> args[0] >> args[1] >> args[2];
            User.userWrite(args[0], args[1], args[2]);
        }
        else if (cmd == "fdelete") {
            ss >> args[0];
            if (args[0][0] != '/')
                args[0] = User.curDirPath + args[0];
            User.userDelete(args[0]);
        }
    }
    return 0;
}
