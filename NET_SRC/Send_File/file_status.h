#ifndef FILE_STATUS_H_
#define FILE_STATUS_H_

#include <filesystem>
#include <string>

enum class receive_enum {
    Failed,
    Success
};

class file_status {
private:
    std::string m_path;
    std::string m_filename;
    size_t m_size;

public:
    file_status()
        : m_path(), m_filename(), m_size(0) {}

    explicit file_status(const std::string& name, size_t size)
        : m_path(name), m_size(size) {}

    explicit file_status(std::filesystem::path path) {
        using namespace std::filesystem;
        if (!exists(path)) {
            throw std::runtime_error("文件不存在");
        } else if (!is_regular_file(path)) {
            throw std::runtime_error("该文件类型为目录");
        }
        m_path     = path.string();
        m_filename = path.filename().string();
        m_size     = file_size(path);
    }

    // 序列化
    std::string
    to_string() const {
        return m_path + " size: " + std::to_string(m_size);
    }

    // getter
    std::string
    path() const {
        return m_path;
    }

    size_t
    size() const {
        return m_size;
    }
};

#endif // !FILE_STATUS_H_
