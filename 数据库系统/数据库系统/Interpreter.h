
#ifndef Interpreter_h
#define Interpreter_h
#include "base.h"


class InterManager{
private:
    std::string qs;//query string
public:
    int EXEC();
    void EXEC_SELECT();
    void EXEC_DROP();
    void EXEC_CREATE();
    void EXEC_CREATE_TABLE();
    void EXEC_CREATE_INDEX();
    void EXEC_INSERT();
    void EXEC_DELETE();
    void EXEC_SHOW();
    void EXEC_EXIT();
    void EXEC_FILE();
	void EXEC_UPDATE();
    inline int GOGOGO(int pos);
    
    void interwhere(int& pos1, std::vector<int> &attrwhere, std::vector<where> &w, Attribute A,
               Table* t);
    
    void GetQs();
    //Get the inputing string
    
    void Normolize();
    //Normolize the inputting string
};


class QueryException:std::exception{
public:
    QueryException(std::string s):text(s){}
    std::string what(){
        return text;
    };
private:
    std::string text;
};

bool To_int(std::string s, int& a);

bool To_float(std::string s, float& a);

#endif /* Interpreter_h */

