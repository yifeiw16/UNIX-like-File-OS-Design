#include "SystemCall.h"
#include "UserCall.h"
#include "Common.h"
#include <iostream>
#include <fstream>
using namespace std;

extern SystemCall mySystemCall;

UserCall::UserCall()  //初始化
{
	userErrorCode = U_NOERROR;  //存放错误码
	systemCall = &mySystemCall;

	dirp = "/";                                   //系统调用参数(一般用于Pathname)的指针
	curDirPath = "/";                             //当前工作目录完整路径
	nowDirINodePointer = systemCall->rootDirINode;//指向当前目录的Inode指针
	paDirINodePointer = NULL;                     //指向父目录的Inode指针
	memset(arg, 0, sizeof(arg));                  //指向核心栈现场保护区中EAX寄存器
}

UserCall::~UserCall()
{

}

//此函数改变Usercall对象的dirp(系统调用参数 一般用于Pathname)数据成员
//只检查文件名是否过长
//检查用户输出的path，规范化他，存在dirp中
bool UserCall::checkPathName(string path)
{
	if (path.empty()) {
		cout << "参数路径为空" << endl;
		return false;
	}

	if (path[0] == '/' || path.substr(0, 2) != "..")
		dirp = path;            //系统调用参数(一般用于Pathname)的指针
	else {
		if (curDirPath.back() != '/')
			curDirPath += '/';
		string pre = curDirPath;//当前工作目录完整路径 cd命令才会改变curDirPath的值
		unsigned int p = 0;
		//可以多重相对路径 .. ../../
		for (; pre.length() > 3 && p < path.length() && path.substr(p, 2) == ".."; ) {
			pre.pop_back();
			pre.erase(pre.find_last_of('/') + 1);
			p += 2;
			p += p < path.length() && path[p] == '/';
		}
		dirp = pre + path.substr(p);
	}

	if (dirp.length() > 1 && dirp.back() == '/')
		dirp.pop_back();

	for (unsigned int p = 0, q = 0; p < dirp.length(); p = q + 1) {
		q = dirp.find('/', p);
		q = Common::min(q, (unsigned int)dirp.length());
		if (q - p > DirectoryEntry::DIRSIZ) {
			cout << "文件名或文件夹名过长" << endl;
			return false;
		}
	}
	return true;
}

//新建文件夹
void UserCall::userMkDir(string dirName)
{
	if (!checkPathName(dirName))
		return;
	arg[1] = INode::IFDIR;//存放当前系统调用参数 文件类型：目录文件
	systemCall->Creat();
	checkError();
}

//列出当前文件夹内的所有内容
void UserCall::userLs()
{
	ls.clear();
	systemCall->Ls();
	if (checkError())
		return;
	cout << ls << endl;
}

void UserCall::userCd(string dirName)
{
	if (!checkPathName(dirName))
		return;
	systemCall->ChDir();
	checkError();
}

void UserCall::__userCd__(string dirName)
{
	if (!checkPathName(dirName))
		return;
	systemCall->ChDir();
}

void UserCall::userCreate(string fileName)
{
	if (!checkPathName(fileName))
		return;
	arg[1] = (INode::IREAD | INode::IWRITE);//存放当前系统调用参数
	systemCall->Creat();
	checkError();
}



void UserCall::userDelete(string fileName)
{
	if (!checkPathName(fileName))
		return;
	systemCall->UnLink();
	checkError();
}

void UserCall::userOpen(string fileName)
{
	if (!checkPathName(fileName))
		return;
	arg[1] = (File::FREAD | File::FWRITE);//存放当前系统调用参数
	systemCall->Open();
	if (checkError())
		return;
	cout << "打开文件成功，返回的文件句柄fd为 " << ar0[UserCall::EAX] << endl;
}


//传入sfd句柄
void UserCall::userClose(string sfd)
{
	arg[0] = stoi(sfd);//存放当前系统调用参数
	systemCall->Close();
	checkError();
}


void UserCall::userSeek(string sfd, string offset, string origin)
{
	arg[0] = stoi(sfd);
	arg[1] = stoi(offset);
	arg[2] = stoi(origin);
	systemCall->Seek();
	checkError();
}

void UserCall::userWrite(string sfd, string inFile, string size)
{
	int fd = stoi(sfd), usize = 0;
	if (size.empty() || (usize = stoi(size)) < 0) {
		cout << "参数必须大于等于零 ! \n";
		return;
	}
	char* buffer = new char[usize];
	fstream fin(inFile, ios::in | ios::binary);
	if (!fin.is_open()) {
		cout << "打开文件" << inFile << "失败" << endl;
		return;
	}
	fin.read(buffer, usize);
	fin.close();


	arg[0] = fd;
	arg[1] = (int)buffer;
	arg[2] = usize;

	//将buffer写入sfd中
	systemCall->Write();

	if (checkError())
		return;
	cout << "成功写入" << ar0[UserCall::EAX] << "字节" << endl;
	delete[] buffer;
}

void UserCall::userRead(string sfd, string outFile, string size)
{
	int fd = stoi(sfd);
	int usize = stoi(size);
	char* buffer = new char[usize];
	arg[0] = fd;
	arg[1] = (int)buffer;
	arg[2] = usize;
	systemCall->Read();
	if (checkError())
		return;

	cout << "成功读出" << ar0[UserCall::EAX] << "字节" << endl;
	if (outFile == "std") {
		for (uint32 i = 0; i < ar0[UserCall::EAX]; ++i)
			cout << (char)buffer[i];
		cout << endl;
		delete[] buffer;
		return;
	}
	else {
		fstream fout(outFile, ios::out | ios::binary);
		if (!fout) {
			cout << "打开文件" << outFile << "失败" << endl;
			return;
		}
		fout.write(buffer, ar0[UserCall::EAX]);
		fout.close();
		delete[] buffer;
		return;
	}
}



bool UserCall::checkError()
{
	if (userErrorCode != U_NOERROR) {
		switch (userErrorCode) {
		case UserCall::U_ENOENT:
			cout << "找不到文件或者文件夹" << endl;
			break;
		case UserCall::U_EBADF:
			cout << "找不到文件句柄" << endl;
			break;
		case UserCall::U_EACCES:
			cout << "权限不足" << endl;
			break;
		case UserCall::U_ENOTDIR:
			cout << "文件夹不存在" << endl;
			break;
		case UserCall::U_ENFILE:
			cout << "文件表溢出" << endl;
			break;
		case UserCall::U_EMFILE:
			cout << "打开文件过多" << endl;
			break;
		case UserCall::U_EFBIG:
			cout << "文件过大" << endl;
			break;
		case UserCall::U_ENOSPC:
			cout << "磁盘空间不足" << endl;
			break;
		default:
			break;
		}

		userErrorCode = U_NOERROR;
		return true;
	}
	return false;
}