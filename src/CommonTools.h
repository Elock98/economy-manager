#pragma once
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <cassert>

#include <chrono>
#include <iomanip>
#include <ctime>

namespace FileSystem {
    bool FileExists(const char* filepath);
    bool IsDirectory(const char* path);
    bool ListFiles(const char* path, std::vector<std::string>& fileslist);

    bool MakeDir(const char* path);
    bool ReadFile(const std::string filepath, std::vector<std::string>& contents);

    template<size_t N>
    bool ReadStaticRowCSV(const std::string filepath, std::vector<std::array<std::string, N>>& contents) {
        std::vector<std::string> fileLines;
        if (!ReadFile(filepath, fileLines))
            return false;

        for (auto line : fileLines) {
            std::array<std::string, N> row = {};
            std::stringstream ss(line);
            std::string cell;
            size_t colIndex = 0;

            while (std::getline(ss, cell, ',')) {
                row[colIndex++] = cell;
            }

            if (colIndex != N) {
                assert(false);
                return false; // Invalid file data
            }

            contents.push_back(row);
        }
        return true;
    }

    bool WriteFile(const std::string& filename, const std::string& filecontent);
}

namespace Time {
    std::tm GetCurrentTime();
    std::string TimeToStr(std::tm time, const char* format);
}

namespace String {
    std::string JoinStrings(const std::vector<std::string> strings, const std::string delimiter);

    template<size_t N>
    std::string JoinStrings(const std::array<std::string, N> strings, const std::string delimiter) {
        std::ostringstream result;

        for (size_t ix = 0; ix < N; ix++) {
            result << strings[ix];
            if (ix < N)
                result << delimiter;
        }

        return result.str();
    }
}