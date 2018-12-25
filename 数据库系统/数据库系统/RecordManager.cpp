
#include "RecordManager.h"
#include "Interpreter.h"
#include "IndexManager.h"
#include <cmath>
RecordManager::~RecordManager()
{
}

bool RecordManager::isSatisfied(Table& tableinfor, tuper& row, vector<int> mask, vector<where> w)
{
	bool res = true;
	for (int i = 0; i < mask.size();i++){
		if (w[i].d == NULL){ //不存在where条件
			continue;
		}
		else if (row[mask[i]]->flag == -1) { //int
			switch (w[i].flag) {
			case eq: if (!(((Datai*)row[mask[i]])->x == ((Datai*)w[i].d)->x)) return false;break;
			case leq: if (!(((Datai*)row[mask[i]])->x <= ((Datai*)w[i].d)->x)) return false; break;
			case l: if (!(((Datai*)row[mask[i]])->x < ((Datai*)w[i].d)->x)) return false; break;
			case geq: if (!(((Datai*)row[mask[i]])->x >= ((Datai*)w[i].d)->x)) return false; break;
			case g: if (!(((Datai*)row[mask[i]])->x >((Datai*)w[i].d)->x)) return false; break;
			case neq: if (!(((Datai*)row[mask[i]])->x != ((Datai*)w[i].d)->x)) return false; break;
			default: ;
			}
		}
		else if (row[mask[i]]->flag == 0) { //Float
			switch (w[i].flag) {
			case eq: if (!(abs(((Dataf*)row[mask[i]])->x - ((Dataf*)w[i].d)->x)<MIN_Theta)) return false; break;
			case leq: if (!(((Dataf*)row[mask[i]])->x <= ((Dataf*)w[i].d)->x)) return false; break;
			case l: if (!(((Dataf*)row[mask[i]])->x < ((Dataf*)w[i].d)->x)) return false; break;
			case geq: if (!(((Dataf*)row[mask[i]])->x >= ((Dataf*)w[i].d)->x)) return false; break;
			case g: if (!(((Dataf*)row[mask[i]])->x >((Dataf*)w[i].d)->x)) return false; break;
			case neq: if (!(((Dataf*)row[mask[i]])->x != ((Dataf*)w[i].d)->x)) return false; break;
			default: ;
			}
		}
		else if (row[mask[i]]->flag > 0){ //string
			switch (w[i].flag) {
			case eq: if (!(((Datac*)row[mask[i]])->x == ((Datac*)w[i].d)->x)) return false; break;
			case leq: if (!(((Datac*)row[mask[i]])->x <= ((Datac*)w[i].d)->x)) return false; break;
			case l: if (!(((Datac*)row[mask[i]])->x < ((Datac*)w[i].d)->x)) return false; break;
			case geq: if (!(((Datac*)row[mask[i]])->x >= ((Datac*)w[i].d)->x)) return false; break;
			case g: if (!(((Datac*)row[mask[i]])->x >((Datac*)w[i].d)->x)) return false; break;
			case neq: if (!(((Datac*)row[mask[i]])->x != ((Datac*)w[i].d)->x)) return false; break;
			default: ;
			}
		}
		else { //just for debug
			cout << "Error in RecordManager in function is satisified!" << endl;
			system("pause");
		}
	}
	return res;
}

