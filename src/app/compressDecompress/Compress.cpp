#include "Compression.hpp"
#include "../processes/Task.hpp"
#include <zlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <filesystem>
#include <iomanip>
#include <ctime>

#define CHUNK 16384

namespace fs = std::filesystem;

int compressData(const std::string &inputPath, const std::string &outputPath)
{
    FILE *source = fopen(inputPath.c_str(), "rb");
    FILE *dest = fopen(outputPath.c_str(), "wb");
    if (!source || !dest)
    {
        fprintf(stderr, "Could not open files.\n");
        if (source)
            fclose(source);
        if (dest)
            fclose(dest);
        return Z_ERRNO;
    }

    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // Allocate deflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    // Change compression level to maximum
    ret = deflateInit(&strm, Z_BEST_COMPRESSION);
    if (ret != Z_OK)
    {
        fclose(source);
        fclose(dest);
        return ret;
    }

    // Compress until end of file
    do
    {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source))
        {
            deflateEnd(&strm);
            fclose(source);
            fclose(dest);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        // Run deflate() on input until output buffer not full, finish
        // compression if all of source has been read in
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);   // No bad return value
            assert(ret != Z_STREAM_ERROR); // State not clobbered
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest))
            {
                deflateEnd(&strm);
                fclose(source);
                fclose(dest);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); // All input will be used

        // Done when last data in file processed
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END); // Stream will be complete

    // Clean up and return
    deflateEnd(&strm);
    fclose(source);
    fclose(dest);
    return Z_OK;
}

// ... (decompressData function remains unchanged)
int decompressData(const std::string &inputPath, const std::string &outputPath)
{
    FILE *source = fopen(inputPath.c_str(), "rb");
    FILE *dest = fopen(outputPath.c_str(), "wb");
    if (!source || !dest)
    {
        fprintf(stderr, "Could not open files.\n");
        if (source)
            fclose(source);
        if (dest)
            fclose(dest);
        return Z_ERRNO;
    }

    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // Allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
    {
        fclose(source);
        fclose(dest);
        return ret;
    }

    // Decompress until deflate stream ends or end of file
    do
    {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source))
        {
            inflateEnd(&strm);
            fclose(source);
            fclose(dest);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        // Run inflate() on input until output buffer not full
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR); // State not clobbered
            switch (ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR; // And fall through
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                fclose(source);
                fclose(dest);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest))
            {
                inflateEnd(&strm);
                fclose(source);
                fclose(dest);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        // Done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // Clean up and return
    inflateEnd(&strm);
    fclose(source);
    fclose(dest);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int executeCompression(const std::string &taskData)
{
    Task task = Task::fromString(taskData);
    fs::path inputPath(task.filePath);
    fs::path outputPath;

    if (task.action == Action::COMPRESS)
    {
        // Store the original extension in the compressed file name
        std::string originalExtension = inputPath.extension().string();
        if (originalExtension.empty())
        {
            originalExtension = ".none"; // Use .none if there was no extension
        }
        outputPath = inputPath.parent_path() / (inputPath.stem().string() + originalExtension + ".zlib");
    }
    else // Decompression
    {
        // Remove .zlib and restore the original extension
        if (inputPath.extension() == ".zlib")
        {
            std::string nameWithoutZlib = inputPath.stem().string();
            size_t lastDotPos = nameWithoutZlib.find_last_of('.');
            if (lastDotPos != std::string::npos)
            {
                std::string restoredExtension = nameWithoutZlib.substr(lastDotPos);
                std::string nameWithoutExtension = nameWithoutZlib.substr(0, lastDotPos);
                if (restoredExtension == ".none")
                {
                    outputPath = inputPath.parent_path() / nameWithoutExtension;
                }
                else
                {
                    outputPath = inputPath.parent_path() / (nameWithoutExtension + restoredExtension);
                }
            }
            else
            {
                // If no extension found, just remove .zlib
                outputPath = inputPath.parent_path() / nameWithoutZlib;
            }
        }
        else
        {
            // If the input file doesn't end with .zlib, just add _decompressed
            outputPath = inputPath.parent_path() / (inputPath.stem().string() + "_decompressed" + inputPath.extension().string());
        }
    }

    int result;
    if (task.action == Action::COMPRESS)
    {
        result = compressData(task.filePath, outputPath.string());
        if (result == Z_OK)
        {
            std::cout << "Compression successful. Compressed file saved as: " << outputPath << std::endl;

            // Print file sizes
            uintmax_t inputSize = fs::file_size(inputPath);
            uintmax_t outputSize = fs::file_size(outputPath);
            std::cout << "Original size: " << inputSize << " bytes" << std::endl;
            std::cout << "Compressed size: " << outputSize << " bytes" << std::endl;
            double compressionRatio = (1.0 - static_cast<double>(outputSize) / inputSize) * 100.0;
            std::cout << "Compression ratio: " << compressionRatio << "%" << std::endl;
        }
        else
        {
            std::cerr << "Compression failed with error code: " << result << std::endl;
        }
    }
    else
    {
        result = decompressData(task.filePath, outputPath.string());
        if (result == Z_OK)
        {
            std::cout << "Decompression successful. Decompressed file saved as: " << outputPath << std::endl;
        }
        else
        {
            std::cerr << "Decompression failed with error code: " << result << std::endl;
        }
    }

    time_t t = time(nullptr);

    tm *now = localtime(&t);
    cout << "Exiting the encryption Decryption at:" << std::put_time(now, "%Y-%m-%d %H:%M:%S") << std::endl;
    return result;
}