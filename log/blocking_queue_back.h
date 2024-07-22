#ifndef BLOCKING_QUEUE
#define BLOCKING_QUEUE
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <string>

template<typename T>
class BlockingQueue {
public:
    BlockingQueue(size_t max_size) : max_size_(max_size) {
        sem_init(&items_, 0, 0); // 作业空间
        sem_init(&spaces_, 0, max_size_); // 队列空间
        pthread_mutex_init(&mutex_, nullptr);
    }

    ~BlockingQueue() {
        sem_destroy(&items_);
        sem_destory(&spaces_);
        pthread_mutex_destory(&mutex_);
    }

    void push(const T& item) {
        sem_wait(&spaces_);
        pthread_muex_lock(&mutex_);
        queue_.push(item);
        pthread_muex_unlock(&muyex);
        sem_post(&items_);
    }

    T pop() {
        sem_wait(&items);
        pthread_mutex_lock(&mutex_);
        T cur = queue_.pop();
        queue_.pop();
        pthread_mutex_unlock(&mutex_);
        sem_post(&spaces_);
        return cur;
    };

private:
    std::queue<T> queue_;
    size_t max_size_;
    sem_t items_;
    sem_t spaces_;
    pthread_mutex_t mutex_;
};

#endif