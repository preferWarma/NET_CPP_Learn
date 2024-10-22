#ifndef FILE_STATUS_H_
#define FILE_STATUS_H_

#include <cstring>
#include <filesystem>
#include <string>

class file_status {
private:
    char m_path[512];
    char m_filename[256];
    size_t m_size;

public:
    file_status()
        : m_size(0) {
        memset(m_path, 0, sizeof(m_path));
        memset(m_filename, 0, sizeof(m_filename));
    }

    explicit file_status(const std::string& filePath, const std::string& fileName, size_t size)
        : m_size(size) {
        memcpy(m_path, filePath.data(), filePath.size());
        memcpy(m_filename, fileName.data(), fileName.size());
    }

    explicit file_status(std::filesystem::path path) {
        using namespace std::filesystem;
        if (!exists(path)) {
            throw std::runtime_error("文件不存在");
        } else if (!is_regular_file(path)) {
            throw std::runtime_error("该文件类型为目录");
        }
        memcpy(m_path, path.string().data(), path.string().size());
        memcpy(m_filename, path.filename().string().data(), path.filename().string().size());
        m_size = file_size(path);
    }

    // 序列化
    std::string
    to_string() const {
        return "[filename: (" + std::string(m_path) + ") size: (" + std::to_string(m_size) + ") ]";
    }

    // getter
    std::string
    path() const {
        return m_path;
    }

    std::string
    filename() const {
        return m_filename;
    }

    size_t
    size() const {
        return m_size;
    }
};

#endif // !FILE_STATUS_H_
