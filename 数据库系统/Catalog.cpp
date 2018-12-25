#include <iostream>
#include <fstream>
#include <string>
#include "API.h"
#include "Catalog.h"

void CataManager::create_table(std::string s, Attribute atr, short primary, Index index){

	if (hasTable(s))
		throw TableException("ERROR in create_table: redefinition of table: " + s);
	if (primary >= 0)
		atr.unique[primary] = true;

	s = "T_" + s;
	int i;

	string filename = s;
	fstream fout(filename.c_str(), ios::out);
	fout.close();

	int No = bf.getbufferNum(s, 0);

	char* begin = bf.bufferBlock[No].values;
	//cout << begin[0] << begin[1] << begin[2];
	int pos = 0;

	memcpy(&begin[pos], &atr.num, sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &index.num, sizeof(int));
	pos = pos + sizeof(int);

	int bn = 0;
	memcpy(&begin[pos], &bn, sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &primary, sizeof(short));
	pos = pos + sizeof(short);


	for (i = 0; i < atr.num; i++){
		memcpy(&begin[pos], atr.name[i].data(), atr.name[i].length()*sizeof(char));
		pos = pos + (int)atr.name[i].length()*sizeof(char);
		memcpy(&begin[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}

	for (i = 0; i < atr.num; i++){
		memcpy(&begin[pos], &atr.flag[i], sizeof(short));
		pos = pos + sizeof(short);
	}

	for (i = 0; i < atr.num; i++){
		memcpy(&begin[pos], &atr.unique[i], sizeof(bool));
		pos = pos + sizeof(bool);
	}

	for (i = 0; i < index.num; i++){
		memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length()*sizeof(char));
		pos = pos + (int)index.indexname[i].length()*sizeof(char);
		memcpy(&begin[pos], "\0", sizeof(char));
		pos += sizeof(char);
	}

	for (i = 0; i < index.num; i++){
		memcpy(&begin[pos], &index.location[i], sizeof(short));
		pos = pos + sizeof(short);
	}

	//    for(int j=0;j<100;j++)
	//      cout << (int)begin[j] << "  ";

	bf.writeBlock(No);

	Table* t = getTable(s.substr(2, s.length() - 2));
	API api;
	api.CreateTable(*t);
	delete t;

}

bool CataManager::hasTable(std::string s){
	s = "T_" + s;
	std::ifstream in(s);
	if (!in){
		return false;
	}
	else {
		in.close();
		return true;
	}
}

Table* CataManager::getTable(std::string s){
	std::string s1 = "T_" + s;
	if (!hasTable(s))
		throw TableException("ERROR in getTable: No table named " + s);

	Attribute atr;
	Index ind;
	int pri;
	int bn;
	int No = bf.getbufferNum(s1, 0);
	char* begin = bf.bufferBlock[No].values;
	int pos = 0;

	memcpy(&atr.num, &begin[pos], sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&ind.num, &begin[pos], sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&bn, &begin[pos], sizeof(int));
	pos = pos + sizeof(int);
	memcpy(&pri, &begin[pos], sizeof(short));
	pos = pos + sizeof(short);
	int pos1 = pos;
	char temp[20];
	int i;
	for (i = 0; i < atr.num; i++){
		while (begin[pos1] != '\0')
			pos1 += sizeof(char);
		memcpy(temp, &begin[pos], (pos1 - pos + 1)*sizeof(char));
		atr.name[i] = temp;
		pos = pos1 + sizeof(char);
		pos1 = pos;
	}

	for (i = 0; i < atr.num; i++){
		memcpy(&atr.flag[i], &begin[pos], sizeof(short));
		pos = pos + sizeof(short);
	}

	for (i = 0; i < atr.num; i++){
		memcpy(&atr.unique[i], &begin[pos], sizeof(bool));
		pos = pos + sizeof(bool);
	}

	pos1 = pos;
	for (i = 0; i < ind.num; i++){
		while (begin[pos1] != '\0')
			pos1 += 1;
		memcpy(temp, &begin[pos], (pos1 - pos + 1)*sizeof(char));
		ind.indexname[i] = temp;
		pos = pos1 + sizeof(char);
		pos1 = pos;
	}

	for (i = 0; i < ind.num; i++){
		memcpy(&ind.location[i], &begin[pos], sizeof(short));
		pos = pos + sizeof(short);
	}

	bf.useBlock(No);

	Table* t = new Table(s, atr, bn);


	t->Copyindex(ind);
	t->setprimary(pri);
	return t;
}


void CataManager::create_index(std::string tname, std::string aname, std::string iname){
	Table* temp = getTable(tname);
	try{
		int i;
		for (i = 0; i < temp->getCsize(); i++){
			if (temp->attr.name[i] == aname)
				break;
		}
		if (i == temp->getCsize())//列数目
			throw TableException("No attribute named " + aname);
		if (temp->attr.unique[i]==0)//列名称重复
			throw TableException("This attribute is not unique!");
		temp->setindex(i, iname);

		int No = bf.getbufferNum("T_" + temp->getname(), 0);

		char* begin = bf.bufferBlock[No].values;
		//cout << begin[0] << begin[1] << begin[2];
		int pos = 0;
		Attribute atr = temp->attr;
		Index index = temp->index;
		short primary = temp->primary;

		memcpy(&begin[pos], &atr.num, sizeof(int)); //由src所指内存区域复制count个字节到dest所指内存区域
		pos = pos + sizeof(int);
		memcpy(&begin[pos], &index.num, sizeof(int));
		pos = pos + sizeof(int);

		int bn = 0;
		memcpy(&begin[pos], &bn, sizeof(int));
		pos = pos + sizeof(int);
		memcpy(&begin[pos], &primary, sizeof(short));
		pos = pos + sizeof(short);


		for (i = 0; i < atr.num; i++){
			memcpy(&begin[pos], atr.name[i].data(), atr.name[i].length()*sizeof(char));
			pos = pos + (int)atr.name[i].length()*sizeof(char);
			memcpy(&begin[pos], "\0", sizeof(char));
			pos += sizeof(char);
		}

		for (i = 0; i < atr.num; i++){
			memcpy(&begin[pos], &atr.flag[i], sizeof(short));
			pos = pos + sizeof(short);
		}

		for (i = 0; i < atr.num; i++){
			memcpy(&begin[pos], &atr.unique[i], sizeof(bool));
			pos = pos + sizeof(bool);
		}

		for (i = 0; i < index.num; i++){
			memcpy(&begin[pos], index.indexname[i].data(), index.indexname[i].length()*sizeof(char));
			pos = pos + (int)index.indexname[i].length()*sizeof(char);
			memcpy(&begin[pos], "\0", sizeof(char));
			pos += sizeof(char);
		}

		for (i = 0; i < index.num; i++){
			memcpy(&begin[pos], &index.location[i], sizeof(short));
			pos = pos + sizeof(short);
		}

		//    for(int j=0;j<100;j++)
		//      cout << (int)begin[j] << "  ";

		bf.writeBlock(No);


		delete temp;
	}
	catch (TableException e1){
		delete temp;
		throw e1;
	}
}

void CataManager::drop_table(std::string t){
	if (!hasTable(t))
		throw TableException("ERROR in drop_table: No table named " + t);
	Table* tb = getTable(t);
	API api;
	api.DropTable(*tb);
	remove(("T_" + t).c_str());
}

void CataManager::drop_index(std::string tname, std::string iname){
	Table* temp = getTable(tname);
	try{
		temp->dropindex(iname);
		drop_table(tname);
		create_table(tname, temp->attr, temp->primary, temp->index);
		delete temp;
	}
	catch (TableException e1){
		delete temp;
		throw e1;
	}
}

void CataManager::show_table(std::string tname){
	Table* t = getTable(tname);
	std::cout << tname << ":" << std::endl;
	Index ind;
	ind = t->Getindex();
	int i;
	for (i = 0; i < t->getCsize(); i++){
		std::cout << t->attr.name[i] << " ";
		if (t->attr.flag[i] == -1)
			std::cout << "int" << " ";
		else if (t->attr.flag[i] == 0)
			std::cout << "float" << " ";
		else
			std::cout << "char(" << t->attr.flag[i] << ") ";
		if (t->attr.unique[i] == 1)
			std::cout << "unique ";
		if (i == t->primary)
			std::cout << "primary key";
		std::cout << std::endl;
	}
	if (ind.num > 0){
		std::cout << "index: ";
		for (i = 0; i < ind.num; i++){
			std::cout << ind.indexname[i] << "(" << t->attr.name[ind.location[i]] << ") ";
		}
		std::cout << std::endl;
	}
	delete t;


}


void CataManager::changeblock(std::string tname, int bn){
	string s = "T_" + tname;
	int No = bf.getbufferNum(s, 0);
	char* begin = bf.bufferBlock[No].values;
	//cout << begin[0] << begin[1] << begin[2];
	int pos = 0;
	pos = pos + sizeof(int);
	pos = pos + sizeof(int);
	memcpy(&begin[pos], &bn, sizeof(int));

	bf.writeBlock(No);

}



