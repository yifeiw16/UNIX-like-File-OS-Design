#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "INode.h"   //����inode��diskinode

//ϵͳ���ļ���
//�κ�һ���ڴ�inodeֻ��Ӧһ��File�ṹ

//open�ļ�    1.����հ��ڴ�inode 2.copy  3.�հ�File�ṹ  4.�հ׵�user�еĽṹ
class File
{

public:
	enum FileFlags
	{
		FREAD = 0x1,	//����������
		FWRITE = 0x2,	//д��������
	};

public:
	File();
	~File();
	void Reset();

	unsigned int flag;	//�Դ��ļ��Ķ���д����Ҫ��
	int	count;		    //�����ǲ��ǿ�����ȥ��  ��ǰ���ø��ļ����ƿ�Ľ�����������Ϊ0���ʾ��File���У����Է���������
	INode* inode;		//ָ����ļ����ڴ�INodeָ��

	int	offset;		    //�ļ���дλ��ָ��
};


/*     
���̴��ļ���������(OpenFiles)�Ķ���
 * ά���˵�ǰ���̵����д��ļ���
 */

class ProcessOpenFile {
public:
	static const int MAX_FILES = 100;		//���������򿪵�����ļ���


private:
	File* processwyfOpenFileTable[MAX_FILES];	//File�����ָ�����飬ָ��ϵͳ���ļ����е�File����


public:

	ProcessOpenFile();
	~ProcessOpenFile();
	void Reset()   //����
	{
		memset(processwyfOpenFileTable, 0, sizeof(processwyfOpenFileTable));
	};

	int AllocFreeSlot();   //����������ļ�ʱ���ڴ��ļ����������з���һ�����б���  Ӧ�û᷵��һ��int fd
	File* GetF(int fd);    //�����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ	
	void SetF(int fd, File* pFile);//Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ
};



/*    �ļ�I/O�Ĳ�����
 * ���ļ�����дʱ���õ��Ķ���дƫ�������ֽ����Լ�Ŀ�������׵�ַ������
 */

class IOParametereter {
public:
	unsigned char* base;	//��ǰ����д�û�Ŀ��������׵�ַ
	int offset;			    //��ǰ����д�ļ����ֽ�ƫ����
	int count;			    //��ǰ��ʣ��Ķ���д�ֽ�����
};