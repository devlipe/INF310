#include "my_sync.h"
#include <map>
#include <pthread.h>

pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;
std::map<pthread_t, int> thread_map;

void block()
{
    bool sair = false;
    long me = pthread_self();

    do
    {
        pthread_mutex_lock(&mux);
        if (thread_map[me] > 0)
        {
            thread_map[me]--;
            sair = true;
        }
        pthread_mutex_unlock(&mux);
    } while (!sair);
}

void wakeup(pthread_t thread)
{
    pthread_mutex_lock(&mux);

    thread_map[thread]++;

    pthread_mutex_unlock(&mux);
}
