#include "Common.h"
#include "OpenFileManager.h"
#include "UserCall.h"
#include <ctime>


//����ļ����ڴ��д�
extern UserCall myUserCall;
extern INodeTable myINodeTable;  //�ڴ�Inode��

extern FileSystem myFileSystem;  //�����ļ��ṹ
extern CacheManager myCacheManager;   //����

wyfOpenFileTable::wyfOpenFileTable()
{
}

wyfOpenFileTable::~wyfOpenFileTable()
{
}

void wyfOpenFileTable::Reset()
{
	for (int i = 0; i < wyfOpenFileTable::MAX_FILES; ++i)
		wyfsysFileTable[i].Reset();
}


//���ã����̴��ļ������������ҵĿ�����֮�±�д�� ar0[EAX]
//��ϵͳ���ļ����з���һ�����е�File�ṹ
File* wyfOpenFileTable::FAlloc()
{
	int fd = myUserCall.ofiles.AllocFreeSlot();  //��������������������������
	if (fd < 0)
		return NULL;

	//ֻ����100������Ҫ�Ƕ������Ҿ�ֻ�ܱ�����
	for (int i = 0; i < wyfOpenFileTable::MAX_FILES; ++i) {
		//count == 0��ʾ�������
		//wyfsysFileTable ָ����File�ṹ����
		//��ǰ���ø��ļ����ƿ�Ľ�����������Ϊ0���ʾ��File���У����Է���������

		if (this->wyfsysFileTable[i].count == 0) {
			myUserCall.ofiles.SetF(fd, &this->wyfsysFileTable[i]);   //������ɶ�ã���������
			this->wyfsysFileTable[i].count++;
			this->wyfsysFileTable[i].offset = 0;
			return (&this->wyfsysFileTable[i]);  //���ط��䵽��File�ṹ
		}
	}
	myUserCall.userErrorCode = UserCall::U_ENFILE;
	return NULL;
}

//�ر��ļ� 
//�Դ��ļ����ƿ�File�ṹ�����ü���count��1�������ü���countΪ0�����ͷ�File�ṹ
void wyfOpenFileTable::CloseF(File* pFile)
{
	pFile->count--;
	if(pFile->count <=0)
		myINodeTable.IPUT(pFile->inode);
}



INodeTable::INodeTable()
{
	fileSystem = &myFileSystem;
}

INodeTable::~INodeTable()
{

}

//���InodeTable �����ÿһ��Inode�����ǻ�����100���ģ�ֻ����������˶���
void INodeTable::Reset()
{
	INode emptyINode;  
	for (int i = 0; i < INodeTable::NINODE; ++i)
		m_INodeTable[i].Reset();
}





//�������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
//��������ڴ��У���������ڴ�����������ظ��ڴ�INode������NULL:INode Table OverFlow
INode* INodeTable::IGet(int inumber)
{
	INode* pINode;
	int index = IsLoaded(inumber);
	//Inode�Ѿ����ڴ�����
	if (index >= 0) {
		pINode = m_INodeTable + index;
		++pINode->i_count;
		return pINode;
	}

	pINode = GetFreeINode();
	if (NULL == pINode) {
		cout << "�ڴ� INode �����!" << endl;
		myUserCall.userErrorCode = UserCall::U_ENFILE;
		return NULL;
	}
	//������һ���յ�INode�����ҵð�DiskInode����Ķ��������?
	pINode->i_number = inumber;
	pINode->i_count++;

	//�����е���֣���������ʲô��ϵ��?
	//�൱��һ������IO ���뿴����blkno�൱��ʽ�����ţ�һ��512Ϊһ�����������̵�������superblock��ʼ���൱��1/0��������
	//����DiskInodeҲ���п������Լ��Ļ�����  superblockռ2������
	// 
	//inumberָ������DiskInode�е����
	wyfCacheBlock* pCache = myCacheManager.Bread(FileSystem::INODE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);
	
	//��DiskInode�������ڴ�Inode��
	//��ô�����ڴ棬memcpy����
	pINode->ICopy(pCache, inumber);
	myCacheManager.Brelse(pCache);
	return pINode;
}


//�����Ϊinumber��DiskINode�Ƿ����ڴ濽����������򷵻ظ��ڴ�INode���ڴ�INode���е�����
//���û�У�����-1
//�����ȷ��һ�£�ɶ��DiskInode�ı�ţ���
int INodeTable::IsLoaded(int inumber)
{
	for (int i = 0; i < NINODE; ++i)
		if (m_INodeTable[i].i_number == inumber && m_INodeTable[i].i_count)
			return i;
	return -1;
}



//���ڴ�INode����Ѱ��һ�����е��ڴ�INode
//�����������INode
INode* INodeTable::GetFreeINode()
{
	for (int i = 0; i < INodeTable::NINODE; i++)
		if (this->m_INodeTable[i].i_count == 0)
			return m_INodeTable + i;
	return NULL;
}




//ɾ���ļ�  //���֣��Ǻ͹ر��ļ���ʲô����
void INodeTable::IPUT(INode* pINode)
{
	//��ǰ����Ϊ���ø��ڴ�INode��Ψһ���̣���׼���ͷŸ��ڴ�INode
	if (pINode->i_count == 1) {
		//���ļ��Ѿ�û��Ŀ¼·��ָ����
		if (pINode->i_nlink <= 0) {
			//�ͷŸ��ļ�ռ�ݵ������̿�
			pINode->ITrunc();
			pINode->i_mode = 0;
			//�ͷŶ�Ӧ�����INode
			this->fileSystem->IFree(pINode->i_number);
		}
		//�������INode��Ϣ
		pINode->IUpdate((int)time(NULL));
		//����ڴ�INode�����б�־λ
		pINode->i_flag = 0;

		//�����ڴ�inode���еı�־֮һ����һ����i_count == 0
		pINode->i_number = -1;
	}

	pINode->i_count--;
}

//�����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode��
void INodeTable::UpdateINodeTable()
{
	for (int i = 0; i < INodeTable::NINODE; ++i)
		if (this->m_INodeTable[i].i_count)
			this->m_INodeTable[i].IUpdate((int)time(NULL));
}