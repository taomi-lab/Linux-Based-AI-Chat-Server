#ifndef LST_TIMER
#define LST_TIMER

#include <stdio.h>
#include <time.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 64
class util_timer;   

// 用户数据结构
struct client_data
{
    sockaddr_in address;    
    int sockfd;            
    util_timer* timer;       
};

// 定时器类
class util_timer {
public:
    util_timer() : prev(NULL), next(NULL){}

public:
   time_t expire;   // 任务超时时间，这里使用绝对时间
   void (*cb_func)( client_data* ); // 任务回调函数，回调函数处理的客户数据，由定时器的执行者传递给回调函数
   client_data* user_data; 
   util_timer* prev;    // 指向前一个定时器
   util_timer* next;    // 指向后一个定时器
};

// 定时器链表，它是一个升序、双向链表，且带有头节点和尾节点。
class sort_timer_lst {
public:
    sort_timer_lst() : head( NULL ), tail( NULL ) {}
    ~sort_timer_lst();
    // 将目标定时器timer添加到链表中
    void add_timer( util_timer* timer );
    /* 当某个定时任务发生变化时，调整对应的定时器在链表中的位置。这个函数只考虑被调整的定时器的
    超时时间延长的情况，即该定时器需要往链表的尾部移动。*/
    void adjust_timer(util_timer* timer);
    // 将目标定时器 timer 从链表中删除
    void del_timer( util_timer* timer );
    /* SIGALARM 信号每次被触发就在其信号处理函数中执行一次 tick() 函数，以处理链表上到期任务。*/
    void tick();

private:
    /* 一个重载的辅助函数，它被公有的 add_timer 函数和 adjust_timer 函数调用
    该函数表示将目标定时器 timer 添加到节点 lst_head 之后的部分链表中 */
    void add_timer(util_timer* timer, util_timer* lst_head);

private:
    util_timer* head;   // 头结点
    util_timer* tail;   // 尾结点
};


class Timer_Utils
{
public:
    Timer_Utils() {}
    ~Timer_Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, bool et);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif
