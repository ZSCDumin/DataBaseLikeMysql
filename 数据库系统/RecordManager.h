#ifndef _RECORDMANAGER_H
#define	_RECORDMANAGER_H
#include "base.h"
#include "const.h"
#include "BufferManager.h"
#include "Catalog.h"
class RecordManager
{
public:
	RecordManager(BufferManager *bf):buf_ptr(bf){}
	~RecordManager();
	bool isSatisfied(Table& tableinfor, tuper& row, vector<int> mask, vector<where> w);
	Table Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where>& w);
	Table Select(Table& tableIn, vector<int>attrSelect);
	bool CreateIndex(Table& tableIn, int attr);
	int FindWithIndex(Table& tableIn, tuper& row, int mask);
	void Insert(Table& tableIn, tuper& singleTuper);
	void Update(Table& tableIn, tuper& singleTuper);
	tuper* Char2Tuper(Table& tableIn, char* stringRow);
	void InsertWithIndex(Table& tableIn, tuper& singleTuper);
	char* Tuper2Char(Table& tableIn, tuper& singleTuper);
	int Delete(Table& tableIn, vector<int>mask, vector<where> w);
	bool DropTable(Table& tableIn);
	bool CreateTable(Table& tableIn);
	Table SelectProject(Table& tableIn, vector<int>attrSelect);
	tuper String2Tuper(Table& tableIn, string stringRow);
    bool UNIQUE(Table& tableinfo, where w, int loca);
private:
	RecordManager(){} //产生rm的时候必须把bufferManager的指针赋值给它
	BufferManager *buf_ptr;
};

#endif

