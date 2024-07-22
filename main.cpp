#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./webserver/webserver.h"
using namespace std;

int main( int argc, char* argv[] ) {
    int port=8080;
    if( argc > 1 ) {
        port = atoi(argv[1]);
    }
    //需要修改的数据库信息,登录名, 密码, 数据库名称
    string user = "root";
    string passwd = "12345678";
    string databasename = "webserver";

    // 触发组合模式,默认listenfd LT + connfd LT
    int TRIGMode = 1;

    //日志写入方式，默认同步 
    int LOGWrite = 0;

    webserver server(port, user, passwd, databasename, LOGWrite, TRIGMode);

    server.start();

}