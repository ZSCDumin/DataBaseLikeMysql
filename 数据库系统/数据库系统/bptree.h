#ifndef _INDEX_H_
#define _INDEX_H_
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "BufferManager.h"

#define BLOCKSIZE 4096
#define MESSAGE 24
#define POINTERLENGTH 20
//block内结构，前20位记录当前节点信息，后键值信息分为四块：第一块为键值，第二块为兄弟键值位置
//第三块为叶子节点独有，记录文件对应位置；对应的内节点记录下一块地址(比自己小的)，第四块为当前编号位置,第五块专门记比自己大的地址
//节点第一部分空出特地用来记录第一个键值位置
//编号均从0开始

using namespace std;

enum nodetype{ Internal, Leaf };
extern BufferManager bf;
class index{
public:
	int Number;//记录当前index文件节点数目
	int keylength[3];//对应三种类型的键值：int float string
	
private:
	int maxchild;//b+树最大儿子个数
	int order;//阶数
	int type;//0 int; 1 double; 2 string 键值类型
	string name;//index文件名
public:
	index(string filename);
	~index(){};
	void initialize(Data* key, int Addr, int ktype);//初始化
	int find(Data* key);//搜索键值
	void insert(Data* key, int Addr);//插入
	int* split(char* currentBlock, Data* mid, Data* key, int Addr, int leftpos, int rightpos);//分裂b+树节点
	void Internal_insert(char* currentBlock, Data* mid, int leftpos, int rightpos);//向中间节点插入数据
	void SplitLeaf(char* block1, char*block2, char* currentBlock, Data* key, int Addr);//分裂b+树叶节点
	void SplitInternal(char* block1, char*block2, char* currentBlock, Data* mid, int leftpos, int rightpos); //分裂b + 树中间节点
	void Delete(Data* key);//删除键值
	int* Range(Data*key1, Data*key2);//查找范围中的键值
};

#endif