Table RecordManager::Select(Table& tableIn, vector<int>attrSelect, vector<int>mask, vector<where>& w)
{
	if (mask.size() == 0){
		return Select(tableIn,attrSelect);
	}
	string stringRow;

	string filename = tableIn.getname() + ".table";
	string indexfilename;
	int length = tableIn.dataSize() + 1;
	const int recordNum = BLOCKSIZE / length;

	//to find whether there is an index on selected key
	//IndexManager indexMA;
	//int inPos = -1;//index position
	//for (int i = 0; i < w.size();i++) {
	//	if (w[i].flag == eq){
	//		for (int j = 0; j < tableIn.index.num;j++) {
	//			if (tableIn.index.location[j] == mask[i]){	
	//				Data* ptrData;
	//				ptrData = w[i].d;
	//				indexfilename = tableIn.Tname + to_string(mask[i]) + ".index";
	//				inPos = indexMA.Find(indexfilename, ptrData);
	//				break;
	//			}
	//		}
	//		if (inPos != -1){
	//			break;
	//		}
	//	}
	//}/// finding.... 
	//if (inPos!=-1){ //result found
	//	int blockOffset = inPos / recordNum;
	//	int recordOffset = inPos % recordNum;
	//	int datalen = tableIn.dataSize();
	//	int bitOffset = recordOffset *(1+datalen);
	//	int blockNum;
	//	blockNum = bf.GiveMeABlock(filename, blockOffset);
	//	char *pdata;
	//	pdata = new char(datalen+1);
	//	memcpy(pdata, &(buf_ptr->bufferBlock[blockNum].values[bitOffset]), datalen+1);

	//	tuper *single_tuper = Char2Tuper(tableIn, pdata);
	//	//delete[] pdata;
	//	tableIn.addData(single_tuper);
	//	return SelectProject(tableIn, attrSelect);
	//}
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++){
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1){
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum;offset++){
			int position = offset * length;
			stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			tuper *temp_tuper = new tuper;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++){
				if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuper->addData(new Datai(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0){//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuper->addData(new Dataf(value));
				}
				else{
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index]+1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuper->addData(new Datac(string(value)));
				}
			}//以上内容先从文件中生成一行tuper，一下判断是否满足要求
                             
			if (isSatisfied(tableIn,*temp_tuper,mask,w)){
				tableIn.addData(temp_tuper); //可能会存在问题;solved!
			}
            else delete temp_tuper;
		}
	}
    return SelectProject(tableIn,attrSelect);
}

Table RecordManager::Select(Table& tableIn, vector<int>attrSelect)
{
	string stringRow;
	string filename = tableIn.getname() + ".table";
	tuper* temp_tuper;
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	const int recordNum = BLOCKSIZE / length; //一个block中存储的记录条数
	for (int blockOffset = 0; blockOffset < tableIn.blockNum;blockOffset++){//读取整个文件中的所有内容
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1){ //该块不再内存中，读取之
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum;offset++){
			int position = offset * length;
			stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
			if(stringRow.c_str()[0]==EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
            temp_tuper = new tuper;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num;attr_index++){
				if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuper->addData(new Datai(value));
				}
				else if (tableIn.getattribute().flag[attr_index]==0){//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuper->addData(new Dataf(value));
				}
				else{
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index]+1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
                    c_pos += strLen;
					temp_tuper->addData(new Datac(string(value)));
				}
			}           
			tableIn.addData(temp_tuper); //可能会存在问题;solved!
		}
	}
	return SelectProject( tableIn, attrSelect);
}



int RecordManager::FindWithIndex(Table& tableIn, tuper& row, int mask)
{
	IndexManager indexMA;
	for (int i = 0; i < tableIn.index.num; i++) {
		if (tableIn.index.location[i] == mask) { //找到索引
			Data* ptrData;
			ptrData = row[mask];
			int pos = indexMA.Find(tableIn.getname() + to_string(mask) + ".index", ptrData);
			return pos;
		}
	}
	return -1;
}

void RecordManager::Insert(Table& tableIn, tuper& singleTuper)
{
	//check Redundancy using index
	for (int i = 0; i < tableIn.attr.num; i++) {
		if (tableIn.attr.unique[i] == 1) {
			int addr = FindWithIndex(tableIn, singleTuper, i);
			if (addr >= 0) { //already in the table 
				throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
				return;
			}
		}
	}

	for (int i = 0; i < tableIn.attr.num;i++) {		
		if (tableIn.attr.unique[i]){
			vector<where> w;
			vector<int> mask;
			where *uni_w = new where;
			uni_w->flag = eq;
			switch (singleTuper[i]->flag) {
			case -1:uni_w->d = new Datai(((Datai*)singleTuper[i])->x); break;
			case 0:uni_w->d = new Dataf(((Dataf*)singleTuper[i])->x); break;
			default:uni_w->d = new Datac(((Datac*)singleTuper[i])->x); break;
			}
			w.push_back(*uni_w);
			mask.push_back(i);
			/*Table temp_table = Select(tableIn, mask, mask, w);
            
            
			if (temp_table.T.size() != 0) {
				throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
			}*/
            //code by hrg
            if(!UNIQUE(tableIn, w[0], i)){
                throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
            }
                
            //code by hrg
            
            delete uni_w->d;
            delete uni_w;
		}
	}

	char *charTuper;
	charTuper = Tuper2Char(tableIn, singleTuper);//把一个元组转换成字符串
	//判断是否unique
	insertPos iPos = buf_ptr->getInsertPosition(tableIn);//获取插入位置
    
	buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position] = NOTEMPTY;
	memcpy(&(buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position + 1]), charTuper, tableIn.dataSize());
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	//insert tuper into index file
	IndexManager indexMA;
	int blockCapacity = BLOCKSIZE / length;
	for (int i = 0; i < tableIn.index.num; i++) {
		int tuperAddr = buf_ptr->bufferBlock[iPos.bufferNUM].blockOffset*blockCapacity + iPos.position / length; //the tuper's addr in the data file
		for (int j = 0; j < tableIn.index.num; j++) {
			indexMA.Insert(tableIn.getname() + to_string(tableIn.index.location[j]) + ".index", singleTuper[tableIn.index.location[i]], tuperAddr);
		}
	}
	buf_ptr->writeBlock(iPos.bufferNUM);
    delete[] charTuper;

}

