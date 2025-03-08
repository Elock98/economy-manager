#include "CommonTools.h"

namespace FileSystem {

    bool FileExists(const char* filepath) {
        struct stat buffer;
        return (stat(filepath, &buffer) == 0);
    }

    bool IsDirectory(const char* path) {
        struct stat buffer;
        return (stat(path, &buffer) == 0 && S_ISDIR(buffer.st_mode));
    }

    bool ListFiles(const char* path, std::vector<std::string>& filelist) {
        DIR* dir = opendir(path);
        if (!dir)
            return false;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (!IsDirectory(entry->d_name))
                filelist.push_back(entry->d_name);
        }
        return true;
    }

    bool MakeDir(const char* path) {
        return mkdir(path, 0777) == 0;
    }

    bool ReadFile(const std::string filepath, std::vector<std::string>& contents) {
        std::ifstream file(filepath);
        if (!file)
            return false; // Unable to open file

        std::string line;
        while (std::getline(file, line)) {
            contents.push_back(line);
        }
        file.close();
        return true;
    }

    bool WriteFile(const std::string& filename, const std::string& filecontent) {
        std::ofstream outFile(filename);

        if (!outFile)
            return false;

        outFile << filecontent;

        outFile.close();
        return true;
    }

}

namespace Time {

    std::tm GetCurrentTime() {
        std::time_t now = std::time(nullptr);

        std::tm localTime;
        localtime_r(&now, &localTime);

        return localTime;
    }

    std::string TimeToStr(std::tm time, const char* format) {
        std::ostringstream ss;
        ss << std::put_time(&time, format);
        return ss.str();
    }
}

namespace String {
    std::string JoinStrings(const std::vector<std::string> strings, const std::string delimiter) {
        std::ostringstream result;

        for (size_t ix = 0; ix < strings.size(); ix++) {
            result << strings[ix];
            if (ix < strings.size())
                result << delimiter;
        }

        return result.str();
    }
}