#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Common.h"
#include "wyfCacheBlock.h"

//ÿһ���ļ���һ��DiskInode��㣬��һ���ڴ�Inode��㡣
// 
//�ڴ�Inode

class INode
{
public:
	
	/*i_flag��־λ*/
	enum INodeFlag
	{
		IUPD = 0x1,//�ڴ�INode���޸Ĺ�����Ҫ���¶�Ӧ���INode
		IACC = 0x2 //�ڴ�INode�����ʹ�����Ҫ�޸����һ�η���ʱ��  �����ʲô������

	};

	static const unsigned int IALLOC = 0x8000;    //�ļ���ʹ��
	static const unsigned int IFMT = 0x6000;      //�ļ���������
	static const unsigned int IFDIR = 0x4000;     //�ļ����ͣ�Ŀ¼�ļ�
	static const unsigned int ILARG = 0x1000;     //�ļ��������ͣ����ͻ�����ļ�
	static const unsigned int IREAD = 0x100;      //���ļ��Ķ�Ȩ��
	static const unsigned int IWRITE = 0x80;      //���ļ���дȨ��

	static const int BLOCK_SIZE = 512;

	static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);

	static const int SMALL_FILE_BLOCK = 6;                                    //С���ļ���ֱ������������Ѱַ���߼����
	static const int LARGE_FILE_BLOCK = 128 * 2 + 6;                          //�����ļ�����һ�μ������������Ѱַ���߼����
	static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;

	unsigned int i_flag;	//״̬�ı�־λ�������enum INodeFlag
	unsigned int i_mode;	//�ļ�������ʽ��Ϣ
	int		i_count;		//���ü���  ��������Ӧ��ֻ��1��0��  nonono �������ﶼ������
	                        //��νһ���ļ���Ӧһ��Inode�����ǽ��̿����Բ�ͬ�Ķ�дȨ�޶�ȡͬһ���ļ�������
							//һ��Inode�����ж��File�ṹ����ͬ��offset����ͬ��mode��ָ������

	int		i_nlink;		//�ļ���������������ļ���Ŀ¼���в�ͬ·����������
	int		i_number;		//*** DiskINode���еı�� *** 
	                        //���������ڴ�� Inode �е������ڵ�������Ҫ֪�����������ĸ���� DiskInode��
	                        //�Ա��ڽ����ڴ渱�����޸�֮����µ�����Ӧ�� DiskInode ��ȥ
	int		i_size;			//�ļ���С���ֽ�Ϊ��λ
	int		i_addr[10];		//�����ļ��߼���ź��������ת���Ļ���������
	int		i_lastr;		//������һ�ζ�ȡ�ļ����߼���ţ������ж��Ƿ���ҪԤ��




public:
	
	INode();
	~INode();

	void Reset()
	{
		i_mode = 0;
		i_count = 0;
		i_number = -1;
		i_size = 0;
		memset(i_addr, 0, sizeof(i_addr));
	}
	void ReadI();                           //����Inode�����е��������̿�����������ȡ��Ӧ���ļ�����
	void WriteI();                          //����Inode�����е��������̿���������������д���ļ�
	int Bmap(int wyf_lbn);                      //���ļ����߼����ת���ɶ�Ӧ�������̿��
	void IUpdate(int time);                 //�������Inode�����ķ���ʱ�䡢�޸�ʱ��
	void ITrunc();                          //�ͷ�Inode��Ӧ�ļ�ռ�õĴ��̿�
	void Clean();                           //���Inode�����е�����
	void ICopy(wyfCacheBlock* bp, int inumber);//���������Inode�ַ�������Ϣ�������ڴ�Inode��

};



/* ����INode
*      ������ INode �ṹ���� INode �ļ򻯰汾
*/
class DiskINode//64�ֽ� 
{
public:
	unsigned int d_mode;//״̬�ı�־λ�������enum INodeFlag
	int	d_nlink;		//�ļ���������������ļ���Ŀ¼���в�ͬ·����������
	int	d_size;			//�ļ���С���ֽ�Ϊ��λ
	int	d_addr[10];		//�����ļ��߼���ź��������ת���Ļ���������
	int	d_atime;		//������ʱ��
	int	d_mtime;		//����޸�ʱ��

	int padding;  //���������ֽڲ��ù���

public:
	DiskINode();
	~DiskINode();
};