void RecordManager::InsertWithIndex(Table& tableIn, tuper& singleTuper)
{
	//check Redundancy using index
	for (int i = 0; i < tableIn.attr.num; i++) {
		if (tableIn.attr.unique[i] == 1) {
			int addr = FindWithIndex(tableIn, singleTuper, i);
			if (addr >= 0) { //already in the table 
				throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
				return;
			}
		}
	}

	for (int i = 0; i < tableIn.attr.num; i++) {
		if (tableIn.attr.unique[i]) {
			vector<where> w;
			vector<int> mask;
			where *uni_w = new where;
			uni_w->flag = eq;
			switch (singleTuper[i]->flag) {
			case -1:uni_w->d = new Datai(((Datai*)singleTuper[i])->x); break;
			case 0:uni_w->d = new Dataf(((Dataf*)singleTuper[i])->x); break;
			default:uni_w->d = new Datac(((Datac*)singleTuper[i])->x); break;
			}
			w.push_back(*uni_w);
			mask.push_back(i);
			Table temp_table = Select(tableIn, mask, mask, w);


			if (temp_table.T.size() != 0) {
			throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
			}
			//code by hrg

		/*	if (!UNIQUE(tableIn, w[0], i)) {
				throw QueryException("Unique Value Redundancy occurs, thus insertion failed");
			}*/

			//code by hrg

			delete uni_w->d;
			delete uni_w;
		}
	}
	tableIn.addData(&singleTuper);
	char *charTuper;
	charTuper = Tuper2Char(tableIn, singleTuper);//把一个元组转换成字符串
	//判断是否unique
	insertPos iPos = buf_ptr->getInsertPosition(tableIn);//获取插入位置

	buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position] = NOTEMPTY;
	memcpy(&(buf_ptr->bufferBlock[iPos.bufferNUM].values[iPos.position + 1]), charTuper, tableIn.dataSize());
	int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
	//insert tuper into index file
	IndexManager indexMA;
	int blockCapacity = BLOCKSIZE / length;
	for (int i = 0; i < tableIn.index.num; i++) {
		int tuperAddr = buf_ptr->bufferBlock[iPos.bufferNUM].blockOffset*blockCapacity + iPos.position / length; //the tuper's addr in the data file
		for (int j = 0; j < tableIn.index.num; j++) {
			indexMA.Insert(tableIn.getname() + to_string(tableIn.index.location[j]) + ".index", singleTuper[tableIn.index.location[i]], tuperAddr);
		}
	}
	buf_ptr->writeBlock(iPos.bufferNUM);
	delete[] charTuper;
}

