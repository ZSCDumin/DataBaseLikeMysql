#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H
#include "base.h"
#include "const.h"
#include <fstream>
#include <time.h>
using namespace std;

class insertPos{
public:
	int bufferNUM;//在内存中的第几个区块
	int position; //在区块中的位置
};
class buffer{
public:
	bool isWritten;
	bool isValid;
	string filename;
	int blockOffset;	//block offset in file, indicate position in file
	int LRUvalue;		
	int recent_time; //show the recent visit time
	char values[BLOCKSIZE + 1];
	buffer(){
		initialize();
	}
	void initialize(){
		isWritten = 0;
		isValid = 0;
		filename = "NULL";
		blockOffset = 0;
		LRUvalue = 0;
		memset(values,EMPTY,BLOCKSIZE); 
		values[BLOCKSIZE] = '\0';
	}
	string getvalues(int startpos, int endpos){
		string tmpt = "";
		if (startpos >= 0 && startpos <= endpos && endpos <= BLOCKSIZE)
			for (int i = startpos; i < endpos; i++)
				tmpt += values[i];
		return tmpt;
	}
	char getvalues(int pos){
		if (pos >= 0 && pos <= BLOCKSIZE)
			return values[pos];
		return '\0';
	}
};

class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	void flashBack(int bufferNum);//将block立刻写入文件
	int getbufferNum(string filename, int blockOffset);//获取指定block在内存中的编号
	void readBlock(string filename, int blockOffset, int bufferNum); //将文件块读取到block中
	void writeBlock(int bufferNum);//block被改写时调用，做标记
	void useBlock(int bufferNum); //使用block
	int getEmptyBuffer();//寻找内存中空的block
	int getEmptyBufferExcept(string filename);
	insertPos getInsertPosition(Table& tableinfor);//返回插入数据的可行位置
	int addBlockInFile(Table& tableinfor);//文件后插入新的block，返回新的block在内存中的编号
	//int addBlockInFile(Index& indexinfor);
	int getIfIsInBuffer(string filename, int blockOffset);//找到指定block在内存中的编号
	void scanIn(Table tableinfo);//把整个表文件全部都读入内存中（慎用，如果文件太大，所有的block占满都读不完，会崩溃。。。。）
	void setInvalid(string filename);
	int GiveMeABlock(string filename, int blockOffset); //外部函数调用接口

	friend class RecordManager;
	friend class CataManager;
//private:
	buffer bufferBlock[MAXBLOCKNUM];
};


#endif

