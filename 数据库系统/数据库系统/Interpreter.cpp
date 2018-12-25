#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
#include "Catalog.h"
#include "Interpreter.h"
#include "API.h"


using namespace std;
int InterManager::EXEC(){
	Normolize();//处理查询字符串
	//cout << qs << endl;
	if (qs.substr(0, 6) == "select"){
		EXEC_SELECT();
		cout << "Interpreter: successful select!" << endl;
		return 1;
	}

	else if (qs.substr(0, 4) == "drop"){
		EXEC_DROP();
		cout << "Interpreter: successful drop!" << endl;
		return 1;
	}

	else if (qs.substr(0, 6) == "insert"){
		EXEC_INSERT();
		cout << "Interpreter: successful insert!" << endl;
		return 1;
	}

	else if (qs.substr(0, 6) == "create"){
		EXEC_CREATE();
		cout << "Interpreter: successful create!" << endl;
		return 1;
	}

	else if (qs.substr(0, 6) == "delete"){
		EXEC_DELETE();
		cout << "Interpreter: successful delete!" << endl;
		return 1;
	}

	else if (qs.substr(0, 10) == "show table"){
		EXEC_SHOW();
		return 1;
	}

	else if (qs.substr(0, 4) == "exit"){
		EXEC_EXIT();
		return 0;
	}

	else if (qs.substr(0, 9) == "execfile:"){
		EXEC_FILE();
		return 1;
	}

	else if (qs.substr(0, 6) == "update"){
		EXEC_UPDATE();
		cout << "Interpreter: successful update!" << endl;
		return 1;
	}

	else
		throw QueryException("ERROR: invalid query format!");
}

/*获取查询语句*/
void InterManager::GetQs(){
	string temp;
	qs = "";
	do{
		getline(cin, temp);
		qs = qs + " ";
		qs = qs + temp;
	} while (qs[qs.length() - 1] != ';');
	//cout << qs;
}

/*标准化查询语句*/
void InterManager::Normolize(){
	int pos;

	for (pos = 0; pos < qs.length(); pos++){
		if (qs[pos] == '<' || qs[pos] == '>' || qs[pos] == '=' || qs[pos] == '(' || qs[pos] == ')' ||
			qs[pos] == ';' || qs[pos] == ',' || qs[pos] == '*')
		{
			if (qs[pos - 1] != ' ')
			{
				qs.insert(pos, " ");
				pos++;
			}

			if (qs[pos + 1] != ' ')
			{
				qs.insert(pos + 1, " ");
				pos++;
			}

		}
	}
	//cout << qs;

	//去除字符串中间的多个空格，比如 a  b -> a b 
	int flag = 0;
	string::iterator it;
	for (it = qs.begin(); it < qs.end(); it++){
		if (flag == 0 && (*it == ' ' || *it == '\t'))
		{
			flag = 1;
			continue;
		}
		if (flag == 1 && (*it == ' ' || *it == '\t')){
			qs.erase(it);
			if (it != qs.begin())
				it--;
			continue;
		}
		if (*it != ' ' && *it != '\t'){
			flag = 0;
			continue;
		}
	}

	//去除头部和尾部的空格
	if (qs[0] == ' ')
		qs.erase(qs.begin());
	if (qs[qs.length() - 1] == ' ')
		qs.erase(--qs.end());

	//cout << qs;
}


void InterManager::EXEC_DROP(){
	int pos, pos1;
	string temp;
	CataManager cm;
	if (qs.substr(0, 10) == "drop table"){
		pos = 11;
		pos1 = GOGOGO(pos);
		temp = qs.substr(pos, pos1 - pos);
		Table* t = cm.getTable(temp);

		if (t->index.num > 0){
			for (int j = 0; j < t->index.num; j++){
				cm.drop_index(temp, t->index.indexname[j]);
			}
		}

		if (qs[pos1 + 1] != ';')
		{
			delete t;
			throw QueryException("ERROR: invalid query format!");
		}


		delete  t;
		cm.drop_table(temp);
		//code:drop the data in datafile!
	}

	else if (qs.substr(0, 10) == "drop index"){
		pos = 11;
		pos1 = GOGOGO(pos);
		temp = qs.substr(pos, pos1 - pos);
		string tname;
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "on")
		{
			throw QueryException("ERROR: invalid query format!");
		}

		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		tname = qs.substr(pos, pos1 - pos);

		if (qs[pos1 + 1] != ';')
			throw QueryException("ERROR: invalid query format!");

		cm.drop_index(tname, temp);
		//code:delete the index


	}

	else throw QueryException("ERROR: invalid query format!");
}


