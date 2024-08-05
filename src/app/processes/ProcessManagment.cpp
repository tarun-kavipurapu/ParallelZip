#include "ProcessManagment.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "../compressDecompress/Compression.hpp"
#include <sys/wait.h>
using namespace std;

bool ProcessManagment::submitToQueue(unique_ptr<Task> task)
{

    // create a child process and submit it to it
    taskQueue.push(move(task));
    int pid = fork();
    if (pid < 0)
    {
        return false;
    }
    else if (pid > 0)
    {
        cout << "Entering The Parent Process" << endl;
    }
    else
    {

        cout << "Entering the child Process" << endl;
        processTask();
        cout << "Exiting the child Process" << endl;
    }

    return true;
}

void ProcessManagment::processTask()
{
    while (taskQueue.size() != 0)
    {
        unique_ptr<Task> task = move(taskQueue.front());
        taskQueue.pop();
        std::cout << "Executing task: " << task->toString() << std::endl;

        executeCompression(task->toString());
    }
}