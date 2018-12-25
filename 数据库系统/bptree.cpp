
#include"bptree.h"



index::index(string filename) :name(filename)//建立索引,通过寻找当前目录下是否有索引文件来建立索引节点信息和起始指针位置，一个节点以一个块为单位
{
	keylength[0] = 4;
	keylength[1] = 4;
	keylength[2] = 20;
	fstream _file;
	_file.open(filename, ios::in);

	if (!_file)
	{
		_file.open(filename, ios::out);
		Number = 0;
	}
	else
	{
		_file.seekp(0, ios::end);
		long size = _file.tellg();
		Number = size / BLOCKSIZE;
		_file.close();

		if (Number == 0)
		{
			return;
		}
		char *currentBlock = new char[BLOCKSIZE];
		int buffernum = bf.GiveMeABlock(name, 0);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		type = *(int*)(currentBlock + 20);
		maxchild = (BLOCKSIZE - MESSAGE) / (keylength[type] + POINTERLENGTH) - 1;
		order = maxchild;
	}
}


void index::initialize(Data* key, int Addr, int ktype)//初始化，构造
{
	char* root = new char[BLOCKSIZE];
	*(int*)(root) = Internal;                                //block内结构，前20位记录当前节点信息，后键值信息分为四块：第一块为键值，第二块为兄弟键值位置
	*(int*)(root + 4) = 0;//node position                    //第三块为叶子节点独有，记录文件对应位置；对应的内节点记录下一块地址(比自己小的)，第四块为当前编号位置,第五块专门记比自己大的地址
	*(int*)(root + 8) = -1;//father node position            //节点第一部分空出特地用来记录第一个键值位置
	*(int*)(root + 12) = 1;//number of keys
	*(int*)(root + 16) = 0;//delete or not
	*(int*)(root + 20) = ktype;//type
	int NumOfKeys = *(int*)(root + 12);
	type = ktype;
	*(int*)(root + MESSAGE + keylength[type]) = NumOfKeys;//first member position
	if (key->flag == -1)
		*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
	else if (key->flag == 0)
		*(float*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
	else
	{
		memcpy((char*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
	}


	*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
	*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
	*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = -1;
	*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = 1;//leaf node position
	*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
	int buffernum = bf.GiveMeABlock(name, 0);
	memcpy(bf.bufferBlock[buffernum].values, root, BLOCKSIZE);
	bf.writeBlock(buffernum);
	bf.flashBack(buffernum);
	//writebuffer

	char* newBlock = new char[BLOCKSIZE];
	*(int*)(newBlock) = Leaf;
	*(int*)(newBlock + 4) = 1;//node position
	*(int*)(newBlock + 8) = 0;//father node position
	*(int*)(newBlock + 12) = 1;//number of keys
	*(int*)(newBlock + 16) = 0;//delete or not

	NumOfKeys = *(int*)(newBlock + 12);
	*(int*)(newBlock + MESSAGE + keylength[type]) = NumOfKeys;
	if (key->flag == -1)
		*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
	else if (key->flag == 0)
		*(float*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
	else
	{
		memcpy((char*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
	}

	*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
	*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
	*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
	*(int*)(newBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
	buffernum = bf.GiveMeABlock(name, 1);
	memcpy(bf.bufferBlock[buffernum].values, newBlock, BLOCKSIZE);
	bf.writeBlock(buffernum);
	bf.flashBack(buffernum);

	maxchild = (BLOCKSIZE - MESSAGE) / (keylength[type] + POINTERLENGTH) - 1;
	order = maxchild;
	Number = 2;
	type = ktype;
	delete[] newBlock;
	delete[] root;
}

int index::find(Data* key)//搜索键值
{
	char *currentBlock = new char[BLOCKSIZE];//查找也就是查找键值的过程

	int buffernum = bf.GiveMeABlock(name, 0);
	bf.useBlock(buffernum);
	memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
	//readbuffer;
	int LeafType = *(int*)(currentBlock);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int tempBro, position;

	while (LeafType == Internal)//若处于叶子节点，则在键值中查找，同时记录地址
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int i = 0;

		for (i = 0; i < NumOfKeys; i++)
		{
			if (type == 0)//int型
			{
				int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datai*)key)->x < NowKey))
					break;
			}
			else if (type == 1)//double型
			{
				float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Dataf*)key)->x < NowKey))
					break;
			}
			else//string型
			{
				string NowKey((char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH)));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datac*)key)->x.compare(NowKey)) < 0)
					break;
			}
		}
		if (i == NumOfKeys)
			position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
		if (position == -1)
		{
			return -1;
		}
		int buffernum = bf.GiveMeABlock(name, position);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		LeafType = *(int*)(currentBlock);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		//readbuffer
	}//查找的过程是将块读入内存中，按照查找算法即可，此处用的是顺序查找

	int NumOfKeys = *(int*)(currentBlock + 12);
	Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	for (int i = 0; i < NumOfKeys; i++)
	{
		if (type == 0)
		{
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Datai*)key)->x == NowKey) && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				delete[]currentBlock;
				return Addr;
			}
		}
		else if (type == 1)
		{
			float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Dataf*)key)->x == NowKey) && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				delete[]currentBlock;
				return Addr;
			}
		}
		else
		{
			string NowKey((char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH)));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Datac*)key)->x.compare(NowKey)) == 0 && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				delete[]currentBlock;
				return Addr;
			}
		}
	}
	return -1;
	delete[]currentBlock;
}

