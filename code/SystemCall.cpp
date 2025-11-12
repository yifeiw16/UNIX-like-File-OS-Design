#include "Common.h"
#include "SystemCall.h"
#include "CacheManager.h"
#include "UserCall.h"

extern CacheManager myCacheManager;   //����
extern wyfOpenFileTable mywyfOpenFileTable; 
extern FileSystem myFileSystem;      
extern INodeTable myINodeTable;
extern UserCall myUserCall;

SystemCall::SystemCall()  //��ʼ��
{
	fileSystem = &myFileSystem;
	wyfOpenFileTable = &mywyfOpenFileTable;
	inodeTable = &myINodeTable;
	rootDirINode = inodeTable->IGet(FileSystem::ROOT_INODE_NO);//�������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
																//��������ڴ��У���������ڴ�����������ظ��ڴ�INode
																//�ļ�ϵͳ��Ŀ¼���INode���
	rootDirINode->i_count += 0xff;//���ü���    

}


SystemCall::~SystemCall()
{
	INode* pINode = this->NameI(SystemCall::OPEN);
}



//usr/ast/mbox
//Ŀ¼����,
//���ݸ�Ŀ¼INode�ţ����ڴ�򿪸�Ŀ¼�ļ�
//����Ŀ¼�ļ������̿�һ�ζ����ڴ棬�����ַ���usr
//����usr�ļ�INode�ţ����ڴ�򿪸�Ŀ¼�ļ�
//����Ŀ¼�ļ������̿�һ�ζ����ڴ棬�����ַ���ast
//nameI�ı����Ƕ�����  ��INode����addr����INode

//ɾ����Ӧ������ΪԤ����Ŀ¼������Ŀ¼�ļ����ڴ�Inodeָ��
// ��Ŀ¼��λ�ü�¼��u_IOParameter�е�m_Offset�� Ӧ����ast���ڴ�Inodeָ���m_offset
// Ȼ���������Ŀ¼�� i_nlink-- //�ļ���������������ļ���Ŀ¼���в�ͬ·����������
// ��Ϊͬһ���ļ��п����ڲ�ͬ��·����
// 
//�򿪣����������һ���Ǵ���Ŀ¼��ģ����ҵ��׵ġ�Ӧ������ΪԤ����Ŀ¼������Ŀ¼�ļ����ڴ�Inodeָ��
// ��Ŀ¼��λ�ü�¼��u_IOParameter�е�m_Offset�� Ӧ����ast���ڴ�Inodeָ���m_offset
// ����Open�����ж�ast��ȡ��mbox�ļ���Inode
// 
// 
//����������ļ��Ѵ��ڣ���open����һ��������ast�����ļ���
//�����ڣ������һ�������������ɹ����Ǿ���ast������һ���µ�DiskInode������Ŀ¼��

