#pragma once

#include<iostream>
using namespace std;

//LRU ���� ��������ʱ��ͨ��һ���ļ�ϵͳ�Ľӿڵ��ã����blkno��Ӧ��512�ֽڵĶ�д

//�û���ַ�ռ�ʹ��̵����ݽ���ͨ���ڴ��еĸ��ٻ�������ɣ�����������Ͳ�������Ҫ����IO

//ÿһ��������һ�������ṹ 512

//ֻ��һ��NODEV���У������豸һ��

//������ƿ�
class wyfCacheBlock
{

public:    

	enum wyfCacheBlockFlag
	{
		CB_DONE = 0X1,  //IO��������
		CB_DELWRI=0X2
	};

	unsigned int flags;

	//�����߼����,�Ӷ��жϿɲ���������
	int b_blkno;

	//ָ��û�����ƿ�����Ļ������׵�ַ
	unsigned char* b_addr;

	//�贫�͵��ֽ���
	int wcount;

	int no;   //�������е����

	//������ϵ
	wyfCacheBlock* b_forw;
	wyfCacheBlock* b_back;

	wyfCacheBlock()
	{
		flags = 0;
		b_forw = NULL;
		b_back = NULL;
		wcount = 0;
		b_addr = NULL;
		b_blkno = -1;
		no = 0;    //no��ʲô��֪��
	}

	void Reset()
	{
		flags = 0;
		b_forw = NULL;
		b_back = NULL;
		wcount = 0;
		b_addr = NULL;
		b_blkno = -1;
		no = 0;
	}




};
