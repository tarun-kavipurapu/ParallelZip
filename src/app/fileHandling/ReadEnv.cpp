#include <iostream>
#include "IO.hpp"
#include <fstream>
#include <sstream>
using namespace std;

class ReadEnv
{
public:
    std::string getenv()
    {
        string env_path = ".env";
        IO io("env_path");
        fstream file_stream = io.getFileStream();
        stringstream buffer;
        buffer << file_stream.rdbuf();
        string content = buffer.str();

        return content;
    }
};