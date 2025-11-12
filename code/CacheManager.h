#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include "DiskDriver.h"
#include "wyfCacheBlock.h"
#include <unordered_map>
using namespace std;

//���ڻ�����һЩ����
class CacheManager
{
public:
	static const int NBUF = 100; //������������
	static const int BUFFER_SIZE = 512;

private:
	//���⣺���ƿ�ͻ�����ι���

	wyfCacheBlock* bufferList;  //������ƿ��ͷ

	wyfCacheBlock nBuffer[NBUF];    //������ƿ�����
	
	unsigned char buffer[NBUF][BUFFER_SIZE]; //���������� �����ݵ�

	//����ɶ
	unordered_map<int, wyfCacheBlock*> map;
	DiskDriver* diskDriver;
	
public:
	CacheManager();
	~CacheManager();

	wyfCacheBlock* GetBlk(int blkno);  //����һ�黺�棬���ڴ����̿��blkno
	void Brelse(wyfCacheBlock* bp);    //�ͷŻ�����ƿ�buf
	wyfCacheBlock* Bread(int blkno);   //��һ�����̿飬blknoΪĿ����̿��߼����

	void Bwrite(wyfCacheBlock* bp);           //дһ�����̿�
	void Bdwrite(wyfCacheBlock* bp);          //�ӳ�д���̿�

	//��ջ���������
	void Bclear(wyfCacheBlock* bp);           

	//���������ӳ�д�Ļ���ȫ�����������
	void Bflush();                         

	void FormatBuffer();                   //��ʽ������Buffer


private:
	
	//�����������еĺ���
	void InitList();                 //������ƿ���еĳ�ʼ��
	void DetachNode(wyfCacheBlock* pb);
	void InsertTail(wyfCacheBlock* pb);
	
};
