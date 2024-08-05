#include <iostream>
#include "IO.hpp"
using namespace std;
IO::IO(const string &file_path)
{
    file_stream.open(file_path, ios::in | ios::out | ios::binary);
    if (!file_stream.is_open())
    {
        cout << "Unable to open the file :" << file_path << std::endl;
    }
}

fstream IO::getFileStream()
{
    // we are basically want to give  the ownership oof the move file_stream variable to the caller of this
    return move(file_stream);
}

IO::~IO()
{
    if (file_stream.is_open())
    {
        file_stream.close();
    }
}