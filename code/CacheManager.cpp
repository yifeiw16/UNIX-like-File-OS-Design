#include "CacheManager.h"
//#include "Common.h"

extern DiskDriver myDiskDriver;

//ֻ��һ���豸һ������  ����ֻ��һ��˫��ѭ������������������ƿ�
// 
//���캯�����Զ�����
CacheManager::CacheManager()
{
	bufferList = new wyfCacheBlock;  //������е�ͷ
	InitList();
	//diskDriver = &myDiskDriver;
}

//����
CacheManager::~CacheManager()
{
	Bflush();
	delete bufferList;
}

//��ʽ�� ��0
void CacheManager::FormatBuffer()
{
	for (int i = 0; i < NBUF; ++i)
		nBuffer[i].Reset();
	InitList();
}

//bufferList->nbuffer[0]->1->2->3->4...n
//����һ��˫��ѭ������
//bufferList�ƺ���ֱ�ӹ���wyfCacheBlock�cacheblock
// 
//����һ�� ������ƿ�bufferList->nbuffer[0]->1->2->3->4...n��һ��˫��ѭ������
//ÿһ��nbuffer����һ��ֱ�Ӵ����ݵ�buffer  unsigned char buffer[100][512]
void CacheManager::InitList()
{
	for (int i = 0; i < NBUF; ++i) {

		//ǰ����
		if (i)  //i>0
			nBuffer[i].b_forw = nBuffer + i - 1;
		else {  //i=0
			nBuffer[i].b_forw = bufferList;
			bufferList->b_back = nBuffer + i;
		}

		if (i + 1 < NBUF)//ǰ��
			nBuffer[i].b_back = nBuffer + i + 1;
		else {    //���һ��
			nBuffer[i].b_back = bufferList;
			bufferList->b_forw = nBuffer + i;
		}

		nBuffer[i].b_addr = buffer[i];  //��������ƿ� ������׵�ַ
		nBuffer[i].no = i;  
	}
}

//ΪʲôҪ��bufferlist��������Ϊ�����൱��˫���������˵�����ͷ������ͳһ���в���
// 
//LRU����ͷwyfCacheBlockheBlock   ����������þͲ�һ����ͷ��
void CacheManager::DetachNode(wyfCacheBlock* pb)
{
	if (pb->b_back == NULL)
		return;
	pb->b_forw->b_back = pb->b_back;
	pb->b_back->b_forw = pb->b_forw;
	pb->b_back = NULL;
	pb->b_forw = NULL;
}

//ʹ�����˺�������ɻ�����е�β��
void CacheManager::InsertTail(wyfCacheBlock* pb)
{
	if (pb->b_back != NULL)
		return;

	pb->b_forw = bufferList->b_forw;
	pb->b_back = bufferList;

	bufferList->b_forw->b_back = pb;
	bufferList->b_forw = pb;
}

//����һ�黺�棬�ӻ��������ȡ�������ڶ�д�豸�ϵĿ�blkno
//����Ҫ��һ�������߼��̿����blkno���̿�
wyfCacheBlock* CacheManager::GetBlk(int blkno)
{
	wyfCacheBlock* pb;
	//�ҵ��������õ�
	if (map.find(blkno) != map.end()) {
		pb = map[blkno];
		DetachNode(pb);
		return pb;
	}

	pb = bufferList->b_back;   //pb��Ϊ��һ��ʵ�ʻ�����ƿ�
	if (pb == bufferList) {  //ֻ��һ�����������ʲôʱ�����ڣ�
		cout << "�޻����ɹ�ʹ��" << endl;
		return NULL;
	}

	//���û���ҵ������õģ��ʹӶ��е�ͷ���ó�һ��
	DetachNode(pb);
	map.erase(pb->b_blkno);

	if (pb->flags & wyfCacheBlock::CB_DELWRI)  //����Ƿ����ӳ�д
		/***************************************************************************************************************/
		diskDriver->write(pb->b_addr, BUFFER_SIZE, pb->b_blkno * BUFFER_SIZE);
	
    //��ձ�־
	pb->flags &= ~(wyfCacheBlock::CB_DELWRI | wyfCacheBlock::CB_DONE);
	pb->b_blkno = blkno;

	//map��¼ pb��Ӧblkno
	map[blkno] = pb;

	//�������뵽��  ������ƿ�
	return pb;
}


//�ͷŻ�����ƿ�buf �ŵ���βȥ
void CacheManager::Brelse(wyfCacheBlock* pb)
{
	InsertTail(pb);
}


//��һ�����̿飬blknoΪĿ����̿��߼����
//���ص��ǻ�����ƿ�   ��һ������Ҫ����ȡ��pb->b_addr

wyfCacheBlock* CacheManager::Bread(int blkno)
{
	//ȡ�û�����ƿ�
	wyfCacheBlock* pb = GetBlk(blkno);

	//ȡ������done�����߼���done�����ӳ�д��
	//������ɶ�����1��˵����ȡ���Ŀ������õ�buffer  2���ȴ����е��£�����ֱ�Ӷ�д
	if (pb->flags & (wyfCacheBlock::CB_DONE | wyfCacheBlock::CB_DELWRI))
		return pb;
	else {  //�������û��bdone��ʾ��˵���õ������·���Ļ�����ƿ顣��Ҫ�Լ���IO���������е��̿���뻺��buffer Ҳ����pb->addr
		/****************************************************************************************/
		diskDriver->read(pb->b_addr, BUFFER_SIZE, pb->b_blkno * BUFFER_SIZE);

		//
		pb->flags |= wyfCacheBlock::CB_DONE;
		return pb;
	}
	
}

//дһ�����̿�
void CacheManager::Bwrite(wyfCacheBlock* pb)
{   
	//����ӳ�д��־ ������һ�ּ�飨�Ǳ�Ҫ�ɣ�
	pb->flags &= ~(wyfCacheBlock::CB_DELWRI);

	/****************************************************************************************/
	diskDriver->write(pb->b_addr, BUFFER_SIZE, pb->b_blkno * BUFFER_SIZE);
	
	//��done��ʶ
	pb->flags |= (wyfCacheBlock::CB_DONE);
	this->Brelse(pb);
}


//�ӳ�д���̿飬���ѽ���������д��������һ�±�־���ȵ�֮��Ҫ�����ڱ�ĺ�����д
void CacheManager::Bdwrite(wyfCacheBlock* bp)
{
	//��done���ӳ�д��־
	bp->flags |= (wyfCacheBlock::CB_DELWRI | wyfCacheBlock::CB_DONE);
	this->Brelse(bp);
	return;
}



//���ָ�������е����� ����������ƿ飬���bp->b_addr
//void *memset(void *ptr, int value, size_t num);
//memset ������ ptr ָ����ڴ������ǰ num ���ֽ�����Ϊֵ value
void CacheManager::Bclear(wyfCacheBlock* bp)
{
	memset(bp->b_addr, 0, CacheManager::BUFFER_SIZE);
	return;
}


//���������ӳ�д�Ļ���ȫ����������� 
//���⣺���ɶʱ����õ����ǲ���exit��������ã�
void CacheManager::Bflush()
{
	wyfCacheBlock* pb = NULL;
	for (int i = 0; i < NBUF; ++i) {
		pb = nBuffer + i;
		if ((pb->flags & wyfCacheBlock::CB_DELWRI)) {
			pb->flags &= ~(wyfCacheBlock::CB_DELWRI);
			//diskDriver->write(pb->addr, BUFFER_SIZE, pb->blkno * BUFFER_SIZE);
			pb->flags |= (wyfCacheBlock::CB_DONE);
		}
	}
}

