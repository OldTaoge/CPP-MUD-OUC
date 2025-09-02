#include "storage.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

Storage& Storage::getInstance() {
    static Storage instance;
    return instance;
}

Storage::Storage() {
    // 设置默认保存目录
#ifdef _WIN32
    CHAR documentsPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, documentsPath))) {
        saveDirectory_ = std::string(documentsPath) + "\\CPP-MUD-OUC\\Saves\\";
    } else {
        saveDirectory_ = ".\\Saves\\";
    }
#else
    const char* homeDir = getenv("HOME");
    if (homeDir == nullptr) {
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) {
            homeDir = pw->pw_dir;
        }
    }
    
    if (homeDir != nullptr) {
        saveDirectory_ = std::string(homeDir) + "/.cpp-mud-ouc/saves/";
    } else {
        saveDirectory_ = "./saves/";
    }
#endif
    
    // 创建保存目录
    std::filesystem::create_directories(saveDirectory_);
}

Storage::~Storage() {}

bool Storage::saveString(const std::string& filename, const std::string& data) {
    std::string fullPath = saveDirectory_ + filename;
    
    try {
        std::ofstream file(fullPath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        
        file << data;
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

std::string Storage::loadString(const std::string& filename) {
    std::string fullPath = saveDirectory_ + filename;
    
    if (!fileExists(filename)) {
        return "";
    }
    
    try {
        std::ifstream file(fullPath, std::ios::in);
        if (!file.is_open()) {
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return buffer.str();
    } catch (...) {
        return "";
    }
}

bool Storage::saveBinary(const std::string& filename, const char* data, size_t size) {
    std::string fullPath = saveDirectory_ + filename;
    
    try {
        std::ofstream file(fullPath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        
        file.write(data, size);
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool Storage::loadBinary(const std::string& filename, char*& data, size_t& size) {
    std::string fullPath = saveDirectory_ + filename;
    
    if (!fileExists(filename)) {
        return false;
    }
    
    try {
        std::ifstream file(fullPath, std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }
        
        size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        data = new char[size];
        file.read(data, size);
        file.close();
        
        return true;
    } catch (...) {
        return false;
    }
}

bool Storage::fileExists(const std::string& filename) {
    std::string fullPath = saveDirectory_ + filename;
    return std::filesystem::exists(fullPath);
}

bool Storage::deleteFile(const std::string& filename) {
    std::string fullPath = saveDirectory_ + filename;
    
    try {
        return std::filesystem::remove(fullPath);
    } catch (...) {
        return false;
    }
}

std::string Storage::getSaveDirectory() const {
    return saveDirectory_;
}

void Storage::setSaveDirectory(const std::string& directory) {
    saveDirectory_ = directory;
    // 确保目录存在
    std::filesystem::create_directories(saveDirectory_);
}