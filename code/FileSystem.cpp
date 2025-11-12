#include "Common.h"
#include "FileSystem.h"
#include "UserCall.h"
#include "DiskDriver.h"
#include "OpenFileManager.h"
#include "CacheManager.h"
#include <ctime>

extern DiskDriver myDiskDriver;
extern CacheManager myCacheManager;
extern SuperBlock mySuperBlock;
extern INodeTable myINodeTable;
extern UserCall myUserCall;

//����
FileSystem::FileSystem()
{
	diskDriver = &myDiskDriver;
	superBlock = &mySuperBlock;
	cacheManager = &myCacheManager;

	if (!diskDriver->Exists())
		FormatDevice();
	else
		LoadSuperBlock();
}

FileSystem::~FileSystem()
{
	Update();
	diskDriver = NULL;
	superBlock = NULL;
}

//��ʽ��SuperBlock
//����ɶ�и�ʽ��
void FileSystem::FormatSuperBlock()
{
	superBlock->s_isize = FileSystem::INODE_SECTOR_NUMBER;
	superBlock->s_fsize = FileSystem::DISK_SECTOR_NUMBER;
	superBlock->s_nfree = 0;
	superBlock->s_free[0] = -1;
	superBlock->s_ninode = 0;
	superBlock->s_fmod = 0;
	time((time_t*)&superBlock->s_time);
}

//��ʽ�������ļ�ϵͳ
void FileSystem::FormatDevice()
{
	FormatSuperBlock();//��ʽ��SuperBlock��
	diskDriver->Construct();
	diskDriver->write((uint8*)(superBlock), sizeof(SuperBlock), 0);

	DiskINode emptyDINode, rootDINode;
	rootDINode.d_mode |= INode::IALLOC | INode::IFDIR;
	diskDriver->write((uint8*)&rootDINode, sizeof(rootDINode));
	for (int i = 1; i < FileSystem::INODE_NUMBER_ALL; ++i) {
		if (superBlock->s_ninode < SuperBlock::MAX_NUMBER_INODE)
			superBlock->s_inode[superBlock->s_ninode++] = i;
		diskDriver->write((uint8*)&emptyDINode, sizeof(emptyDINode));
	}
	char freeBlock[BLOCK_SIZE], freeBlock1[BLOCK_SIZE];
	memset(freeBlock, 0, BLOCK_SIZE);
	memset(freeBlock1, 0, BLOCK_SIZE);

	for (int i = 0; i < FileSystem::DATA_SECTOR_NUMBER; ++i) {
		if (superBlock->s_nfree >= SuperBlock::MAX_NUMBER_FREE) {
			memcpy(freeBlock1, &superBlock->s_nfree, sizeof(int) + sizeof(superBlock->s_free));
			diskDriver->write((uint8*)&freeBlock1, BLOCK_SIZE);
			superBlock->s_nfree = 0;
		}
		else
			diskDriver->write((uint8*)freeBlock, BLOCK_SIZE);
		superBlock->s_free[superBlock->s_nfree++] = i + FileSystem::DATA_START_SECTOR;
	}

	time((time_t*)&superBlock->s_time);
	diskDriver->write((uint8*)(superBlock), sizeof(SuperBlock), 0);
}

//��������������������������������������
//ϵͳ��ʼ��ʱ����SuperBlock
void FileSystem::LoadSuperBlock()
{
	fseek(diskDriver->diskPointer, 0, 0);
	diskDriver->read((uint8*)(superBlock), sizeof(SuperBlock), 0);
}






//�ڴ洢�豸dev�Ϸ���һ������DiskINode��һ�����ڴ����µ��ļ�
// ����һ��ȡ�õĿյ�Inode�����������������������������ﲻ̫��Ϊʲô����diskinode��������Ϳ��Դ����ļ���
//ջʽ�洢
INode* FileSystem::IAlloc()
{
	wyfCacheBlock* pCache;
	INode* pINode;
	int ino;

	//SuperBlockֱ�ӹ����Ŀ���Inode�������ѿգ����뵽��������������Inode ����100��inode
	/****************���ﻹû����ȫ��********************/

	if (superBlock->s_ninode <= 0) {
		ino = -1;
		for (int i = 0; i < superBlock->s_isize; ++i) { //�������е�inode
			pCache = this->cacheManager->Bread(FileSystem::INODE_START_SECTOR + i);
			int* p = (int*)pCache->b_addr;
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; ++j) {
				++ino;
				int mode = *(p + j * FileSystem::INODE_SIZE / sizeof(int));
				if (mode)
					continue;
				//������inode��i_mode == 0����ʱ������ȷ����inode�ǿ��еģ�
				//��Ϊ�п������ڴ�inodeû��д��������, ����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
				if (myINodeTable.IsLoaded(ino) == -1) {
					superBlock->s_inode[superBlock->s_ninode++] = ino;
					if (superBlock->s_ninode >= SuperBlock::MAX_NUMBER_INODE)
						break;
				}
			}

			this->cacheManager->Brelse(pCache);
			if (superBlock->s_ninode >= SuperBlock::MAX_NUMBER_INODE)
				break;
		}
		if (superBlock->s_ninode <= 0) {
			myUserCall.userErrorCode = UserCall::U_ENOSPC;
			return NULL;
		}
	}

	//����һ��inode����ջ
	ino = superBlock->s_inode[--superBlock->s_ninode];  

	pINode = myINodeTable.IGet(ino);

	if (NULL == pINode) {
		cout << "�޿����ڴ�洢INode" << endl;
		return NULL;
	}

	pINode->Clean();
	superBlock->s_fmod = 1;
	return pINode;
}