//�򿪷��ص��ǣ������Ǹ�Inode mbox��Inode�� û�оͷ���NUll
//����������NULL����Ŀ¼myUserCall.paDirINodePointer������Ѵ��ڣ���openһ��������Inode mbox��Inode
//
//�����ɾ���������򷵻ظ�Ŀ¼Inode����Ҫɾ���ļ���Inode����u.u_dent.m_ino��
INode* SystemCall::NameI(enum DirectorySearchMode mode)
{
	INode* pINode = myUserCall.nowDirINodePointer; //����������
	wyfCacheBlock* pCache;

	int freeEntryOffset; //�Դ����ļ�ģʽ����Ŀ¼ʱ����¼����Ŀ¼���ƫ����
	unsigned int index = 0, nindex = 0;

	//�����·����'/'��ͷ�ģ��Ӹ�Ŀ¼��ʼ����������ӽ��̵�ǰ����Ŀ¼��ʼ����
	if ('/' == myUserCall.dirp[0]) {
		nindex = ++index + 1;
		pINode = this->rootDirINode;
	}


	//���ѭ��ÿ�δ���pathname��һ��·������
	while (1) {
		//����������ͷŵ�ǰ��������Ŀ¼�ļ�Inode�����˳�
		if (myUserCall.userErrorCode != UserCall::U_NOERROR)
			break;


		//����·��������ϣ�������ӦInodeָ�롣Ŀ¼�����ɹ�����
		if (nindex >= myUserCall.dirp.length())
			return pINode;

		//���Ҫ���������Ĳ���Ŀ¼�ļ����ͷ����Inode��Դ���˳�
		if ((pINode->i_mode & INode::IFMT) != INode::IFDIR) {//�ļ����ͣ�Ŀ¼�ļ�
			myUserCall.userErrorCode = UserCall::U_ENOTDIR;//�ļ��в�����
			break;
		}

		//��Pathname�е�ǰ׼������ƥ���·������������u.u_dbuf[]�У����ں�Ŀ¼����бȽϡ�
		nindex = myUserCall.dirp.find_first_of('/', index);
		memset(myUserCall.dbuf, 0, sizeof(myUserCall.dbuf));
		memcpy(myUserCall.dbuf, myUserCall.dirp.data() + index, (nindex == (unsigned int)string::npos ? myUserCall.dirp.length() : nindex) - index);
		index = nindex + 1;
		//�ڲ�ѭ�����ֶ���u.u_dbuf[]�е�·���������������Ѱƥ���Ŀ¼��
		myUserCall.IOParameter.offset = 0;


		//����ΪĿ¼����� �����հ׵�Ŀ¼��
		myUserCall.IOParameter.count = pINode->i_size / sizeof(DirectoryEntry);
		freeEntryOffset = 0;
		pCache = NULL;
		while (1) {
			/* ��Ŀ¼���Ѿ�������� */
			if (0 == myUserCall.IOParameter.count) {
				if (NULL != pCache)
					myCacheManager.Brelse(pCache);
				//����Ǵ������ļ�

				if (SystemCall::CREATE == mode && nindex >= myUserCall.dirp.length()) {
					//����Ŀ¼Inodeָ�뱣���������Ժ�дĿ¼��WriteDir()�������õ�
					myUserCall.paDirINodePointer = pINode;

					//����freeEntryOffset��ʵû���õ�������ʵ���Ǵ�����myUserCall.IOParameter.offset��
					if (freeEntryOffset) //�˱�������˿���Ŀ¼��λ��Ŀ¼�ļ��е�ƫ����
						myUserCall.IOParameter.offset = freeEntryOffset - sizeof(DirectoryEntry); //������Ŀ¼��ƫ��������u���У�дĿ¼��WriteDir()���õ�
					else //���⣺Ϊ��if��֧û����IUPD��־��  ������Ϊ�ļ��ĳ���û�б�ѽ
						pINode->i_flag |= INode::IUPD;

					//�ҵ�����д��Ŀ���Ŀ¼��λ�ã�NameI()��������
					return NULL;
				}
				//Ŀ¼��������϶�û���ҵ�ƥ����ͷ����Inode��Դ�����˳�
				myUserCall.userErrorCode = UserCall::U_ENOENT;
				goto out;
			}
			//�Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿�
			//�������ڶ�ĳһ��Inode��daddr
			if (0 == myUserCall.IOParameter.offset % INode::BLOCK_SIZE) {
				if (pCache)
					myCacheManager.Brelse(pCache);
				//����Ҫ���������̿��
				int phyBlkno = pINode->Bmap(myUserCall.IOParameter.offset / INode::BLOCK_SIZE);
				pCache = myCacheManager.Bread(phyBlkno);
			}
			//û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����u.u_dent
			memcpy(&myUserCall.dent, pCache->b_addr + (myUserCall.IOParameter.offset % INode::BLOCK_SIZE), sizeof(myUserCall.dent));
			myUserCall.IOParameter.offset += sizeof(DirectoryEntry);
			myUserCall.IOParameter.count--;
			//����ǿ���Ŀ¼���¼����λ��Ŀ¼�ļ���ƫ����
			if (0 == myUserCall.dent.m_ino) {
				if (0 == freeEntryOffset)
					freeEntryOffset = myUserCall.IOParameter.offset;
				//��������Ŀ¼������Ƚ���һĿ¼��
				continue;
			}

			if (!memcmp(myUserCall.dbuf, &myUserCall.dent.name, sizeof(DirectoryEntry) - 4))
				break;
		}

		//���ڲ�Ŀ¼��ƥ��ѭ�������˴���˵��pathname�е�ǰ·������ƥ��ɹ��ˣ�����ƥ��pathname����һ·��������ֱ������'\0'����
		if (pCache)
			myCacheManager.Brelse(pCache);

		//�����ɾ���������򷵻ظ�Ŀ¼Inode����Ҫɾ���ļ���Inode����u.u_dent.m_ino��
		if (SystemCall::DELETE == mode && nindex >= myUserCall.dirp.length())
			return pINode;

		//ƥ��Ŀ¼��ɹ������ͷŵ�ǰĿ¼Inode������ƥ��ɹ���Ŀ¼��m_ino�ֶλ�ȡ��Ӧ��һ��Ŀ¼���ļ���Inode
		this->inodeTable->IPUT(pINode);
		pINode = this->inodeTable->IGet(myUserCall.dent.m_ino);
		//�ص����While(true)ѭ��������ƥ��Pathname����һ·������

		if (NULL == pINode) //��ȡʧ��
			return NULL;
	}

out:
	this->inodeTable->IPUT(pINode);
	return NULL;

}