void InterManager::EXEC_EXIT(){

}

void InterManager::EXEC_CREATE(){
	if (qs.substr(0, 13) == "create table ")
		EXEC_CREATE_TABLE();
	else if (qs.substr(0, 13) == "create index ")
		EXEC_CREATE_INDEX();
	else
		throw QueryException("ERROR: invalid query format!");
}

void InterManager::EXEC_INSERT(){
	int pos, pos1;
	int i;
	if (qs.substr(0, 12) != "insert into ")
		throw QueryException("ERROR: invalid query format!");
	pos = 12;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "values")
		throw QueryException("ERROR: invalid query format!");

	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "(")
		throw QueryException("ERROR: invalid query format!");
	pos = pos1 + 1;
	tuper* tp = new tuper();
	string temp;
	CataManager cm;
	float p;
	Table* tb = cm.getTable(tname);
	Attribute attr = tb->getattribute();
	Data* dtemp;
	try{
		while (1){
			pos1 = GOGOGO(pos);
			//int
			temp = qs.substr(pos, pos1 - pos);
			if (attr.flag[tp->length()] == -1){
				if (!To_int(temp, i))
					throw QueryException("ERROR: " + temp + " is not a (int)!");
				dtemp = new Datai(i);
				tp->addData(dtemp);
			}

			//float
			else if (attr.flag[tp->length()] == 0){
				if (!To_float(temp, p))
					throw QueryException("ERROR: " + temp + "is not a (float)!");
				dtemp = new Dataf(p);
				tp->addData(dtemp);
			}
			//char
			else{




				if (temp[0] != '\'')
					throw QueryException("ERROR: " + temp + " is not a (string)");
				while (qs[pos1 + 1] != ',' && qs[pos1 - 1] != '\'')
				{
					pos = pos1 + 1;
					pos1 = GOGOGO(pos);
					temp += " ";
					temp += qs.substr(pos, pos1 - pos);
				}

				if (temp[temp.length() - 1] != '\'')
					throw QueryException("ERROR: " + temp + " is not a (string)");


				temp.erase(temp.begin());
				temp.erase(--temp.end());
				dtemp = new Datac(temp);
				//dtemp->flag = 1;
				tp->addData(dtemp);
			}

			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			if (qs.substr(pos, pos1 - pos) != ","){
				if (qs.substr(pos, pos1 - pos) != ")")
					throw QueryException("ERROR: invalid query format!");
				else break;
			}

			pos = pos1 + 1;
		}
		if (qs[pos1 + 1] != ';')
			throw QueryException("ERROR: invalid query format!");


	}
	catch (QueryException qe){
		delete tb;
		delete tp;
		throw qe;

	}
	//tb->blockNum = 1;
	API api;
	api.Insert(*tb, *tp);

	//
	//  tb->addData(tp);

	//debug
	//    tb->disp();
	//debug

	delete tb;
	//delete tp;
}

void InterManager::EXEC_SELECT(){
	int pos = 7;
	int pos1 = GOGOGO(pos);
	CataManager cm;
	vector<int> attrselect;//the index of selected attributes
	vector<int> attrwhere;//the index of where attributes
	vector<where> w;
	string temp[32];//attribute name of selected attribute
	string temp0;
	int countselect = 0;
	if (qs.substr(pos, pos1 - pos) == "*"){
		countselect = -1;//flag : select all
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "from")
			throw QueryException("ERROR: invalid query format!");
	}
	else{
		while (qs.substr(pos, pos1 - pos) != "from"){
			temp[countselect] = qs.substr(pos, pos1 - pos);
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			if (qs.substr(pos, pos1 - pos) != ","){
				countselect++;
				if (qs.substr(pos, pos1 - pos) == "from")
					break;
				throw QueryException("ERROR: invalid query format!");
			}

			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			countselect++;
		}
	}

	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	Table* t = cm.getTable(tname);
	int i, j;
	Attribute A = t->getattribute();
	if (countselect > 0){
		for (i = 0; i < countselect; i++){
			for (j = 0; j < t->getCsize(); j++)
			if (A.name[j] == temp[i]){
				attrselect.push_back(j);
				break;
			}

			if (j == t->getCsize())
				throw QueryException("No attribute named " + temp[i]);
		}
	}
	else {
		for (i = 0; i < A.num; i++)
			attrselect.push_back(i);
	}
	if (qs[pos1 + 1] != ';'){
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "where")
			throw  QueryException("ERROR: invalid query format!");
		interwhere(pos1, attrwhere, w, A, t);
	}

	/*
		cout << "location of selected attr:\n";
		for(int i = 0;i<attrselect.size();i++)
		cout << attrselect[i] << " ";
		cout << "\n";

		cout << "location of where attr:\n";
		for(int i = 0;i<attrwhere.size();i++)
		cout << attrselect[i] << " ";
		cout << "\n";

		for(int i = 0;i<w.size();i++)
		cout << w[i].flag << " ";
		cout << "\n";

		for(int i = 0;i<w.size();i++){
		if(w[i].d->flag==-1)
		cout << ((Datai*)(w[i].d))->x << " ";
		if(w[i].d->flag==0)
		cout << ((Dataf*)(w[i].d))->x << " ";
		if(w[i].d->flag>0)
		cout << ((Datac*)(w[i].d))->x << " ";
		}
		cout << endl;
		*/

	API api;
	//t->blockNum = 1;
	Table output = api.Select(*t, attrselect, attrwhere, w);
	output.disp();
	//   throw std::bad_alloc();
	//delete t; //#Todo 不应该删


}

