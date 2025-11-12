#include "Common.h"
#include "File.h"
#include "UserCall.h"

extern UserCall myUserCall;

File::File()  //����ʱ��ʼ��
{
	count = 0;
	inode = NULL;
	offset = 0;
}

File::~File()
{

}

void File::Reset()
{
	count = 0;
	inode = NULL;
	offset = 0;
}

ProcessOpenFile::ProcessOpenFile()  //��ʼ�����̴��ļ���
{
	memset(processwyfOpenFileTable, 0, sizeof(processwyfOpenFileTable));
}

ProcessOpenFile::~ProcessOpenFile()
{
}

//�ڽ��̵Ĵ��ļ����з���һ��fd
//����������е�����ˣ���ô���ַ��䰡 ��֪���ˣ�Ӧ�����ٵ���ProcessOpenFile::SetF������
int ProcessOpenFile::AllocFreeSlot()
{
	for (int i = 0; i < ProcessOpenFile::MAX_FILES; i++) {
		//���̴��ļ������������ҵ�������򷵻�֮
		if (!processwyfOpenFileTable[i]) {
			myUserCall.ar0[UserCall::EAX] = i;
			return i;
		}
	}
		

	myUserCall.ar0[UserCall::EAX] = -1; //Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ
	myUserCall.userErrorCode = UserCall::U_EMFILE;
	return -1;
}



//�����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ
//�����wyfOpenFileTableFileTable������ĵ�fd��
File* ProcessOpenFile::GetF(int fd)
{
	File* pFile;

	if (fd < 0 || fd >= ProcessOpenFile::MAX_FILES) {
		myUserCall.userErrorCode = UserCall::U_EBADF;
		return NULL;
	}

	pFile = this->processwyfOpenFileTable[fd];
	if (pFile == NULL)
		myUserCall.userErrorCode = UserCall::U_EBADF;
	return pFile;
}


//Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ
void ProcessOpenFile::SetF(int fd, File* pFile)
{
	if (fd < 0 || fd >= ProcessOpenFile::MAX_FILES)
		return;
	this->processwyfOpenFileTable[fd] = pFile;
}