//trf == 0��open����
//trf == 1��creat���ã�creat�ļ���ʱ��������ͬ�ļ������ļ�
//trf == 2��creat���ã�creat�ļ���ʱ��δ������ͬ�ļ������ļ��������ļ�����ʱ��һ������
//mode���������ļ�ģʽ����ʾ�ļ������� ����д���Ƕ�д
//��������̶��ļ��ķ���Ȩ�� �������ļ��ṹ
void SystemCall::Open1(INode* pINode, int trf)
{
	//��creat�ļ���ʱ��������ͬ�ļ������ļ����ͷŸ��ļ���ռ�ݵ������̿飬����̿�
	if (1 == trf)
		pINode->ITrunc();//�ͷ�Inode��Ӧ�ļ�ռ�õĴ��̿�

	//������ļ����ƿ�File�ṹ
	File* pFile = this->wyfOpenFileTable->FAlloc();//��ϵͳ���ļ����з���һ�����е�File�ṹ
	//pFile���Ƿ��䵽��file�ṹ

	if (NULL == pFile) {
		this->inodeTable->IPUT(pINode);
		return;
	}
	
	//Fileָ���ļ����ڴ�Inode
	pFile->inode = pINode;

	//��ʱ�˿̹�����ϵ�Ѿ���������

	//Ϊ�򿪻��ߴ����ļ��ĸ�����Դ���ѳɹ����䣬��������
	if (myUserCall.userErrorCode == 0)
		return;
	else { //����������ͷ���Դ
		//�ͷŴ��ļ�������
		int fd = myUserCall.ar0[UserCall::EAX];
		if (fd != -1) {
			myUserCall.ofiles.SetF(fd, NULL);
			//�ݼ�File�ṹ��Inode�����ü��� ,File�ṹû���� f_countΪ0�����ͷ�File�ṹ��
			pFile->count--;
		}
		this->inodeTable->IPUT(pINode);
	}
}



//��Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ
//Ϊ�´������ļ�д�µ�i�ڵ�͸�Ŀ¼���µ�Ŀ¼��(��Ӧ������User�ṹ�� ��Ŀ¼��offset)
//���ص�pINode�����������ڴ�i�ڵ㣬���е�i_count�� 1
INode* SystemCall::MakNode(int mode)
{
	INode* pINode;

	//ջʽ����һ������DiskInode������������ȫ�����
	pINode = this->fileSystem->IAlloc();

	if (NULL == pINode)
		return NULL;

	pINode->i_flag = (INode::IACC | INode::IUPD);
	pINode->i_mode = mode | INode::IALLOC;
	pINode->i_nlink = 1;

	//��Ŀ¼��д��u.u_u_dent�����д��Ŀ¼�ļ�          //����������������������������������������������������������
	this->WriteDir(pINode);
	return pINode;
}


//���ܣ����ļ�
//Ч�����������ļ��ṹ���ڴ�i�ڵ㿪�� ��i_count Ϊ������i_count ++��
void SystemCall::Open()
{
	INode* pINode = this->NameI(SystemCall::OPEN);   //����Ŀ���ļ���Inode

	if (pINode == NULL)
		return;   //���ļ������ڣ�����

	this->Open1(pINode, 0);   //�����ļ��򿪽ṹ
}