void InterManager::EXEC_DELETE(){
	int pos = 7;
	int pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "from")
		throw QueryException("ERROR: invalid query format!");

	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	CataManager cm;
	Table* t = cm.getTable(tname);
	vector<int> attrwhere;
	vector<where> w;
	Attribute A = t->getattribute();
	if (qs[pos1 + 1] != ';'){
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "where")
			throw  QueryException("ERROR: invalid query format!");
		interwhere(pos1, attrwhere, w, A, t);
	}
	//
	//
	/*
	int i;
	cout << attrwhere.size() << " " << up.length() << endl;
	for(i=0;i<attrwhere.size();i++)
	cout << attrwhere[i] << endl;
	up.disptuper();
	down.disptuper();
	*/
	API api;
	//t->blockNum = 1;
	api.Delete(*t, attrwhere, w);
	delete t;

}



int InterManager::GOGOGO(int pos){
	int pos1 = pos;
	while (pos1 < qs.length() && qs[pos1] != ' ')
		pos1++;
	if (pos1 == qs.length())
		throw QueryException("ERROR: invalid query format!");
	return pos1;
}


void InterManager::EXEC_CREATE_TABLE(){
	int i, k;
	int pos = 13;
	int pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	pos = pos1 + 1;
	pos1 = pos + 2;
	//cout << endl;
	//cout <<qs.substr(pos,pos1-pos);
	if (qs.substr(pos, pos1 - pos) != "( ")
		throw QueryException("ERROR: invalid query format!");
	Attribute attr;
	attr.num = 0;
	for (i = 0; i < 32; i++)
		attr.unique[i] = false;
	pos = pos + 2;
	string temp;
	int priflag = 0;
	while (1){
		pos1 = GOGOGO(pos);
		temp = qs.substr(pos, pos1 - pos);
		if (temp == "primary"){
			priflag = 1;
			break;
		}
		attr.name[attr.num] = temp;
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		temp = qs.substr(pos, pos1 - pos);
		if (temp == "int"){
			attr.flag[attr.num] = -1;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
		}
		else if (temp == "float"){
			attr.flag[attr.num] = 0;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
		}
		else if (temp == "char"){
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			if (qs[pos] != '(')
				throw QueryException("ERROR: invalid query format!");
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			k = 0;
			for (i = pos1 - 1; i >= pos; i--){
				if (qs[i] <= '9' && qs[i] >= '0')
					k += pow(10, pos1 - 1 - i)*(qs[i] - '0');
				else
					throw QueryException("ERROR: invalid query format!");
			}
			attr.flag[attr.num] = k;
			pos = pos1 + 1;
			if (qs.substr(pos, 2) != ") ")
				throw QueryException("ERROR: invalid query format!");
			pos = pos + 2;
			pos1 = GOGOGO(pos);

		}
		else{
			throw QueryException("ERROR: invalid query format!");
		}

		if (qs[pos] == ')'){
			attr.num++;
			break;
		}
		else if (qs.substr(pos, 6) == "unique"){
			attr.unique[attr.num] = 1;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			if (qs[pos] == ')'){
				break;
			}
			else if (qs[pos] == ','){
				attr.num++;
				pos = pos1 + 1;
				continue;
			}
			else throw QueryException("ERROR: invalid query format!");
		}
		else if (qs[pos] == ','){
			attr.num++;
			pos = pos1 + 1;
			continue;
		}
		else{
			throw QueryException("ERROR: invalid query format!");
		}

	}

	//Table T(tname,attr,0);
	string pmk;
	if (priflag){
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "key")
			throw QueryException("ERROR: invalid query format!");
		pos = pos1 + 1;
		if (qs.substr(pos, 2) != "( ")
			throw QueryException("ERROR: invalid query format!");
		pos = pos + 2;
		pos1 = GOGOGO(pos);
		pmk = qs.substr(pos, pos1 - pos);
		pos = pos1 + 1;
		if (qs.substr(pos, 2) != ") ")
			throw QueryException("ERROR: invalid query format!");
		pos = pos + 2;
		if (qs[pos] != ')')
			throw QueryException("ERROR: invalid query format!");
		pos1 = pos + 1;
	}

	if (qs[pos1 + 1] != ';')
		throw QueryException("ERROR: invalid query format!");

	short pid;
	if (priflag){
		for (pid = 0; pid < attr.num; pid++){
			if (attr.name[pid] == pmk)
				break;
		}
	}
	else pid = -1;

	CataManager cm;
	Index ind;
	if (pid == attr.num || !priflag)
		pid = -1;
	else
		attr.unique[pid] = 1;
	ind.num = 0;
	cm.create_table(tname, attr, pid, ind);
	if (pid != -1){
		cm.create_index(tname, attr.name[pid], attr.name[pid]);
	}

}