//�пյ��̿飬�������ӷ����ϣ� 
//����Ѿ�100���ˣ����Ǿ�Ӧ����ѡ��һ���鳤�����ȥ�ľ͵��鳤�ɡ�
// ���鳤��ø��𿽱�ԭ��superBlock->s_free���������ݣ�Ȼ���ٸ���superBlock->s_free nfree
// 
//�ͷŴ洢�豸dev�ϱ��Ϊblkno�Ĵ��̿�  ������Ҫ�ͷŲ�������̿��
void FileSystem::Free(int blkno)
{
	wyfCacheBlock* pCache;
	if (superBlock->s_nfree >= SuperBlock::MAX_NUMBER_FREE) {
		pCache = this->cacheManager->GetBlk(blkno);
		//����ԭ��superBlock->s_free����������
		int* p = (int*)pCache->b_addr;
		*p++ = superBlock->s_nfree;
		memcpy(p, superBlock->s_free, sizeof(int) * SuperBlock::MAX_NUMBER_FREE);
		superBlock->s_nfree = 0;
		this->cacheManager->Bwrite(pCache);
	}

	//�ٸ���superBlock->s_free nfree   ͬʱ������ӦΪ���С�鳤�Ļ����Ѿ���0����superBlock->s_nfree = 0;
	superBlock->s_free[superBlock->s_nfree++] = blkno;
	superBlock->s_fmod = 1;
}

//��SuperBlock������ڴ渱�����µ��洢�豸��SuperBlock��ȥ
void FileSystem::Update() {
	wyfCacheBlock* pCache;
	superBlock->s_fmod = 0;
	superBlock->s_time = (int)time(NULL);
	for (int j = 0; j < 2; j++) {
		int* p = (int*)superBlock + j * 128;
		pCache = this->cacheManager->GetBlk(FileSystem::SUPERBLOCK_START_SECTOR + j);
		memcpy(pCache->b_addr, p, BLOCK_SIZE);
		this->cacheManager->Bwrite(pCache);
	}
	myINodeTable.UpdateINodeTable();
	this->cacheManager->Bflush();
}

//�ڴ洢�豸�Ϸ�����д��̿�
//�������ӷ� ���ڴ�superblocj�е�s_nfree��s_free[]�в�ѯһ���µ��̿��

wyfCacheBlock* FileSystem::Alloc()
{
	int blkno;
	wyfCacheBlock* pCache;

	//ȡ��ֱ�ӹ��������һ�� ���д��̿�
	blkno = superBlock->s_free[--superBlock->s_nfree];

	//����ȡ���̿���Ϊ�㣬���ʾ�ѷ��価���еĿ��д��̿�
	/*************************************************************************************/
	if (blkno <= 0) {
		superBlock->s_nfree = 0;
		myUserCall.userErrorCode = UserCall::U_ENOSPC;
		return NULL;
	}

	pCache = this->cacheManager->GetBlk(blkno);


	//���release�����鳤������Ҫ��һЩ������
	// 
	// 
	// s_free[0]  [1]   [2] ��������blkno��
	if(superBlock->s_nfree <= 0){  //ֱ�ӹ������̿�����û��  ��Ϊǰ��--superBlock->s_nfree�� 
		//�鳤���ֳ�ȥ�� //����Ҫ֪���鳤blkno��Ӧ�����ݿ�����д����Щblkno��
		//����Ҫ���鳤blkno��Ӧ���ݿ�����Ķ���Ų�� ֱ�ӹ�����s_free��

		//step1����blkno���ոն����ģ��е��������� Ӧ����101��int  ��һ����100������100����ǰһ���blkno
		pCache = this->cacheManager->Bread(blkno);  //����������������ƿ� 

		int* p = (int*)pCache->b_addr;   //p�������ݿ�������ʼ��ַ

		//step2�����鳤blkno��Ӧ���ݿ�����Ķ���Ų�� ֱ�ӹ�����s_free��
		superBlock->s_nfree = *p++;
		memcpy(superBlock->s_free, p, sizeof(superBlock->s_free));
		this->cacheManager->Brelse(pCache);
	}

	if (pCache)
		this->cacheManager->Bclear(pCache);
	
	superBlock->s_fmod = 1;   //��ʾ�ڴ��SB���޸Ĺ���Ҫ��������е�SB
	return pCache;
}


//�пյ�inode
//�ͷű��Ϊnumber��DiskINode��һ������ɾ���ļ�
//�յ�inode��ջ��������blkno������s_inode����

void FileSystem::IFree(int number)
{
	if (superBlock->s_ninode >= SuperBlock::MAX_NUMBER_INODE)  //��ֻ�ܹ�100�������˹ܲ��� ���߰�
		return;
	superBlock->s_inode[superBlock->s_ninode++] = number;
	superBlock->s_fmod = 1;
}


