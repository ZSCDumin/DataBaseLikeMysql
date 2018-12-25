#include "BufferManager.h"
#include "Catalog.h"
#include <stdio.h>

BufferManager::BufferManager()
{
	for (int i = 0; i< MAXBLOCKNUM; i++) 
		bufferBlock[i].initialize();
}

BufferManager::~BufferManager()
{
    //std::cout << "~buffer" << endl;
	for (int i = 0; i < MAXBLOCKNUM; i++)
		flashBack(i);
}

void BufferManager::flashBack(int bufferNum)
{
	if(!bufferBlock[bufferNum].isWritten) return;//如果没有被改写
	//被改写
	string filename = bufferBlock[bufferNum].filename;
	//fstream fout;
	FILE *fp;
	if ((fp = fopen(filename.c_str(),"r+b"))==NULL	){
		cout << "Open file error!" << endl; // #Todo
		return;
	}
	fseek(fp, BLOCKSIZE*bufferBlock[bufferNum].blockOffset, SEEK_SET);
	fwrite(bufferBlock[bufferNum].values, BLOCKSIZE, 1, fp);
	bufferBlock[bufferNum].initialize();
	fclose(fp);
	/*fout.open(filename.c_str(), ios::in | ios::out);
	fout.seekp(BLOCKSIZE*bufferBlock[bufferNum].blockOffset, fout.beg);
	fout.write(bufferBlock[bufferNum].values, BLOCKSIZE);
	bufferBlock[bufferNum].initialize();
	fout.close();*/
}

int BufferManager::getbufferNum(string filename, int blockOffset)
{
	int bufferNum = getIfIsInBuffer(filename, blockOffset);
	if (bufferNum == -1){
		bufferNum = getEmptyBufferExcept(filename);
		readBlock(filename, blockOffset, bufferNum);
	}
	return bufferNum;
}

void BufferManager::readBlock(string filename, int blockOffset, int bufferNum)
{
	bufferBlock[bufferNum].isValid = 1;
	bufferBlock[bufferNum].isWritten = 0;
	bufferBlock[bufferNum].filename = filename;
	bufferBlock[bufferNum].blockOffset = blockOffset;
	bufferBlock[bufferNum].recent_time = clock();

	FILE *fp;
	if ((fp = fopen(filename.c_str(),"rb"))==NULL){
		cout << "Open file error" << endl;
		return;
	}
	fseek(fp, BLOCKSIZE*blockOffset, SEEK_SET);
	fread(bufferBlock[bufferNum].values, BLOCKSIZE, 1, fp);
	fclose(fp);

	/*fstream  fin;
	fin.open(filename.c_str(), ios::in | ios::binary);
	fin.seekp(BLOCKSIZE*blockOffset, ios::beg);
	fin.read(bufferBlock[bufferNum].values, BLOCKSIZE);
	fin.close();*/
}

void BufferManager::writeBlock(int bufferNum)
{
	bufferBlock[bufferNum].isWritten = 1;
	useBlock(bufferNum);
}

void BufferManager::useBlock(int bufferNum)
{
	bufferBlock[bufferNum].recent_time = clock();
}

int BufferManager::getEmptyBuffer()
{
	int bufferNum = -1; //for invalid buffer block
	int least_recent_block = 0;//当前编号最小的空块的块号
	for (int i = 0; i < MAXBLOCKNUM;i++){
		if (!bufferBlock[i].isValid){
			bufferBlock[i].initialize();
			bufferBlock[i].isValid = 1;
			return i;
		}
		else if (bufferBlock[least_recent_block].recent_time > bufferBlock[i].recent_time){
			least_recent_block = i;
		}
	}
	//执行到这里，最不常访问的block下标为least_recent_block
	flashBack(least_recent_block);
	bufferBlock[least_recent_block].isValid = 1;
	return least_recent_block;
}

