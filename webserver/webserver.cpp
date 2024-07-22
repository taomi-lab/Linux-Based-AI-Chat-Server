#include "webserver.h"

using namespace std;

// 添加信号捕捉
void addsig(int sig, void( handler )(int)){
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

webserver::webserver(int port, string user, string passWord, string databaseName, int LOGWrite, int TrigMode): 
port(port), m_user(user), m_passWord(passWord), m_databaseName(databaseName), m_log_write(LOGWrite), m_TrigMode(TrigMode)
{
    timeout = false;
    stop_server = false;
    users = new http_conn[ MAX_FD ];
    //定时器
    users_timer = new client_data[MAX_FD];
    // 对SIGPIE信号进行处理
    addsig( SIGPIPE, SIG_IGN );
    if (!Initthreadpool()){
        printf("Fail to create threadpool!");
        throw exception();
    }

    if (!InitSocket()){
        printf("Fail to create socket!");
        throw exception();
    }

    if (!Initepoll()){
        printf("Fail to create epoll!");
        throw exception();
    }
    if (!Inittimer()){
        printf("Fail to create timer!");
        throw exception();
    }
    if (!InitSQL()){
        printf("Fail to connect to SQL!");
        throw exception();
    }
    if(!InitLoger()){
        printf("Fail to create Loger!");
        throw exception();
    }
}
webserver::~webserver(){
    close( epollfd );
    close( listenfd );
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete [] users;
    delete [] users_timer;
    delete pool;
}


bool webserver::Initthreadpool(){
    try {
        pool = new threadpool<http_conn>;
    } catch( ... ) {
        return false;
    }
    return true;
}

bool webserver::InitSocket(){
    // 创建监听套接字
    // PF_INET TCP/IPv4协议族    SOCK_STREAM  流式协议(TCP)
    listenfd = socket( PF_INET, SOCK_STREAM, 0 );
    if (listenfd == -1) return false;

    int ret = 0;
    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons( port );

    // 端口复用
    int reuse = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );
    // 监听
    ret = listen( listenfd, 5 );
    return true;
}

bool webserver::Initepoll(){
    epollfd = epoll_create( 5 ); 
    if (epollfd == -1) return false;
    return true;
}

bool webserver::Inittimer(){
    timer_utils.init(TIMESLOT);
    timer_utils.addfd(epollfd, listenfd, false, m_ListenTrigMode);
    http_conn::m_epollfd = epollfd;

    // 创建管道
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    timer_utils.setnonblocking(m_pipefd[1]);
    timer_utils.addfd(epollfd, m_pipefd[0], false, false);

    timer_utils.addsig(SIGPIPE, SIG_IGN);
    timer_utils.addsig(SIGALRM, timer_utils.sig_handler, false);
    timer_utils.addsig(SIGTERM, timer_utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Timer_Utils::u_pipefd = m_pipefd;
    Timer_Utils::u_epollfd = epollfd;
    return true;

}

bool webserver::InitSQL(){
    m_SQL = new MySQL(m_user, m_passWord, m_databaseName);
    // m_SQL = nullptr;
    http_conn::init_SQL(m_SQL);
    return true;
}

bool webserver::InitLoger(){
    // 初始化日志
    m_close_log=0;
    if (m_log_write == 1)
        Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
    else
        Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    return true;
}

void webserver::trig_mode()
{
    //LT + LT
    if (0 == m_TrigMode)
    {
        m_ListenTrigMode = 0;
        m_ConntTrigMode = 0;
    }
    //LT + ET
    else if (1 == m_TrigMode)
    {
        m_ListenTrigMode = 0;
        m_ConntTrigMode  = 1;
    }
    //ET + LT
    else if (2 == m_TrigMode)
    {
        m_ListenTrigMode = 1;
        m_ConntTrigMode  = 0;
    }
    //ET + ET
    else if (3 == m_TrigMode)
    {
        m_ListenTrigMode = 1;
        m_ConntTrigMode  = 1;
    }
}


bool webserver::AddClient(){
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof( client_address );
    int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
    if (connfd < 0){
        printf("accept error!");
        return false;
    }
    users[connfd].init( connfd, client_address, m_ConntTrigMode);
    LOG_INFO("new connect, fd: %d", connfd);

    //初始化client_data数据
    //创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    timer_utils.m_timer_lst.add_timer(timer);
    return true;
}

bool webserver::dealwithsignal(bool &timeout, bool &stop_server)
{
    int sig;
    char signals[1024];
    int ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1 || ret == 0) return false;
    for (int i = 0; i < ret; ++i){
        switch (signals[i])
        {
        case SIGALRM:
            timeout = true;
            break;
        case SIGTERM:
            stop_server = true;
            break;
        }
    }
    return true;
}

void webserver::deal_timer(util_timer *timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if (timer)
    {
        timer_utils.m_timer_lst.del_timer(timer);
    }
    printf("close fd %d\n", users_timer[sockfd].sockfd);
}

//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
void webserver::adjust_timer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    timer_utils.m_timer_lst.adjust_timer(timer);
    printf("adjust timer once");
}



void webserver::start(){
    while(!stop_server){
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number<0) && errno != EINTR){
            LOG_ERROR( "epoll failure\n" );
            break;
        }

        for (int i=0; i<number;i++){
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd){
                // 有客户端连进来
                if (http_conn::m_user_count>=MAX_FD){
                    LOG_WARN("server busy!");
                    continue;
                }
                if (!AddClient()){
                    LOG_ERROR("Add client fail!");
                } 
            }
            else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                // 对方异常、断开或错误等事件
                // 服务器关闭连接，移除定时器
                LOG_INFO("client closed or some errors happened");
                users[sockfd].close_conn();
                util_timer *timer = users_timer[sockfd].timer;
                timer->cb_func(&users_timer[sockfd]);
                if(timer) timer_utils.m_timer_lst.del_timer(timer);
                LOG_INFO("closed fd %d\n", users_timer[sockfd].sockfd);

            } 
            else if ((sockfd == m_pipefd[0] ) && (events[i].events & EPOLLIN)){
                if (!dealwithsignal(timeout, stop_server))
                    LOG_ERROR("dealclientdata failure");
            }

            else if((events[i].events & EPOLLIN)) {
                // 一次性把全部数据读完
                util_timer *timer = users_timer[sockfd].timer;
                if(users[sockfd].read()) {
                    LOG_INFO("deal with the client (%s)\n", inet_ntoa(users[sockfd].get_address()->sin_addr));
                    pool->append(users + sockfd);
                    if(timer){
                        adjust_timer(timer);
                    }
                    else deal_timer(timer, sockfd);
                } else {
                    LOG_INFO("read client data failure!");
                    users[sockfd].close_conn();
                }

            }
            
            else if( events[i].events & EPOLLOUT ) {
                util_timer *timer = users_timer[sockfd].timer;
                // 一次性把全部数据写完
                if( users[sockfd].write() ) {
                    LOG_INFO("send data to the client (%s)\n", inet_ntoa(users[sockfd].get_address()->sin_addr));
                    if (timer) adjust_timer(timer);
                    else deal_timer(timer, sockfd);
                    
                }
                else{
                    users[sockfd].close_conn();
                }
            }   
        }
        if (timeout){
            timer_utils.timer_handler();
            timeout = false;
        }

    }
}



