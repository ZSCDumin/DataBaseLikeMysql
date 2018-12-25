
#include <iostream>
#include <string>
#include "Interpreter.h"
#include "API.h"
#include "BufferManager.h"
#include "RecordManager.h"
using namespace std;

BufferManager bf;
extern int Number;
int main() {
    int re = 1;
    InterManager itp;
	cout << endl
		<< "         _       __  ______  __     ______  ____    __  ___  ______              " << endl
		<< "        | |     / / / ____/ / /    / ____/ / __ \\  /  |/  / / ____/             " << endl
		<< "        | | /| / / / __/   / /    / /     / / / / / /|_/ / / __/                 " << endl
		<< "        | |/ |/ / / /___  / /___ / /___  / /_/ / / /  / / / /___                 " << endl
		<< "        |__/|__/ /_____/ /_____/ \\____/  \\____/ /_/  /_/ /_____/               " << endl << endl;

	cout <<"  ///////////////////////////////////////////////////////////////////////////////" << endl
		<< "  ///**************************** Welcom to MySQL! **************************////" << endl
		<< "  ///**************************** Copyright by HNU **************************////" << endl
		<< "  ///**************************** 作者：杜敏 ********************************////" << endl
		<< "  ///**************************** 邮箱: 2712220318@qq.com *******************////" << endl
		<< "  ///********* 项目地址: https://github.com/ZSCDumin/DataBaseLikeMysql ******////" << endl
		<< "  ///////////////////////////////////////////////////////////////////////////////" << endl<<endl;
    
    while(re){
        try{
            cout << "ZSCDuminSQL>";
            itp.GetQs();//获取查询语句
            re = itp.EXEC();//执行查询语句
        }
        catch(TableException te){
            cout << te.what() << endl;
        }
        catch(QueryException qe){
            cout << qe.what() << endl;
        }
    }
    return 0;
}

