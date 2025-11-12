一、Cmd方式
	开发平台: Windows 11
	开发工具：VS2022 x86
二、命令说明                                                                         
**************************************************************************************************
*                                                                                                    
*                                    Unix-style File System                                          
*                                                                                                    
* [Instructions]:                                                                                    
* [Command]:fformat                      [Description]:Format the file system                       
* [Command]:ls                           [Description]:List contents of the current directory        
* [Command]:mkdir <dirname>              [Description]:Create a new directory 
* [Command]:cd <dirname>                 [Description]:Change directory  
* [Command]:fcreate <filename>           [Description]:Create a new file named filename
* [Command]:fopen <filename>             [Description]:Open a file named filename
* [Command]:fwrite <fd> <infile> <size>  [Description]:Write size bytes from infile to fd
* [Command]:fread <fd> <outfile> <size>  [Description]:Read size bytes from fd to outfile 
* [Command]:fread <fd> std <size>        [Description]:Read size bytes from fd to screen  
* [Command]:fseek <fd> <step> begin      [Description]:Move fd file pointer by step in begin mode 
* [Command]:fseek <fd> <step> cur        [Description]:Move fd file pointer by step in current mode 
* [Command]:fseek <fd> <step> end        [Description]:Move fd file pointer by step in end mode 
* [Command]:fclose <fd>                  [Description]:Close the file with handle fd 
* [Command]:fdelete <filename>           [Description]:Delete the file or folder named filename
* [Command]:exit                         [Description]:Exit system and save cache to disk 
******************************************************************************************************  
三、指令示例
mkdir bin
mkdir etc
mkdir home
mkdir dev

cd home
mkdir texts
mkdir reports
mkdir photos
cd texts
fcreate Readme.txt
fopen Readme.txt
fwrite 8 Readme.txt 2765
fseek 8 0 begin
fread 8 Readmeout.txt 2765
fclose 8

cd ..
cd reports
fcreate Report.docx
fopen Report.docx
fwrite 9 Report.docx 10094
fseek 9 0 begin
fread 9 ReportOut.docx 10094
fclose 9


cd ..
cd photos
fcreate lenna.png
fopen lenna.png
fwrite 10 lenna.jpg 8212
fseek 10 0 begin
fread 10 lennaOUT.png 8212
fclose 10


cd ..
mkdir test
cd test
fcreate Jerry
fopen Jerry
fwrite 12 input.txt 800
fseek 12 500 begin
fread 12 abc.txt 500
fclose 12