//Creat()ϵͳ���ô�������
void SystemCall::Creat()
{
	INode* pINode;

	int newACCMode = myUserCall.arg[1];//��ŵ�ǰϵͳ���ò��� �ļ����ͣ�Ŀ¼�ļ�

	//����Ŀ¼��ģʽΪ1����ʾ����������Ŀ¼����д����������
	pINode = this->NameI(SystemCall::CREATE);

	//û���ҵ���Ӧ��INode����NameI����
	//ȷʵ��Ҫ����
	if (NULL == pINode) {
		if (myUserCall.userErrorCode)
			return;

		//ȷʵ��Ҫ����
		pINode = this->MakNode(newACCMode);
		if (NULL == pINode)
			return;
		//������������ֲ����ڣ�ʹ�ò���trf = 2������open1() �����ܵ�
		this->Open1(pINode, 2);
		return;
	}

	//�Ѿ����ڣ������Ǿ����
	//���NameI()�������Ѿ�����Ҫ�������ļ�������ո��ļ������㷨ITrunc()��
	this->Open1(pINode, 1);
	pINode->i_mode |= newACCMode;
}




//��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼��
//�������Լ���Ŀ¼��д����Ŀ¼���޸ĸ�Ŀ¼�ļ���i�ڵ� ������д�ش��̡�
void SystemCall::WriteDir(INode* pINode)
{
	//����Ŀ¼����INode��Ų���
	myUserCall.dent.m_ino = pINode->i_number;
	//����Ŀ¼����pathname��������
	memcpy(myUserCall.dent.name, myUserCall.dbuf, DirectoryEntry::DIRSIZ);

	myUserCall.IOParameter.count = DirectoryEntry::DIRSIZ + 4;
	myUserCall.IOParameter.base = (unsigned char*)&myUserCall.dent;
	//��Ŀ¼��д�븸Ŀ¼�ļ�
	myUserCall.paDirINodePointer->WriteI();
	this->inodeTable->IPUT(myUserCall.paDirINodePointer);
}


void SystemCall::Close()
{
	int fd = myUserCall.arg[0];

	//��ȡ���ļ����ƿ�File�ṹ
	File* pFile = myUserCall.ofiles.GetF(fd);
	if (NULL == pFile)
		return;

	//�ͷŴ��ļ�������fd���ݼ�File�ṹ���ü���
	myUserCall.ofiles.SetF(fd, NULL);
	this->wyfOpenFileTable->CloseF(pFile);
}

//��Ŀ¼�ļ���ɾ�����ļ���Ӧ��Ŀ¼�DiskInodeӲ������ d_link--,�����0����ɾ���ļ�
//����inode���������ݿ�
void SystemCall::UnLink()
{
	//ע��ɾ���ļ����д���й¶
	INode* pINode;
	INode* pDeleteINode;
	pDeleteINode = this->NameI(SystemCall::DELETE);//��Ŀ¼

	if (NULL == pDeleteINode)
		return;

	pINode = this->inodeTable->IGet(myUserCall.dent.m_ino); //�����Ҫɾ��
	if (NULL == pINode)
		return;

	//д��������Ŀ¼��
	myUserCall.IOParameter.offset -= (DirectoryEntry::DIRSIZ + 4);   //���Ŀ¼��
	myUserCall.IOParameter.base = (unsigned char*)&myUserCall.dent;
	myUserCall.IOParameter.count = DirectoryEntry::DIRSIZ + 4;

	myUserCall.dent.m_ino = 0;
	pDeleteINode->WriteI();   //Ŀ¼����Ŀ��дд0


	//�޸�inode��
	pINode->i_nlink--;
	pINode->i_flag |= INode::IUPD;

	this->inodeTable->IPUT(pDeleteINode);  //��һ�����˸о���Ҫ
	this->inodeTable->IPUT(pINode);   //���������Ŀ¼��û�ˣ�����������Ŀ¼��Ҳû��
}


//����File�ṹ�е�offset
void SystemCall::Seek()
{
	File* pFile;
	int fd = myUserCall.arg[0];

	pFile = myUserCall.ofiles.GetF(fd);   //File�ṹ
	if (NULL == pFile)
		return; //��FILE�����ڣ�GetF���������

	int offset = myUserCall.arg[1];

	switch (myUserCall.arg[2]) {
	case 0:
		//��дλ������Ϊoffset
		pFile->offset = offset;
		break;
	case 1:
		//��дλ�ü�offset(�����ɸ�)
		pFile->offset += offset;
		break;
	case 2:
		//��дλ�õ���Ϊ�ļ����ȼ�offset
		pFile->offset = pFile->inode->i_size + offset;
		break;
	default:
		break;
	}
	cout << "�ļ�ָ��ɹ��ƶ��� " << pFile->offset << endl;
}


void SystemCall::Read()
{
	//ֱ�ӵ���Rdwr()��������
	this->Rdwr(File::FREAD);
}