void InterManager::EXEC_CREATE_INDEX(){
	CataManager cm;
	int pos = 13;
	int pos1 = GOGOGO(pos);
	string iname = qs.substr(pos, pos1 - pos);//索引名称
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "on")
		throw QueryException("ERROR: invalid query format!");
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);//索引字段表名
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != "(")
		throw QueryException("ERROR: invalid query format!");
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string aname = qs.substr(pos, pos1 - pos);//索引字段名
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	if (qs.substr(pos, pos1 - pos) != ")" || qs[pos1 + 1] != ';')
		throw QueryException("ERROR: invalid query format!");
	cm.create_index(tname, aname, iname);
}

void InterManager::EXEC_SHOW(){
	CataManager cm;
	string temp;
	int pos = 11;
	int pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	if (qs[pos1 + 1] != ';')
		throw QueryException("ERROR: invalid query format!");

	cm.show_table(tname);
}

void InterManager::EXEC_UPDATE(){
	int pos = 6;
	CataManager cm;
	vector<int> attrselect;//the index of selected attributes
	vector<where> w_select;//the index of selected attributes
	vector<int> attrwhere;//the index of where attributes
	vector<where> w_where;
	string temp[32];//attribute name of selected attribute
	string temp0;
	int countselect = 0;
	pos++;
	int pos1 = GOGOGO(pos);
	string tname = qs.substr(pos, pos1 - pos);
	Table* t = cm.getTable(tname);
	Attribute A = t->getattribute();
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	interwhere(pos1, attrselect, w_select, A, t);
	pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	interwhere(pos1, attrwhere, w_where, A, t);
	cout << "更新功能还有待完善...." << endl;
}

bool To_int(string s, int& a){
	int i;
	a = 0;
	for (i = 0; i < s.length(); i++){
		if (s[i] <= '9' && s[i] >= '0')
			a += pow(10, s.length() - i - 1)*(s[i] - '0');
		else return false;
	}
	return true;
}

bool To_float(string s, float& a){
	int i;
	a = 0;
	int dot;
	for (i = 0; i < s.length(); i++)
	if (s[i] == '.')
		break;
	int j = 0;
	if (i == s.length()){
		if (To_int(s, j))
		{
			a = j;
			return true;
		}
		else return false;
	}

	dot = i;
	for (i = 0; i < dot; i++){
		if (s[i] <= '9' && s[i] >= '0')
			a += pow(10, dot - i - 1)*(s[i] - '0');
		else return false;
	}
	for (i = dot + 1; i < s.length(); i++){
		if (s[i] <= '9' && s[i] >= '0')
			a += pow(0.1, i - dot)*(s[i] - '0');
		else return false;
	}
	return true;
}