int BufferManager::getEmptyBufferExcept(string filename){
	int bufferNum = -1; //for invalid buffer block
	int least_recent_block = 0;
	for (int i = 0; i < MAXBLOCKNUM; i++){
		if (!bufferBlock[i].isValid){
			bufferBlock[i].initialize();
			bufferBlock[i].isValid = 1;
			return i;
		}
		else if (bufferBlock[least_recent_block].recent_time > bufferBlock[i].recent_time && bufferBlock[i].filename != filename){
			least_recent_block = i;
		}
	}
	//执行到这里，最不常访问的block下标为least_recent_block
	flashBack(least_recent_block);
	bufferBlock[least_recent_block].isValid = 1;
	return least_recent_block;
}

insertPos BufferManager::getInsertPosition(Table& tableinfor){
	insertPos iPos;
	if (tableinfor.blockNum == 0){ //new file and no block exist 
		iPos.bufferNUM = addBlockInFile(tableinfor);
		iPos.position = 0;
		return iPos;
	}
	string filename = tableinfor.getname() + ".table";
	int length = tableinfor.dataSize() + 1; //多余的一位放在开头，表示是否有效
	int blockOffset = tableinfor.blockNum - 1;//将新加入的元素插入到最后
	int bufferNum = getIfIsInBuffer(filename, blockOffset);
	if (bufferNum == -1){
		bufferNum = getEmptyBuffer();//获取空的block
		readBlock(filename, blockOffset, bufferNum);
	}
	int recordNum = BLOCKSIZE / length;
	for (int offset = 0; offset < recordNum;offset++){
		int position = offset * length;
		char isEmpty = bufferBlock[bufferNum].values[position];//检查第一位是否有效，判断该行是否有内容
		if (isEmpty == EMPTY){//find an empty space
			iPos.bufferNUM = bufferNum;
			iPos.position = position;
			return iPos;
		}
	}
	//该block已经装满，新开一个block
	iPos.bufferNUM = addBlockInFile(tableinfor);
	iPos.position = 0;
	return iPos;
}

int BufferManager::addBlockInFile(Table& tableinfor){
	int bufferNum = getEmptyBuffer();
	bufferBlock[bufferNum].initialize();
	bufferBlock[bufferNum].isValid = 1;
	bufferBlock[bufferNum].isWritten = 1;
	bufferBlock[bufferNum].filename = tableinfor.getname() + ".table";
	bufferBlock[bufferNum].blockOffset = tableinfor.blockNum++;
	bufferBlock[bufferNum].recent_time = clock();
	CataManager ca;
	ca.changeblock(tableinfor.getname(), tableinfor.blockNum);
	return bufferNum;
}

int BufferManager::getIfIsInBuffer(string filename, int blockOffset){
	for (int bufferNum = 0; bufferNum < MAXBLOCKNUM; bufferNum++)
		if (bufferBlock[bufferNum].filename == filename && bufferBlock[bufferNum].blockOffset == blockOffset)	return bufferNum;
	return -1; //内存中并没有这一块
}

void BufferManager::scanIn(Table tableinfo){
	string filename = tableinfo.getname() + ".table";
	fstream  fin(filename.c_str(), ios::in);
	for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++){
		if (getIfIsInBuffer(filename, blockOffset) == -1){	//block 不再内存中
			int bufferNum = getEmptyBufferExcept(filename);
			readBlock(filename, blockOffset, bufferNum);
		}
	}
	fin.close();
}

void BufferManager::setInvalid(string filename){
	for (int i = 0; i < MAXBLOCKNUM;i++){
		if (bufferBlock[i].filename == filename){
			bufferBlock[i].isValid = 0;
			bufferBlock[i].isWritten = 0;
		}
	}
}

int BufferManager::GiveMeABlock(string filename, int blockOffset)
{
	int bufferNum = getIfIsInBuffer(filename, blockOffset);
	if (bufferNum == -1) {
		bufferNum = getEmptyBuffer();
		readBlock(filename, blockOffset, bufferNum);
	}
	useBlock(bufferNum);
	return bufferNum;
}

