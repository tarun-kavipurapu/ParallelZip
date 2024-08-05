#include <iostream>
#include <filesystem>
#include "./src/app/processes/ProcessManagment.hpp"
#include "./src/app/processes/Task.hpp"
#include <ctime>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    std::string directory;
    std::string action;

    std::cout << "Enter the directory path: ";
    std::getline(std::cin, directory);

    std::cout << "Enter the action (compress/decompress): ";
    std::getline(std::cin, action);

    std::cout << "Directory: " << directory << std::endl;
    std::cout << "Action: " << action << std::endl;

    try
    {
        if (fs::exists(directory) && fs::is_directory(directory))
        {
            ProcessManagment processManagement;
            int fileCount = 0;

            for (const auto &entry : fs::recursive_directory_iterator(directory))
            {
                if (entry.is_regular_file())
                {
                    std::string filePath = entry.path().string();
                    std::cout << "Processing file: " << filePath << std::endl;

                    IO io(filePath);
                    std::fstream f_stream = std::move(io.getFileStream());

                    if (f_stream.is_open())
                    {
                        Action taskAction = (action == "compress") ? Action::COMPRESS : Action::DECOMPRESS;
                        auto task = std::make_unique<Task>(std::move(f_stream), taskAction, filePath);

                        std::time_t t = std::time(nullptr);
                        std::tm *now = std::localtime(&t);
                        std::cout << "Submitting task at: " << std::put_time(now, "%Y-%m-%d %H:%M:%S") << std::endl;

                        if (processManagement.submitToQueue(std::move(task)))
                        {
                            fileCount++;
                            std::cout << "Task submitted successfully." << std::endl;
                        }
                        else
                        {
                            std::cout << "Failed to submit task for file: " << filePath << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "Unable to open file: " << filePath << std::endl;
                    }
                }
            }

            std::cout << "Total files processed: " << fileCount << std::endl;

            if (fileCount > 0)
            {
                std::cout << "Processing tasks..." << std::endl;
                processManagement.processTask();
                std::cout << "All tasks processed." << std::endl;
            }
            else
            {
                std::cout << "No files found to process in the specified directory." << std::endl;
            }
        }
        else
        {
            std::cout << "Invalid directory path or not a directory: " << directory << std::endl;
        }
    }
    catch (const fs::filesystem_error &ex)
    {
        std::cout << "Filesystem error: " << ex.what() << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cout << "An error occurred: " << ex.what() << std::endl;
    }

    std::cout << "Program finished. Press Enter to exit." << std::endl;
    std::cin.get();

    return 0;
}