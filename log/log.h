#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <string>
#include "blocking_queue.h"


class Log {
public:
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *args)
    {
        Log::get_instance()->async_write_log();
    }

    bool init(const char * file_name, int close_log , int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);
private:
    Log();
    virtual ~Log();
     void *async_write_log()
    {
        string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log))
        {   
            m_mutex.lock();
            fputs(single_log.c_str(), m_file);
            m_mutex.unlock();
        }
    }

private:
    char dir_name[128]; // 路径名
    char log_name[128]; // Log 文件名
    long long m_count; // 日志行数记录
    int m_split_lines; // 日志最大行数
    int m_log_buf_size; // 日志缓冲区大小
    int m_today;        // 记录当前是哪天
    locker m_mutex;    //互斥锁
    char * m_buf;
    FILE * m_file; // 文件输入
    block_queue<std::string> *m_log_queue; // 阻塞队列
    bool m_is_async;  // 同步/异步模式设置
    bool m_close_log;  // 是否关闭log
};


#define LOG_DEBUG(format, ...)  {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...)  {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...)  {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...)  {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif