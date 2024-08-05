#ifndef PROCESS_MANAGMENT_HPP
#define PROCESS_MANAGMEENT_HPP
#include "Task.hpp"
#include <memory>
#include <queue> // For std::queue
#include <atomic>
#include <semaphore.h>
using namespace std;

using namespace std;
class ProcessManagment
{

public:
    bool submitToQueue(unique_ptr<Task> task);
    void processTask();

private:
    queue<unique_ptr<Task>> taskQueue;

    struct SharedMemory
    {
        atomic<int> size;
        char tasks[1000][256];
        int front;
        int rear;

        void printSharedMemory()
        {
            std::cout << size << std::endl;
            std::cout << front << std::endl;
            std::cout << rear << std::endl;
        }
    };
    SharedMemory *sharedMem;
    int shFd; // this is a file descriptor
    const char *SHM_NAME = "/my_queue";
};

#endif