//when pos1 is point to the space after "where".
void InterManager::interwhere(int& pos1, vector<int> &attrwhere, vector<where> &w, Attribute A, Table* t)
{
	int flag = 0;
	int x = 0;
	float y = 0;
	string z;
	Data* m;

	int pos = pos1 + 1;
	pos1 = GOGOGO(pos);
	string temp0;
	int j;
	where temp;
	while (1){

		temp0 = qs.substr(pos, pos1 - pos);
		for (j = 0; j < t->getCsize(); j++)
		if (A.name[j] == temp0){
			flag = A.flag[j];
			attrwhere.push_back(j);
			break;
		}
		if (j == t->getCsize())
			throw QueryException("No attribute named " + temp0);
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);

		if (qs.substr(pos, pos1 - pos) == "="){
			temp.flag = eq;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			temp0 = qs.substr(pos, pos1 - pos);

			if (flag == -1 && To_int(temp0, x)){
				m = new Datai(x);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag == 0 && To_float(temp0, y)){
				m = new Dataf(y);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag > 0 && temp0[0] == '\'' && temp0[temp0.length() - 1] == '\'' && temp0.length() - 2 <= flag)
			{
				m = new Datac(temp0.substr(1, temp0.length() - 2));
				temp.d = m;
				w.push_back(temp);
			}
			else throw QueryException("ERROR: Values in where clause is invalid!");

		}
		else if (qs.substr(pos, pos1 - pos) == ">"){
			temp.flag = g;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			temp0 = qs.substr(pos, pos1 - pos);

			if (temp0 == "="){
				temp.flag = geq;
				pos = pos1 + 1;
				pos1 = GOGOGO(pos);
				temp0 = qs.substr(pos, pos1 - pos);
			}

			if (flag == -1 && To_int(temp0, x)){
				m = new Datai(x);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag == 0 && To_float(temp0, y)){
				m = new Dataf(y);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag > 0 && temp0[0] == '\'' && temp0[temp0.length() - 1] == '\'' && temp0.length() - 2 <= flag)
			{
				m = new Datac(temp0.substr(1, temp0.length() - 2));
				temp.d = m;
				w.push_back(temp);
			}
			else throw QueryException("ERROR: Values in where clause is invalid!");

		}

		else if (qs.substr(pos, pos1 - pos) == "<"){
			temp.flag = l;
			pos = pos1 + 1;
			pos1 = GOGOGO(pos);
			temp0 = qs.substr(pos, pos1 - pos);

			if (temp0 == "="){
				temp.flag = leq;
				pos = pos1 + 1;
				pos1 = GOGOGO(pos);
				temp0 = qs.substr(pos, pos1 - pos);
			}
			else if (temp0 == ">"){
				temp.flag = neq;
				pos = pos1 + 1;
				pos1 = GOGOGO(pos);
				temp0 = qs.substr(pos, pos1 - pos);
			}




			if (flag == -1 && To_int(temp0, x)){
				m = new Datai(x);
				temp.d = m;
				w.push_back(temp);
			}

			else if (flag == 0 && To_float(temp0, y)){
				m = new Dataf(y);
				temp.d = m;
				w.push_back(temp);

			}

			else if (flag > 0 && temp0[0] == '\'' && temp0[temp0.length() - 1] == '\'' && temp0.length() - 2 <= flag)
			{
				m = new Datac(temp0.substr(1, temp0.length() - 2));
				temp.d = m;
				w.push_back(temp);
			}
			else throw QueryException("ERROR: Values in where clause is invalid!");
		}
		else throw QueryException("ERROR: invalid query format!");


		if (qs[pos1 + 1] == ';')
			break;

		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
		if (qs.substr(pos, pos1 - pos) != "and")
			throw QueryException("ERROR: invalid query format!");
		pos = pos1 + 1;
		pos1 = GOGOGO(pos);
	}
}

void InterManager::EXEC_FILE(){
	string fname = qs.substr(9, qs.length() - 11);
	ifstream in(fname);
	if (!in)
		throw QueryException("No file named " + fname);
	qs = "";
	string temp;
	while (in.peek() != EOF){

		try{
			in >> temp;
			qs = qs + temp + " ";
			if (temp[temp.length() - 1] == ';'){
				//   cout << qs << endl;
				EXEC();
				qs = "";
			}
		}
		catch (TableException te){
			cout << qs << endl;
			cout << te.what() << endl;
			qs = "";
		}
		catch (QueryException qe){
			cout << qs << endl;
			cout << qe.what() << endl;
			qs = "";
		}



	}
	in.close();
}