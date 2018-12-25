#ifndef _INDEXMANAGER_H_
#define _INDEXMANAGER_H_
#include<string>
#include<iostream>
#include<fstream>
#include"bptree.h"
#include"base.h"

class IndexManager{
public:
	IndexManager(){};
	void Establish(string file);
	void Insert(string file, Data* key, int Addr);
	void Delete(string file, Data* key);
	int Find(string file, Data* key);
	void Drop(string file);
	int*Range(string file, Data*key1, Data*key2);
	~IndexManager(){};
};

#endif
