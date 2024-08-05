#ifndef TASK_HPP
#define TASK_HPP
#include <string>
#include <fstream>
#include <sstream>
#include "../fileHandling/IO.hpp"

using namespace std;
enum class Action
{
    COMPRESS,
    DECOMPRESS
};
// We will have details of a  file which needs to be compressed and decompressed inside the Task Datastructure
struct Task
{
    string filePath;

    fstream f_stream;

    Action action;

    Task(std::fstream &&stream, Action act, std::string filePath)
        : filePath(filePath), f_stream(std::move(stream)), action(act) {}

    string toString()
    {
        // seriallize Task object into a string

        ostringstream oss;
        oss << filePath << "," << (action == Action::COMPRESS ? "COMPRESS" : "DECOMPRESS");

        return oss.str();
    }

    static Task fromString(const string &taskData)
    {
        istringstream iss(taskData);
        string filePath;
        string actionStr;

        if (getline(iss, filePath, ',') && getline(iss, actionStr, ','))
        {
            Action action = (actionStr == "COMPRESS") ? Action::COMPRESS : Action::DECOMPRESS;
            IO io(filePath);

            fstream f_stream = move(io.getFileStream());

            if (f_stream.is_open())
            {
                return Task(move(f_stream), action, filePath);
            }
            else
            {
                throw runtime_error("Failed to open file :" + filePath);
            }
        }
        else
        {
            throw runtime_error("Invalid task data format");
        }
    }
};

#endif