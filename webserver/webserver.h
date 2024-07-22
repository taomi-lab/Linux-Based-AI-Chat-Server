#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include "../locker/locker.h"
#include "../pool/threadpool.h"
#include "../http/http_conn.h"
#include "../timer/timer.h"
#include "../MySQL/MySQL.h"
#include "../log/log.h"

#define MAX_FD 65536            // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量
const int TIMESLOT = 10;         //最小超时单位

class webserver
{
private:
    bool Initthreadpool();
    bool InitSocket();
    bool Initepoll();
    bool Inittimer();
    bool InitSQL();
    bool InitLoger();
    void trig_mode();
    bool AddClient();
    void deal_timer(util_timer *timer, int sockfd);
    void adjust_timer(util_timer *timer);
    bool dealwithsignal(bool &timeout, bool &stop_server);

    http_conn* users = nullptr;
    int m_log_write;
    int m_close_log;

    // 线程池
    threadpool< http_conn >* pool = nullptr;

    //触发模式
    int m_TrigMode;
    int m_ListenTrigMode;
    int m_ConntTrigMode;


    //定时器
    client_data *users_timer=nullptr;
    Timer_Utils timer_utils;

    //数据库
    MySQL *m_SQL;
    string m_user;
    string m_passWord;
    string m_databaseName;
    

private:
    int port;
    int listenfd;
    int epollfd;
    bool stop_server;
    bool timeout;
    int m_pipefd[2];
    epoll_event events[ MAX_EVENT_NUMBER ];

    
    

public:
    webserver(int port, string user, string passWord, string databaseName, int LOGWrite, int TrigMode);
    ~webserver();
    void start();
};
#endif