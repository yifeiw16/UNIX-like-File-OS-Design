#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "File.h"
#include "FileSystem.h"

//ʵ�ֹ�����ϵ

/*   
 * ����u��ProcessOpenFile���ļ�������ָ����ļ����е�File���ļ����ƽṹ���Լ���File
 * �ṹָ���ļ���Ӧ���ڴ�INode��
 */

//ProcessOpenFileֻ��Ҫ��һ��ָ�����飬������������ʲô
// 
//wyfOpenFileTable�ǹ�File�ṹ��

class wyfOpenFileTable{
public:
	static const int MAX_FILES = 100;   //���ļ����ƿ�FILE�ṹ������

public:
	File wyfsysFileTable[MAX_FILES];       //ϵͳ���ļ�����Ϊ���н��̹��������̴�
									     //�ļ����������а���ָ����ļ����ж�ӦFile�ṹ��ָ��

public:
	wyfOpenFileTable();
	~wyfOpenFileTable();
	void Reset();

	//��ϵͳ���ļ����з���һ�����е�File�ṹ
	File* FAlloc();                          

	//�Դ��ļ����ƿ�File�ṹ�����ü���count��1�������ü���countΪ0�����ͷ�File�ṹ
	void CloseF(File* pFile);                
};



/*    �ڴ�INode��(class INodeTable)
 * �����ڴ�INode�ķ�����ͷš�
 */

class INodeTable {
public:
	static const int NINODE = 100;     //�ڴ�Inode������

private:
	INode m_INodeTable[NINODE];    //�ڴ�INODE���100��
	FileSystem* fileSystem;                  //��ȫ�ֶ���g_FileSystem������

public:
	INodeTable();
	~INodeTable();

	//�������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
	//��������ڴ��У���������ڴ�����������ظ��ڴ�INode
	//���ʣ����������ISLoaded��ʲô����
	INode* IGet(int inumber);               

	//���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ���������޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿�
	void IPUT(INode* pINode);

	//�����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode��
	void UpdateINodeTable();  

	
	//�����Ϊinumber�����INode�Ƿ����ڴ濽����
	//������򷵻ظ��ڴ�INode���ڴ�INode���е�����
	//���ʣ����������IGet��ʲô����
	int IsLoaded(int inumber);        

	//���ڴ�INode����Ѱ��һ�����е��ڴ�INode
	INode* GetFreeINode();                   
	
	void Reset();
};