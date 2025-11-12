#include "Common.h"
#include "INode.h"
#include "FileSystem.h"
#include "UserCall.h"

extern UserCall myUserCall;
extern CacheManager myCacheManager;
extern FileSystem myFileSystem;

//���캯��
DiskINode::DiskINode()
{
	this->d_mode = 0;
	this->d_nlink = 0;
	this->d_size = 0;
	memset(d_addr, 0, sizeof(d_addr));  //���0
	this->d_atime = 0;
	this->d_mtime = 0;
}

DiskINode::~DiskINode()
{
}

INode::INode()
{
	this->i_mode = 0;
	this->i_nlink = 0;
	this->i_count = 0;
	this->i_number = -1;
	this->i_size = 0;
	memset(i_addr, 0, sizeof(i_addr));
}


INode::~INode()
{
}

//����Inode�����е��������̿�����������ȡ��Ӧ���ļ�����
void INode::ReadI()
{
	CacheManager& CacheManager = myCacheManager;
	int wyf_lbn,bn;
	int offset,nbytes;
	wyfCacheBlock* pCache;

	//��Ҫ���ֽ���Ϊ�㣬�򷵻�
	if (0 == myUserCall.IOParameter.count)
		return;

	this->i_flag |= INode::IACC;

	//Ӧ���Ǹ�����Ҫ�������ֽڰ�

	while (UserCall::U_NOERROR == myUserCall.userErrorCode && myUserCall.IOParameter.count) {
		//���Ҫ����bn��
		wyf_lbn = bn = myUserCall.IOParameter.offset / INode::BLOCK_SIZE;

		//���Ҫ���Ǹ��������ı߿�ʼ
		offset = myUserCall.IOParameter.offset % INode::BLOCK_SIZE;

		//���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ
		nbytes = Common::min(INode::BLOCK_SIZE - offset, myUserCall.IOParameter.count);

		int remain = this->i_size - myUserCall.IOParameter.offset;  //�ļ���С-�����ļ���offset
		if (remain <= 0)
			return;  //��ѽ�Ѿ����������ļ��ˣ���д���ˣ���߹���

		//����ʣ��û������ֽڣ��ǽ��Ŷ���ֱ������
		//���͵��ֽ�������ȡ����ʣ���ļ��ĳ���
		nbytes = Common::min(nbytes, remain);
		if ((bn = this->Bmap(wyf_lbn)) == 0) //bmap���� daddr��Ӧ����ʵ�Ĵ����̿��bn
			return;

		pCache = CacheManager.Bread(bn);
		//������������ʼ��λ��
		//nbytes�� ������ڿɶ����ֽ���
		//��֣��Ҷ��ļ������Ƕ�������ȥ��myUserCall.IOParameter.base��
		unsigned char* start = pCache->b_addr + offset;
		memcpy(myUserCall.IOParameter.base, start, nbytes);
		myUserCall.IOParameter.base += nbytes;
		myUserCall.IOParameter.offset += nbytes;
		myUserCall.IOParameter.count -= nbytes;

		CacheManager.Brelse(pCache);
	}
}



//����Inode�����е��������̿���������������д���ļ�
void INode::WriteI() {

	int wyf_lbn, bn;
	int offset, nbytes;
	wyfCacheBlock* pCache;
	this->i_flag |= (INode::IACC | INode::IUPD);


	//��Ҫд�ֽ���Ϊ�㣬�򷵻�
	if (0 == myUserCall.IOParameter.count)
		return;

	//Ҫдcount���ֽ�
	while (UserCall::U_NOERROR == myUserCall.userErrorCode && myUserCall.IOParameter.count)
	{
		//wyf_lbn�Ǹ��ļ��еĵڼ����߼����
		wyf_lbn = myUserCall.IOParameter.offset / INode::BLOCK_SIZE;
		//����߼����е�offset
		offset = myUserCall.IOParameter.offset % INode::BLOCK_SIZE;

		//�����д����nbytes
		nbytes = Common::min(INode::BLOCK_SIZE - offset, myUserCall.IOParameter.count);

		if ((bn = this->Bmap(wyf_lbn)) == 0)
			return;

		//д�������Ǵ�ͷ��ʼддһ����
		if (INode::BLOCK_SIZE == nbytes) //���д������������һ���ַ��飬��Ϊ����仺��
			pCache = myCacheManager.GetBlk(bn);

		else //д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ�
			//�е�û��ͨΪʲôҪ�ȶ���дѽ��
			//�ǲ�����Ϊȡ���Ļ���������������ݣ�������������Ҫ�ģ������ȶ���������
			pCache = myCacheManager.Bread(bn);


		//���������ݵ���ʼдλ�� д����: ���û�Ŀ�����������ݵ�������
		unsigned char* start = pCache->b_addr + offset;

		//destination    source
		memcpy(start, myUserCall.IOParameter.base, nbytes);
		myUserCall.IOParameter.base += nbytes;
		myUserCall.IOParameter.offset += nbytes;
		myUserCall.IOParameter.count -= nbytes;

		if (myUserCall.userErrorCode != UserCall::U_NOERROR)
			myCacheManager.Brelse(pCache);

		//��������Ϊ�ӳ�д�������ڽ���I/O�������ַ��������������
		myCacheManager.Bdwrite(pCache);

		//�����Ҵ��ɣ��������ӳ�д��

		//��ͨ�ļ���������
		if (this->i_size < myUserCall.IOParameter.offset)
			this->i_size = myUserCall.IOParameter.offset;
		this->i_flag |= INode::IUPD;
	}
}