char* RecordManager::Tuper2Char(Table& tableIn, tuper& singleTuper)
{
	char* ptrRes;
	int pos = 0;//当前的插入位置
    ptrRes = new char[(tableIn.dataSize() + 1)*sizeof(char)];
	for (int i = 0; i < tableIn.getattribute().num;i++){
		if (tableIn.getattribute().flag[i] == -1){ //int
			int value = ((Datai*)singleTuper[i])->x;
			memcpy(ptrRes + pos, &value, sizeof(int));
			pos += sizeof(int);
		}
		else if (tableIn.getattribute().flag[i] == 0){
			float value = ((Dataf*)singleTuper[i])->x;
			memcpy(ptrRes + pos, &value, sizeof(float));
			pos += sizeof(float);
		}
		else{ //string
			string value(((Datac*)singleTuper[i])->x);
			int strLen = tableIn.getattribute().flag[i] + 1;
			memcpy(ptrRes + pos, value.c_str(), strLen);//多加1，拷贝最后的'\0';
			pos += strLen;
		}
	}
	ptrRes[tableIn.dataSize()] = '\0';
	return ptrRes;
}

int RecordManager::Delete(Table& tableIn, vector<int>mask, vector<where> w)
{
	string filename = tableIn.getname() + ".table";
	string stringRow;
	
	int count = 0;
	int length = tableIn.dataSize() + 1;
	const int recordNum = BLOCKSIZE / length;
	for (int blockOffset = 0; blockOffset < tableIn.blockNum; blockOffset++){
		int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
		if (bufferNum == -1){
			bufferNum = buf_ptr->getEmptyBuffer();
			buf_ptr->readBlock(filename, blockOffset, bufferNum);
		}
		for (int offset = 0; offset < recordNum; offset++){
			int position = offset * length;
			stringRow = buf_ptr->bufferBlock[bufferNum].getvalues(position, position + length);
			if (stringRow.c_str()[0] == EMPTY) continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			tuper *temp_tuper = new tuper;
			for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++){
				if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
					int value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp_tuper->addData(new Datai(value));
				}
				else if (tableIn.getattribute().flag[attr_index] == 0){//float
					float value;
					memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp_tuper->addData(new Dataf(value));
				}
				else{
					char value[MAXSTRINGLEN];
					int strLen = tableIn.getattribute().flag[attr_index] + 1;
					memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp_tuper->addData(new Datac(string(value)));
				}
			}//以上内容先从文件中生成一行tuper，一下判断是否满足要求
               
			if (isSatisfied(tableIn,*temp_tuper,mask,w)){
				buf_ptr->bufferBlock[bufferNum].values[position] = DELETED; //DELETED==EMYTP
				buf_ptr->writeBlock(bufferNum);
				count++;
			}
		}
	}
	return count;
}

bool RecordManager::DropTable(Table& tableIn)
{
	string filename = tableIn.getname() + ".table";
	if (remove(filename.c_str()) != 0){
		throw TableException("Can't delete the file!\n");
	}
	else{
		buf_ptr->setInvalid(filename);
	}
	return true;
}

bool RecordManager::CreateTable(Table& tableIn)
{

	string filename = tableIn.getname() + ".table";
	fstream fout(filename.c_str(), ios::out);
	fout.close();
	tableIn.blockNum = 1;
	CataManager Ca;
	Ca.changeblock(tableIn.getname(), 1);
	return true;
}

Table RecordManager::SelectProject(Table& tableIn, vector<int>attrSelect)
{
	Attribute attrOut;
	tuper *ptrTuper=NULL;
	attrOut.num = attrSelect.size();
	for (int i = 0; i < attrSelect.size();i++){
		attrOut.flag[i] = tableIn.getattribute().flag[attrSelect[i]];
		attrOut.name[i] = tableIn.getattribute().name[attrSelect[i]];
		attrOut.unique[i] = tableIn.getattribute().unique[attrSelect[i]];
	}
	Table tableOut(tableIn.getname(), attrOut, tableIn.blockNum);
    int k;
	for (int i = 0; i < tableIn.T.size(); i++){//tuper的个数
		ptrTuper = new tuper;
		for (int j = 0; j < attrSelect.size();j++){
            k = attrSelect[j];
			Data *resadd =NULL;
			if (tableIn.T[i]->operator [](k)->flag == -1) {
				resadd = new Datai((*((Datai*)tableIn.T[i]->operator [](k))).x);
			}
			else if(tableIn.T[i]->operator [](k)->flag == 0){
				resadd = new Dataf((*((Dataf*)tableIn.T[i]->operator [](k))).x);
			}
			else if (tableIn.T[i]->operator [](k)->flag>0) {
				resadd = new Datac((*((Datac*)tableIn.T[i]->operator [](k))).x);
			}
			
			ptrTuper->addData(resadd);//bug

		}
		tableOut.addData(ptrTuper);
	}

	return tableOut;
}