void SystemCall::Write()
{
	//ֱ�ӵ���Rdwr()��������
	this->Rdwr(File::FWRITE);
}


//myUserCall.ar0[UserCall::EAX]����ʵ�ʶ�д�ֽ���
void SystemCall::Rdwr(enum File::FileFlags mode)
{
	File* pFile;
	//����Read()/Write()��ϵͳ���ò���fd��ȡ���ļ����ƿ�ṹ
	pFile = myUserCall.ofiles.GetF(myUserCall.arg[0]);


	if (NULL == pFile) //�����ڸô��ļ���GetF�Ѿ����ù������룬�������ﲻ��Ҫ��������
		return;

	//�����������ļ��׵�ַ
	myUserCall.IOParameter.base = (unsigned char*)myUserCall.arg[1]; //Ŀ�껺������ַ
	myUserCall.IOParameter.count = myUserCall.arg[2];                //Ҫ���/д���ֽ���

	myUserCall.IOParameter.offset = pFile->offset; //�����ļ���ʼ��λ��


	if (File::FREAD == mode)
		pFile->inode->ReadI();
	else
		pFile->inode->WriteI();

	//���ݶ�д�������ƶ��ļ���дƫ��ָ��
	pFile->offset += (myUserCall.arg[2] - myUserCall.IOParameter.count);


	//����ʵ�ʶ�д���ֽ������޸Ĵ��ϵͳ���÷���ֵ�ĺ���ջ��Ԫ
	myUserCall.ar0[UserCall::EAX] = myUserCall.arg[2] - myUserCall.IOParameter.count;

}

//�����Ǵ�ӡĿ¼�е�����Ŀ¼��
//�����myUserCall.ls += myUserCall.dent.name;
void SystemCall::Ls()
{
	INode* pINode = myUserCall.nowDirINodePointer;
	wyfCacheBlock* pCache = NULL;
	myUserCall.IOParameter.offset = 0;
	myUserCall.IOParameter.count = pINode->i_size / sizeof(DirectoryEntry);  //�õ��м���Ŀ¼��

	//������֪��Inode��������Ҫ��Inode�е�daddr����bmap֪����Ӧ���̿�ţ����û����
	while (myUserCall.IOParameter.count) {
		if (0 == myUserCall.IOParameter.offset % INode::BLOCK_SIZE) {
			if (pCache)
				myCacheManager.Brelse(pCache);
			int phyBlkno = pINode->Bmap(myUserCall.IOParameter.offset / INode::BLOCK_SIZE);
			pCache = myCacheManager.Bread(phyBlkno);
		}
		//32���ֽ� ȡǰ28��
		memcpy(&myUserCall.dent, pCache->b_addr + (myUserCall.IOParameter.offset % INode::BLOCK_SIZE), sizeof(myUserCall.dent));
		myUserCall.IOParameter.offset += sizeof(DirectoryEntry);
		myUserCall.IOParameter.count--;

		if (0 == myUserCall.dent.m_ino)
			continue;

		myUserCall.ls += myUserCall.dent.name;
		myUserCall.ls += "\n";
	}

	if (pCache)
		myCacheManager.Brelse(pCache);
}


//�ı䵱ǰ����Ŀ¼  cd����
//������ʲô����NameI�������õ���Ҫ��Ŀ��Inode
void SystemCall::ChDir()
{
	INode* pINode;
	pINode = this->NameI(SystemCall::OPEN);
	if (NULL == pINode)
		return;
	//���������ļ�����Ŀ¼�ļ�
	if ((pINode->i_mode & INode::IFMT) != INode::IFDIR) {
		myUserCall.userErrorCode = UserCall::U_ENOTDIR;
		this->inodeTable->IPUT(pINode);
		return;
	}

	//�ı�ȫ�ֱ���
	myUserCall.nowDirINodePointer = pINode;

	//���µ�ǰ·��curDirPath����Զ�Ǵ�/��ʼ
	//·�����ǴӸ�Ŀ¼'/'��ʼ����������u.u_curdir������ϵ�ǰ·������
	if (myUserCall.dirp[0] != '/')
		myUserCall.curDirPath += myUserCall.dirp;
	else //����ǴӸ�Ŀ¼'/'��ʼ����ȡ��ԭ�й���Ŀ¼
		myUserCall.curDirPath = myUserCall.dirp;
	if (myUserCall.curDirPath.back() != '/')
		myUserCall.curDirPath.push_back('/');
}