//������DiskInode�ַ�������Ϣ�������ڴ�Inode��
void INode::ICopy(wyfCacheBlock* pb, int inumber)
{
	DiskINode& dINode = *(DiskINode*)(pb->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode));
	i_mode = dINode.d_mode;
	i_size = dINode.d_size;
	i_nlink = dINode.d_nlink;

	//destination    source
	memcpy(i_addr, dINode.d_addr, sizeof(i_addr));
}

//��offset�õ�wyf_lbn
//���ļ����߼����ת���ɶ�Ӧ�������̿�� wyf_lbn��0-���٣������صľ�����������������������ţ������Ƕ���/һ�μ�ӣ������м��Ѿ���������
//�����С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż���
//һ�μ��������index0 = (wyf_lbn - 6) / 128 + 6;  ����d_addr�����   ��0-5��67��8��
//���μ�������� index0= ��wyf_lbn-6-128*2��/��128*128��+8
//index2 = �� ��wyf_lbn-��128*2+6����/128��%128
//index3 = ��wyf_lbn-��128*2+6����%128
//S29 P20

//d_addr���浽�׷���ʲô��int  ֱ�ӵģ��������ݿ�
//һ���� 512/4=128��int Ҳ�Ǵ������ݿ���
//���ص��� ����������
int INode::Bmap(int wyf_lbn)
{
	//Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	//(1) i_addr[0] - i_addr[5]Ϊֱ�����������ļ����ȷ�Χ��0 - 6���̿飻
	//(2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	//�ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	//(3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	//��������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	//(128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	CacheManager& CacheManager = myCacheManager;
	FileSystem& fileSystem = myFileSystem;
	wyfCacheBlock* pFirstCache, * pSecondCache;
	int phyBlkno, index;
	int* iTable;

	if (wyf_lbn >= INode::HUGE_FILE_BLOCK) {
		myUserCall.userErrorCode = UserCall::U_EFBIG;
		return 0;
	}


	//�����С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż���
	if (wyf_lbn < 6) {
		phyBlkno = this->i_addr[wyf_lbn];
		//������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ��������
		if (phyBlkno == 0 && (pFirstCache = fileSystem.Alloc()) != NULL) {
			phyBlkno = pFirstCache->b_blkno;
			CacheManager.Bdwrite(pFirstCache);
			this->i_addr[wyf_lbn] = phyBlkno;
			this->i_flag |= INode::IUPD;
		}
		return phyBlkno;
	}

	//wyf_lbn >= 6 ���͡������ļ�     index��addr�е�������0��������С���ļ�����ֱ�Ӷ������������������Ҫȥ�Ҷ�������
	if (wyf_lbn < INode::LARGE_FILE_BLOCK)
		index = (wyf_lbn - INode::SMALL_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK + 6;
	else //�����ļ�: ���Ƚ���263 - (128 * 128 * 2 + 128 * 2 + 6)���̿�֮��
		index = (wyf_lbn - INode::LARGE_FILE_BLOCK) / (INode::ADDRESS_PER_INDEX_BLOCK * INode::ADDRESS_PER_INDEX_BLOCK) + 8;


	//������addr�����pFirstCache
	phyBlkno = this->i_addr[index];
	if (phyBlkno)
		pFirstCache = CacheManager.Bread(phyBlkno);
	else { //������Ϊ�㣬���ʾ��������Ӧ�ļ����������
		this->i_flag |= INode::IUPD;
		if ((pFirstCache = fileSystem.Alloc()) == 0)
			return 0;
		this->i_addr[index] = pFirstCache->b_blkno;
	}

	//iTable����Ҫ���洢���Ƕ��Ŀ鱾�������ļ����ݿ�buffer���ģ���128��4λint���ݣ���Ӧ��������ţ�����iTable[index]ȡ���ľ�����Ҫ���������
	iTable = (int*)pFirstCache->b_addr;
	if (index >= 8) {
		//���ھ����ļ��������pFirstBuf���Ƕ��μ����������
		//��������߼���ţ����ɶ��μ���������ҵ�һ�μ��������
		index = ((wyf_lbn - INode::LARGE_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
		phyBlkno = iTable[index];

		if (phyBlkno) {
			CacheManager.Brelse(pFirstCache);
			pSecondCache = CacheManager.Bread(phyBlkno);   //�����ļ������һ����������ֻ��Ҫ���������ɻ����Ŀȡ���������
		}
		else {
			if ((pSecondCache = fileSystem.Alloc()) == NULL) {
				CacheManager.Brelse(pFirstCache);
				return 0;
			}
			iTable[index] = pSecondCache->b_blkno;
			CacheManager.Bdwrite(pFirstCache);
		}

		pFirstCache = pSecondCache; //��һ��Ҫ�˸���
		iTable = (int*)pSecondCache->b_addr;
	}

	if (wyf_lbn < INode::LARGE_FILE_BLOCK)//262
		index = (wyf_lbn - INode::SMALL_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;   //�������һ����Ҫ�õ����
	else   //����
		index = (wyf_lbn - INode::LARGE_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;

	if ((phyBlkno = iTable[index]) == 0 && (pSecondCache = fileSystem.Alloc()) != NULL) {    //ͨ��iTable�������������ж���������iTable�����ļ����Լ���iTable
		phyBlkno = pSecondCache->b_blkno;
		iTable[index] = phyBlkno;   //�൱�� *iTable+index*4 �ǶԵ�
		CacheManager.Bdwrite(pSecondCache);
		CacheManager.Bdwrite(pFirstCache);
	}
	else
		CacheManager.Brelse(pFirstCache);
	return phyBlkno;

}



//����ڴ�Inode�����е�����
void INode::Clean()
{
	//Inode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	//�����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	//���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	//�����ʼ��Ϊ��Чֵ��
	this->i_mode = 0;
	this->i_nlink = 0;
	this->i_size = 0;
	memset(i_addr, 0, sizeof(i_addr));
}


//�������Inode�����ķ���ʱ�䡢�޸�ʱ��
//��һ���Ǹ�ʲô��?
//�������
void INode::IUpdate(int time)
{
	wyfCacheBlock* pCache;
	DiskINode dINode;
	FileSystem& fileSystem = myFileSystem;
	CacheManager& CacheManager = myCacheManager;

	//��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskInode
	//Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־

	if (this->i_flag & (INode::IUPD | INode::IACC)) {
		pCache = CacheManager.Bread(FileSystem::INODE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);
		dINode.d_mode = this->i_mode;
		dINode.d_nlink = this->i_nlink;
		dINode.d_size = this->i_size;
		//des                 source
		memcpy(dINode.d_addr, i_addr, sizeof(dINode.d_addr));
		if (this->i_flag & INode::IACC)
			dINode.d_atime = time;
		if (this->i_flag & INode::IUPD)
			dINode.d_mtime = time;

		unsigned char* p = pCache->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
		DiskINode* pNode = &dINode;
		memcpy(p, pNode, sizeof(DiskINode));
		CacheManager.Bwrite(pCache);
	}
}

//�ͷ�Inode��Ӧ�ļ�ռ�õĴ��̿�
void INode::ITrunc()
{
	CacheManager& CacheManager = myCacheManager;
	FileSystem& fileSystem = myFileSystem;
	wyfCacheBlock* pFirstCache, * pSecondCache;

	for (int i = 9; i >= 0; --i) {
		if (this->i_addr[i]) {
			if (i >= 6) {
				pFirstCache = CacheManager.Bread(this->i_addr[i]);
				int* pFirst = (int*)pFirstCache->b_addr;
				for (int j = BLOCK_SIZE / sizeof(int) - 1; j >= 0; --j) {
					if (pFirst[j]) {
						if (i >= 8) {
							pSecondCache = CacheManager.Bread(pFirst[j]);
							int* pSecond = (int*)pSecondCache->b_addr;
							for (int k = BLOCK_SIZE / sizeof(int) - 1; k >= 0; --k) {
								if (pSecond[k]) {
									fileSystem.Free(pSecond[k]);
								}
							}
							CacheManager.Brelse(pSecondCache);
						}
						fileSystem.Free(pFirst[j]);
					}
				}
				CacheManager.Brelse(pFirstCache);
			}
			fileSystem.Free(this->i_addr[i]);
			this->i_addr[i] = 0;
		}
	}
	this->i_size = 0;
	this->i_flag |= INode::IUPD;
	this->i_mode &= ~(INode::ILARG);
	this->i_nlink = 1;
}