tuper RecordManager::String2Tuper(Table& tableIn, string stringRow)
{
	tuper temp_tuper;
	if (stringRow.c_str()[0] == EMPTY) return temp_tuper;//该行是空的
	int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
	for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++){
		if (tableIn.getattribute().flag[attr_index] == -1){//是一个整数
			int value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(int));
			c_pos += sizeof(int);
			temp_tuper.addData(new Datai(value));
		}
		else if (tableIn.getattribute().flag[attr_index] == 0){//float
			float value;
			memcpy(&value, &(stringRow.c_str()[c_pos]), sizeof(float));
			c_pos += sizeof(float);
			temp_tuper.addData(new Dataf(value));
		}
		else{
			char value[MAXSTRINGLEN];
			int strLen = tableIn.getattribute().flag[attr_index] + 1;
			memcpy(value, &(stringRow.c_str()[c_pos]), strLen);
			c_pos += strLen;
			temp_tuper.addData(new Datac(string(value)));
		}
	}//以上内容先从文件中生成一行tuper
	return temp_tuper;
}
tuper* RecordManager::Char2Tuper(Table& tableIn, char* stringRow)
{
	tuper* temp_tuper;
	temp_tuper = new tuper;
	if (stringRow[0] == EMPTY) return temp_tuper;//该行是空的
	int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
	for (int attr_index = 0; attr_index < tableIn.getattribute().num; attr_index++) {
		if (tableIn.getattribute().flag[attr_index] == -1) {//是一个整数
			int value;
			memcpy(&value, &(stringRow[c_pos]), sizeof(int));
			c_pos += sizeof(int);
			temp_tuper->addData(new Datai(value));
		}
		else if (tableIn.getattribute().flag[attr_index] == 0) {//float
			float value;
			memcpy(&value, &(stringRow[c_pos]), sizeof(float));
			c_pos += sizeof(float);
			temp_tuper->addData(new Dataf(value));
		}
		else {
			char value[MAXSTRINGLEN];
			int strLen = tableIn.getattribute().flag[attr_index] + 1;
			memcpy(value, &(stringRow[c_pos]), strLen);
			c_pos += strLen;
			temp_tuper->addData(new Datac(string(value)));
		}
	}//以上内容先从文件中生成一行tuper
	return temp_tuper;
}


bool RecordManager::UNIQUE(Table& tableIn, where w, int loca){
    int length = tableIn.dataSize() + 1; //一个元组的信息在文档中的长度
    const int recordNum = BLOCKSIZE / length; //一个block中存储的记录条数
    string stringRow;
    string filename = tableIn.getname() + ".table";
    int attroff=1;
    for(int i=0;i<loca-1;i++){
        if(tableIn.attr.flag[i]==-1){
            attroff += sizeof(int);
        }
        else if(tableIn.attr.flag[i]==0){
            attroff += sizeof(float);
        }
        else{
            attroff += sizeof(char)*tableIn.attr.flag[i];
        }
    }
    int inflag = tableIn.attr.flag[loca];
    for (int blockOffset = 0; blockOffset < tableIn.blockNum;blockOffset++){//读取整个文件中的所有内容
        int bufferNum = buf_ptr->getIfIsInBuffer(filename, blockOffset);
        if (bufferNum == -1){ //该块不再内存中，读取之
            bufferNum = buf_ptr->getEmptyBuffer();
            buf_ptr->readBlock(filename, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < recordNum;offset++){
            int position = offset * length + attroff;
            if(inflag==-1){
                int value;
                memcpy(&value, &(bf.bufferBlock[bufferNum].values[position+4]), sizeof(int));
                if(value==((Datai*)(w.d))->x)
                    return false;
            }
            else if(inflag==0){
                float value;
                memcpy(&value, &(bf.bufferBlock[bufferNum].values[position+4]), sizeof(float));
                if(value==((Dataf*)(w.d))->x)
                    return false;
            }
            else{
                char value[100];
                memcpy(value, &(bf.bufferBlock[bufferNum].values[position+4]), tableIn.attr.flag[loca]+1);
                if(string(value)==((Datac*)(w.d))->x)
                    return false;
            }

        }
    }
                return true;
    
}


