#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H
#include "base.h"
#include "const.h"
#include <fstream>
#include <time.h>
using namespace std;

class insertPos{
public:
	int bufferNUM;//���ڴ��еĵڼ�������
	int position; //�������е�λ��
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
	void flashBack(int bufferNum);//��block����д���ļ�
	int getbufferNum(string filename, int blockOffset);//��ȡָ��block���ڴ��еı��
	void readBlock(string filename, int blockOffset, int bufferNum); //���ļ����ȡ��block��
	void writeBlock(int bufferNum);//block����дʱ���ã������
	void useBlock(int bufferNum); //ʹ��block
	int getEmptyBuffer();//Ѱ���ڴ��пյ�block
	int getEmptyBufferExcept(string filename);
	insertPos getInsertPosition(Table& tableinfor);//���ز������ݵĿ���λ��
	int addBlockInFile(Table& tableinfor);//�ļ�������µ�block�������µ�block���ڴ��еı��
	//int addBlockInFile(Index& indexinfor);
	int getIfIsInBuffer(string filename, int blockOffset);//�ҵ�ָ��block���ڴ��еı��
	void scanIn(Table tableinfo);//���������ļ�ȫ���������ڴ��У����ã�����ļ�̫�����е�blockռ���������꣬���������������
	void setInvalid(string filename);
	int GiveMeABlock(string filename, int blockOffset); //�ⲿ�������ýӿ�

	friend class RecordManager;
	friend class CataManager;
//private:
	buffer bufferBlock[MAXBLOCKNUM];
};


#endif

