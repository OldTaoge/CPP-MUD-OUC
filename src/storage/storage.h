#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * @brief 存储模块，负责游戏数据的保存和加载
 */
class Storage {
public:
    /**
     * @brief 获取存储模块单例
     */
    static Storage& getInstance();
    
    /**
     * @brief 保存字符串数据到文件
     * @param filename 文件名
     * @param data 要保存的数据
     * @return 是否保存成功
     */
    bool saveString(const std::string& filename, const std::string& data);
    
    /**
     * @brief 从文件加载字符串数据
     * @param filename 文件名
     * @return 加载的数据，如果失败返回空字符串
     */
    std::string loadString(const std::string& filename);
    
    /**
     * @brief 保存二进制数据到文件
     * @param filename 文件名
     * @param data 二进制数据指针
     * @param size 数据大小
     * @return 是否保存成功
     */
    bool saveBinary(const std::string& filename, const char* data, size_t size);
    
    /**
     * @brief 从文件加载二进制数据
     * @param filename 文件名
     * @param[out] data 输出的二进制数据（需要调用者释放）
     * @param[out] size 数据大小
     * @return 是否加载成功
     */
    bool loadBinary(const std::string& filename, char*& data, size_t& size);
    
    /**
     * @brief 检查文件是否存在
     * @param filename 文件名
     * @return 文件是否存在
     */
    bool fileExists(const std::string& filename);
    
    /**
     * @brief 删除文件
     * @param filename 文件名
     * @return 是否删除成功
     */
    bool deleteFile(const std::string& filename);
    
    /**
     * @brief 获取保存目录路径
     * @return 保存目录路径
     */
    std::string getSaveDirectory() const;
    
    /**
     * @brief 设置保存目录路径
     * @param directory 目录路径
     */
    void setSaveDirectory(const std::string& directory);
    
private:
    Storage();
    ~Storage();
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;
    
    std::string saveDirectory_;
};