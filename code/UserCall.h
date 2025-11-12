

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "SystemCall.h"
#include <string>
using namespace std;

class UserCall {

public:
	static const int EAX = 0;  //u.ar0[EAX]�������ֳ���������EAX�Ĵ�����ƫ����

	enum ErrorCode {    //
		U_NOERROR = 0,//û�д���
		U_ENOENT = 1, //�Ҳ����ļ������ļ���
		U_EBADF = 2,  //�Ҳ����ļ����
		U_EACCES = 3, //Ȩ�޲���
		U_ENOTDIR = 4,//�ļ��в�����
		U_ENFILE = 5, //�ļ������
		U_EMFILE = 6, //���ļ�����
		U_EFBIG = 7,  //�ļ�����
		U_ENOSPC = 8  //���̿ռ䲻��
	};

public:
	INode* nowDirINodePointer;        //ָ��ǰĿ¼��Inodeָ��
	INode* paDirINodePointer;         //ָ��Ŀ¼��Inodeָ��

	DirectoryEntry dent;              //��ǰĿ¼��Ŀ¼��

	char dbuf[DirectoryEntry::DIRSIZ];//��ǰ·������

	string curDirPath;                //��ǰ����Ŀ¼����·��

	string dirp;                      //ϵͳ���ò���(һ������Pathname)��ָ��

	int arg[5];                       //��ŵ�ǰϵͳ���ò���
	/* ϵͳ������س�Ա */
	uint32 ar0[5];                    //    ָ�����ջ�ֳ���������EAX�Ĵ���
	//��ŵ�ջ��Ԫ�����ֶδ�Ÿ�ջ��Ԫ�ĵ�ַ��
	//��V6��r0���ϵͳ���õķ���ֵ���û�����
	//x86ƽ̨��ʹ��EAX��ŷ���ֵ�����u.ar0[R0]
	ErrorCode userErrorCode;          //��Ŵ�����
	ProcessOpenFile ofiles;           //���̴��ļ�������������
	IOParametereter IOParameter;              //��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ���ֽ�������

	SystemCall* systemCall;

	string ls;

public:
	UserCall();
	~UserCall();

	void userLs();
	void userCd(string dirName);
	void userMkDir(string dirName);
	void userCreate(string fileName);
	void userDelete(string fileName);
	void userOpen(string fileName);
	void userClose(string fd);
	void userSeek(string fd, string offset, string origin);
	void userWrite(string fd, string inFile, string size);
	void userRead(string fd, string outFile, string size);
	// void userRename(string ori, string cur);  //�������ļ����ļ���
	//void userTree(string path);               //��ӡ��״Ŀ¼

private:
	bool checkError();
	bool checkPathName(string path);
	void __userCd__(string dirName);
	void __userTree__(string path, int depth);//�ڲ���ӡ��״Ŀ¼


};