void index::insert(Data* key, int Addr)//插入
{
	char *currentBlock = new char[BLOCKSIZE];

	if (find(key) != -1)//判断插入是否已存在，存在则抛出异常
	{
		throw TableException("The key is duplicate!");
		return;
	}
	int buffernum = bf.GiveMeABlock(name, 0);
	bf.useBlock(buffernum);
	memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
	//readbuffer;
	int LeafType = *(int*)(currentBlock);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int tempBro, position;

	while (LeafType == Internal)//内部叶子节点有空闲位置，直接在搜索失败处插入新键值和地址
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int i = 0;

		for (i = 0; i < NumOfKeys; i++)
		{
			if (type == 0)
			{
				int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datai*)key)->x < NowKey))
					break;
			}
			else if (type == 1)
			{
				float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Dataf*)key)->x < NowKey))
					break;
			}
			else
			{
				string NowKey((char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH)));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datac*)key)->x.compare(NowKey)) < 0)
					break;
			}
		}
		if (i == NumOfKeys)
			position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
		if (position == -1)
		{
			char* newBlock = new char[BLOCKSIZE];
			*(int*)(newBlock) = Leaf;
			*(int*)(newBlock + 4) = Number++;
			*(int*)(newBlock + 8) = *(int*)(currentBlock + 4);
			*(int*)(newBlock + 12) = 1;
			*(int*)(newBlock + 16) = 0;

			*(int*)(newBlock + MESSAGE + keylength[type]) = *(int*)(newBlock + 12);
			if (key->flag == -1)
				*(int*)(newBlock + MESSAGE + (keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
			else if (key->flag == 0)
				*(float*)(newBlock + MESSAGE + (keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
			else
			{
				memcpy((char*)(newBlock + MESSAGE + (keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
			}
			*(int*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type]) = -1;
			*(int*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type] + 4) = Addr;
			*(int*)(newBlock + MESSAGE + keylength[type] + POINTERLENGTH + keylength[type] + 8) = 1;
			*(int*)(newBlock + MESSAGE + (keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
			position = Number - 1;
			int buffernum = bf.GiveMeABlock(name, position);
			memcpy(bf.bufferBlock[buffernum].values, newBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);

			*(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Number - 1;
			buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
			memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			return;
		}
		int buffernum = bf.GiveMeABlock(name, position);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		LeafType = *(int*)(currentBlock);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		//readbuffer;
	}

	int NumOfKeys = *(int*)(currentBlock + 12);
	Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	if (NumOfKeys < maxchild)//内部叶子节点有空闲位置，直接在搜索失败处插入新键值和地址
	{
		*(int*)(currentBlock + 12) += 1;
		NumOfKeys++;
		int LastBro = 0;
		int i = 0;

		for (i = 0; i < NumOfKeys; i++)
		{
			if (type == 0)
			{
				if (Brother == -1 && i == NumOfKeys - 1)
					break;
				int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				if ((((Datai*)key)->x < NowKey))
					break;
			}
			else if (type == 1)
			{
				if (Brother == -1 && i == NumOfKeys - 1)
					break;
				float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				if ((((Dataf*)key)->x < NowKey))
					break;
			}
			else
			{
				if (Brother == -1 && i == NumOfKeys - 1)
					break;
				string NowKey((char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH)));
				if ((((Datac*)key)->x.compare(NowKey)) < 0)
					break;
			}
			LastBro = Brother;
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
		}

		if (key->flag == -1)
			*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
		else if (key->flag == 0)
			*(float*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
		else
		{
			memcpy((char*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
		}

		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) =
			*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 8);
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) =
			NumOfKeys;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) =
			Addr;
		*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) =
			NumOfKeys;
		if (i == NumOfKeys - 1)
			*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;

		int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
		memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
		bf.writeBlock(buffernum);
		bf.flashBack(buffernum);
		//writebuffer
	}
	else//若没有空闲位置，要对叶子节点进行分裂
	{
		*(int*)(currentBlock + 12) += 1;
		Data* mid = NULL;
		split(currentBlock, mid, key, Addr, 0, 0);
		bf.GiveMeABlock(name, 8);
	}

	delete[]currentBlock;
}

int* index::split(char* currentBlock, Data* mid, Data* key, int Addr, int leftpos, int rightpos)//分裂b+树节点
{
	int NumOfKeys = *(int*)(currentBlock + 12);
	int LeafType = *(int*)(currentBlock);
	int father[2] = { 0, 0 };
	if ((*(int*)(currentBlock + 8) == -1) && NumOfKeys >= maxchild - 1)//如果没有空闲位置
	{
		char* newBlock1 = new char[BLOCKSIZE];
		char* newBlock2 = new char[BLOCKSIZE];
		SplitInternal(newBlock1, newBlock2, currentBlock, mid, leftpos, rightpos);//分裂内部节点
		leftpos = *(int*)(newBlock1 + 4);
		rightpos = *(int*)(newBlock2 + 4);

		*(int*)(newBlock1 + 8) = 0;
		*(int*)(newBlock2 + 8) = 0;

		if (type == 0)
		{
			mid = new Datai(*(int*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
		}
		else if (type == 1)
		{
			mid = new Dataf(*(float*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
		}
		else
		{
			mid = new Datac((char*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
		}

		int buffernum = bf.GiveMeABlock(name, *(int*)(newBlock1 + 4));
		memcpy(bf.bufferBlock[buffernum].values, newBlock1, BLOCKSIZE);
		bf.writeBlock(buffernum);
		bf.flashBack(buffernum);
		//写入缓存
		buffernum = bf.GiveMeABlock(name, *(int*)(newBlock2 + 4));
		memcpy(bf.bufferBlock[buffernum].values, newBlock2, BLOCKSIZE);
		bf.writeBlock(buffernum);
		bf.flashBack(buffernum);
		//写入缓存
		char* root = new char[BLOCKSIZE];
		*(int*)(root) = Internal;
		*(int*)(root + 4) = 0;//节点位置
		*(int*)(root + 8) = -1;//父亲节点的位置
		*(int*)(root + 12) = 1;//键值的位置
		*(int*)(root + 16) = 0;//是否删除
		*(int*)(root + 20) = type;//类型

		int NumOfKeys = *(int*)(root + 12);
		*(int*)(root + MESSAGE + keylength[type]) = NumOfKeys;
		if (key->flag == -1)
			*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Datai*)mid)->x;
		else if (key->flag == 0)
			*(float*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Dataf*)mid)->x;
		else
		{
			memcpy((char*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)), ((Datac*)mid)->x.c_str(), ((Datac*)mid)->x.length() + 1);
		}
		*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
		*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
		*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
		*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
		*(int*)(root + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//删除与否

		buffernum = bf.GiveMeABlock(name, *(int*)(root + 4));
		memcpy(bf.bufferBlock[buffernum].values, root, BLOCKSIZE);
		bf.writeBlock(buffernum);
		bf.flashBack(buffernum);
		//写入缓存

		father[0] = *(int*)(newBlock1 + 4);
		father[1] = *(int*)(newBlock2 + 4);

		delete[]root;
		delete[]newBlock1;
		delete[]newBlock2;
		return father;
	}
	else
	{
		if (LeafType == Leaf&&NumOfKeys >= maxchild)//如果叶子节点已满
		{
			char* newBlock1 = new char[BLOCKSIZE];
			char* newBlock2 = new char[BLOCKSIZE];
			int *temp = NULL;
			SplitLeaf(newBlock1, newBlock2, currentBlock, key, Addr);//分裂叶子节点
			*(int*)(currentBlock + 16) = 1;
			leftpos = *(int*)(newBlock1 + 4);
			rightpos = *(int*)(newBlock2 + 4);

			if (type == 0)
			{
				mid = new Datai(*(int*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
			}
			else if (type == 1)
			{
				mid = new Dataf(*(float*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
			}
			else
			{
				mid = new Datac((char*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
			}

			int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 8));
			bf.useBlock(buffernum);
			char*fatherBlock = new char[BLOCKSIZE];
			memcpy(fatherBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
			//readbuffer;

			temp = split(fatherBlock, mid, key, Addr, leftpos, rightpos);

			*(int*)(newBlock1 + 8) = temp[0];
			*(int*)(newBlock2 + 8) = temp[1];

			buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
			memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer

			buffernum = bf.GiveMeABlock(name, *(int*)(newBlock1 + 4));
			memcpy(bf.bufferBlock[buffernum].values, newBlock1, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			buffernum = bf.GiveMeABlock(name, *(int*)(newBlock2 + 4));
			memcpy(bf.bufferBlock[buffernum].values, newBlock2, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer

			father[0] = *(int*)(newBlock1 + 4);
			father[1] = *(int*)(newBlock2 + 4);

			delete[]newBlock1;
			delete[]newBlock2;
			delete[]fatherBlock;
			return father;
		}
		else if (LeafType == Internal&&NumOfKeys >= maxchild - 1)//如果内部节点没有空闲位置
		{
			char*newBlock1 = new char[BLOCKSIZE];
			char*newBlock2 = new char[BLOCKSIZE];
			SplitInternal(newBlock1, newBlock2, currentBlock, mid, leftpos, rightpos);//mid need be change
			leftpos = *(int*)(newBlock1 + 4);
			rightpos = *(int*)(newBlock2 + 4);
			*(int*)(currentBlock + 16) = 1;
			int*temp = NULL;

			if (type == 0)
			{
				mid = new Datai(*(int*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
			}
			else if (type == 1)
			{
				mid = new Dataf(*(float*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
			}
			else
			{
				mid = new Datac((char*)(newBlock2 + MESSAGE + keylength[type] + POINTERLENGTH));
			}

			int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 8));
			bf.useBlock(buffernum);
			char*fatherBlock = new char[BLOCKSIZE];
			memcpy(fatherBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
			//readbuffer;

			temp = split(fatherBlock, mid, key, Addr, leftpos, rightpos);

			*(int*)(newBlock1 + 8) = temp[0];
			*(int*)(newBlock2 + 8) = temp[1];

			buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
			memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer

			buffernum = bf.GiveMeABlock(name, *(int*)(newBlock1 + 4));
			memcpy(bf.bufferBlock[buffernum].values, newBlock1, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			buffernum = bf.GiveMeABlock(name, *(int*)(newBlock2 + 4));
			memcpy(bf.bufferBlock[buffernum].values, newBlock2, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer

			father[0] = *(int*)(newBlock1 + 4);
			father[1] = *(int*)(newBlock2 + 4);

			delete[]newBlock1;
			delete[]newBlock2;
			delete[]fatherBlock;
			return father;
		}
		else if (LeafType == Internal&&NumOfKeys < maxchild - 1)//分裂之后满足分裂条件即可插入 
		{
			*(int*)(currentBlock + 12) += 1;

			Internal_insert(currentBlock, mid, leftpos, rightpos);

			int buffernum = bf.GiveMeABlock(name, 8);
			buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
			memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
			bf.writeBlock(buffernum);
			bf.flashBack(buffernum);
			//writebuffer
			buffernum = bf.GiveMeABlock(name, 8);
			father[0] = *(int*)(currentBlock + 4);
			father[1] = *(int*)(currentBlock + 4);

			return father;
		}
	}
}

void  index::Internal_insert(char* currentBlock, Data* mid, int leftpos, int rightpos)//向中间节点插入数据
{
	int NumOfKeys = *(int*)(currentBlock + 12);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int LastBro = 0;
	int i = 0;

	for (i = 0; i < NumOfKeys; i++)//插入如果导致树的层次增加，则要调整树
	{
		if (type == 0)
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			if ((((Datai*)mid)->x < NowKey))
				break;
		}
		else if (type == 1)
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			if ((((Dataf*)mid)->x < NowKey))
				break;
		}
		else
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			string NowKey;
			NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			if ((((Datac*)mid)->x.compare(NowKey)) < 0)
				break;
		}
		LastBro = Brother;
		Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
	}
	if (type == 0)
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Datai*)mid)->x;
	else if (type == 1)
		*(float*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)) = ((Dataf*)mid)->x;
	else
	{
		memcpy((char*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH)), ((Datac*)mid)->x.c_str(), ((Datac*)mid)->x.length() + 1);
	}

	if (i == NumOfKeys - 1)
	{
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
		*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) = NumOfKeys;
		*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
	}
	else
	{
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type]) = Brother;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = NumOfKeys;
		*(int*)(currentBlock + MESSAGE + NumOfKeys*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
		*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type]) = NumOfKeys;
		*(int*)(currentBlock + MESSAGE + LastBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
		*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
	}
}

void index::SplitLeaf(char* block1, char*block2, char* currentBlock, Data* key, int Addr)//分裂b+树叶子节点
{
	int NumOfKeys = *(int*)(currentBlock + 12);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int Address, flag = 1;
	int i, position = 1;

	*(int*)(block1) = Leaf;
	*(int*)(block1 + 4) = Number++;
	*(int*)(block1 + 8) = 0;
	*(int*)(block1 + 12) = NumOfKeys / 2;
	*(int*)(block1 + 16) = 0;

	*(int*)(block2) = Leaf;
	*(int*)(block2 + 4) = Number++;
	*(int*)(block2 + 8) = 0;
	*(int*)(block2 + 12) = NumOfKeys - (int)(NumOfKeys / 2);
	*(int*)(block2 + 16) = 0;

	*(int*)(block1 + MESSAGE + keylength[type]) = 1;
	*(int*)(block2 + MESSAGE + keylength[type]) = 1;

	for (i = 0; i < NumOfKeys / 2; i++)//将前NumOfKeys / 2个节点保留在左侧
	{
		if (type == 0)//对于插入的父节点来书只是插入了一个节点，所以直接插入，无需调整树的结构
		{
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if ((((Datai*)key)->x < NowKey) && flag)
			{
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		else if (type == 1)
		{
			float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if ((((Dataf*)key)->x < NowKey) && flag)
			{
				*(float*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(float*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		else
		{
			string NowKey;
			NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if ((((Datac*)key)->x.compare(NowKey)) < 0 && flag)
			{
				memcpy((char*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				memcpy((char*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), NowKey.c_str(), NowKey.length() + 1);
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
	}
	*(int*)(block1 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;

	position = 1;
	for (; i < NumOfKeys; i++)//其他节点移入右侧
	{
		if (type == 0)
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if ((((Datai*)key)->x < NowKey) && flag)
			{
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		else if (type == 1)
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if ((((Dataf*)key)->x < NowKey) && flag)
			{
				*(float*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(float*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
		else
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			string NowKey;
			NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Address = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			if ((((Datac*)key)->x.compare(NowKey)) < 0 && flag)
			{
				memcpy((char*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				memcpy((char*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), NowKey.c_str(), NowKey.length() + 1);
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Address;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			}
		}
	}
	if (flag)
	{
		if (key->flag == -1)
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Datai*)key)->x;
		else if (key->flag == 0)
			*(float*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Dataf*)key)->x;
		else
		{
			memcpy((char*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), ((Datac*)key)->x.c_str(), ((Datac*)key)->x.length() + 1);
		}
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr;
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
		position++;
	}
	*(int*)(block2 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
}

void index::SplitInternal(char* block1, char*block2, char* currentBlock, Data* mid, int leftpos, int rightpos)//分裂b+树中间节点
{
	*(int*)(currentBlock + 12) += 1;
	int NumOfKeys = *(int*)(currentBlock + 12);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int Addr1, Addr2;
	int i, position = 1, flag = 1, LastBro = 0;

	*(int*)(block1) = Internal;
	*(int*)(block1 + 4) = Number++;
	*(int*)(block1 + 8) = 0;
	*(int*)(block1 + 12) = NumOfKeys / 2;
	*(int*)(block1 + 16) = 0;

	*(int*)(block2) = Internal;
	*(int*)(block2 + 4) = Number++;
	*(int*)(block2 + 8) = 0;
	*(int*)(block2 + 12) = NumOfKeys - (int)(NumOfKeys / 2);
	*(int*)(block2 + 16) = 0;

	*(int*)(block1 + MESSAGE + keylength[type]) = 1;
	*(int*)(block2 + MESSAGE + keylength[type]) = 1;

	for (i = 0; i < NumOfKeys / 2; i++)//将前NumOfKeys/2键值和指针留在左侧
	{
		if (type == 0)
		{
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if ((((Datai*)mid)->x <= NowKey) && flag)
			{
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Datai*)mid)->x;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				*(int*)(block1 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				position++;
			}
		}
		else if (type == 1)
		{
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if ((((Dataf*)mid)->x <= NowKey) && flag)
			{
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Dataf*)mid)->x;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				*(int*)(block1 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				position++;
			}
		}

		else
		{
			string NowKey;
			NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if ((((Datac*)mid)->x.compare(NowKey)) <= 0 && flag)
			{
				memcpy((char*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), ((Datac*)mid)->x.c_str(), ((Datac*)mid)->x.length() + 1);
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				*(int*)(block1 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				memcpy((char*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), NowKey.c_str(), NowKey.length() + 1);
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block1 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				position++;
			}
		}
	}

	position = 1;
	int j = i;
	for (; i < NumOfKeys; i++)//将后NumOfKeys / 2个键值和指针保留在右侧
	{
		if (type == 0)
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if ((((Datai*)mid)->x <= NowKey) && flag)
			{
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Datai*)mid)->x;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				if (i == j)
				{
					*(int*)(block1 + MESSAGE + *(int*)(block1 + 12) * (keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				}
				else *(int*)(block2 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				position++;
			}
		}
		else if (type == 1)
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if ((((Dataf*)mid)->x <= NowKey) && flag)
			{
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Dataf*)mid)->x;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				if (i == j)
				{
					*(int*)(block1 + MESSAGE + *(int*)(block1 + 12) * (keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				}
				else *(int*)(block2 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = NowKey;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				position++;
			}
		}

		else
		{
			if (Brother == -1 && i == NumOfKeys - 1)
				break;
			string NowKey;
			NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			Addr1 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Addr2 = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
			if ((((Datac*)mid)->x.compare(NowKey)) <= 0 && flag)
			{
				memcpy((char*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), ((Datac*)mid)->x.c_str(), ((Datac*)mid)->x.length() + 1);
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = rightpos;
				if (i == j)
				{
					*(int*)(block1 + MESSAGE + *(int*)(block1 + 12) * (keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				}
				else *(int*)(block2 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				position++;
				flag = 0;
			}
			else
			{
				memcpy((char*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), NowKey.c_str(), NowKey.length() + 1);
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = position + 1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = Addr1;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = Addr2;
				*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
				LastBro = Brother;
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				position++;
			}
		}
	}
	if (flag)
	{
		if (mid->flag == -1)
			*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Datai*)mid)->x;
		else if (mid->flag == 0)
			*(float*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)) = ((Dataf*)mid)->x;
		else
		{
			memcpy((char*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH)), ((Datac*)mid)->x.c_str(), ((Datac*)mid)->x.length() + 1);
		}
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type]) = -1;
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 4) = leftpos;
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 8) = position;
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 0;//delete or not
		*(int*)(block2 + MESSAGE + position*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = rightpos;
		*(int*)(block2 + MESSAGE + (position - 1)*(keylength[type] + POINTERLENGTH) + keylength[type] + 12) = leftpos;
		position++;
	}
}

void index::Delete(Data* key)//删除键值
{
	char *currentBlock = new char[BLOCKSIZE];

	int buffernum = bf.GiveMeABlock(name, 0);
	bf.useBlock(buffernum);
	memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
	//readbuffer;
	int LeafType = *(int*)(currentBlock);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int tempBro, position;

	while (LeafType == Internal)//删除内部节点，需要考虑删除后节点的数量是否满足b+树
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int i = 0;

		for (i = 0; i < NumOfKeys; i++)
		{
			if (type == 0)
			{
				int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datai*)key)->x < NowKey))
					break;
			}
			else if (type == 1)
			{
				float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Dataf*)key)->x < NowKey))
					break;
			}
			else
			{
				string NowKey;
				NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datac*)key)->x.compare(NowKey)) < 0)
					break;
			}
		}
		if (i == NumOfKeys)//如果等于键值的数量，直接删除
			position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
		if (position == -1)
		{
			throw TableException("Wrong delete!");
		}
		int buffernum = bf.GiveMeABlock(name, position);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		LeafType = *(int*)(currentBlock);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		//readbuffer
	}

	int NumOfKeys = *(int*)(currentBlock + 12);
	Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int i = 0;
	for (i = 0; i < NumOfKeys; i++)//如果删除后小于关键字的数量，则从邻居节点借节点，和分裂相反
	{
		if (type == 0)
		{
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Datai*)key)->x == NowKey) && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 1;
				int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				break;
			}
			else
			{
				throw TableException("Wrong delete!");
			}
		}
		else if (type == 1)
		{
			float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Dataf*)key)->x == NowKey) && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 1;
				int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				break;
			}
			else
			{
				throw TableException("Wrong delete!");
			}
		}
		else
		{
			string NowKey;
			NowKey = (char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Datac*)key)->x.compare(NowKey)) == 0 && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				*(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) = 1;
				int buffernum = bf.GiveMeABlock(name, *(int*)(currentBlock + 4));
				memcpy(bf.bufferBlock[buffernum].values, currentBlock, BLOCKSIZE);
				break;
			}
		}

		if (i == NumOfKeys)
		{
			throw TableException("Wrong delete!");
		}
	}
	delete[]currentBlock;
}

