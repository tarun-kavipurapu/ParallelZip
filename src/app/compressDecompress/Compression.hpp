#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

#include <string>

int compressData(const std::string &inputPath, const std::string &outputPath);
int decompressData(const std::string &inputPath, const std::string &outputPath);
int executeCompression(const std::string &taskData);

#endif // COMPRESSION_HPP