int* index::Range(Data* key1, Data* key2)//查找范围中的键值（非确定值的索引实现）
{
	char *currentBlock = new char[BLOCKSIZE];

	int *re = new int[1000];
	re[0] = -1;
	int buffernum = bf.GiveMeABlock(name, 0);
	bf.useBlock(buffernum);
	memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
	//readbuffer;
	int LeafType = *(int*)(currentBlock);
	int Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	int tempBro, position;

	while (LeafType == Internal)
	{
		int NumOfKeys = *(int*)(currentBlock + 12);
		int i = 0;

		for (i = 0; i < NumOfKeys; i++)
		{
			if (type == 0)
			{
				int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datai*)key1)->x < NowKey))
					break;
			}
			else if (type == 1)
			{
				float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Dataf*)key1)->x < NowKey))
					break;
			}
			else
			{
				string NowKey((char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH)));
				tempBro = Brother;
				position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
				if ((((Datac*)key1)->x.compare(NowKey)) < 0)
					break;
			}
		}
		if (i == NumOfKeys)
			position = *(int*)(currentBlock + MESSAGE + tempBro*(keylength[type] + POINTERLENGTH) + keylength[type] + 12);
		if (position == -1)
		{
			return re;
		}
		int buffernum = bf.GiveMeABlock(name, position);
		bf.useBlock(buffernum);
		memcpy(currentBlock, bf.bufferBlock[buffernum].values, BLOCKSIZE);
		LeafType = *(int*)(currentBlock);
		Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
		//readbuffer
	}
	int NumOfKeys = *(int*)(currentBlock + 12);
	Brother = *(int*)(currentBlock + MESSAGE + keylength[type]);
	for (int i = 0; i < NumOfKeys; i++)//如果查找到范围键值的起始，则直接从下一个节点开始寻找其终止范围
	{
		if (type == 0)
		{
			int NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Datai*)key1)->x == NowKey) && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				int j = 0;
				while (Brother != -1 && NowKey <= ((Datai*)key2)->x)
				{
					re[j++] = Addr;
					Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
					NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
					Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				}
				re[j] = -1;
				delete[]currentBlock;
				return re;
			}
		}
		else if (type == 1)
		{
			float NowKey = *(float*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Dataf*)key1)->x == NowKey) && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				int j = 0;
				while (Brother != -1 && NowKey <= ((Dataf*)key2)->x)
				{
					re[j++] = Addr;
					Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
					NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
					Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				}
				re[j] = -1;
				delete[]currentBlock;
				return re;
			}
		}
		else
		{
			string NowKey((char*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH)));
			tempBro = Brother;
			int Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			position = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
			Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
			if ((((Datac*)key1)->x.compare(NowKey)) == 0 && *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 16) != 1)
			{
				int j = 0;
				while (Brother != -1 && (((Datac*)key2)->x.compare(NowKey)) >= 0)
				{
					re[j++] = Addr;
					Brother = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type]);
					NowKey = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH));
					Addr = *(int*)(currentBlock + MESSAGE + Brother*(keylength[type] + POINTERLENGTH) + keylength[type] + 4);
				}
				re[j] = -1;
				delete[]currentBlock;
				return re;
			}
		}
	}
	return re;
	delete[]currentBlock;
}

