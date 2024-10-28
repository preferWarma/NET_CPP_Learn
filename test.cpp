#include "lyf.h"
#include "stopwatch.h"
#include <cstddef>
#include <filesystem>
#include <optional>

using namespace lyf;

std::optional<size_t>
getFileSize(const std::filesystem::path& path) {
    using namespace std::filesystem;
    // 检查文件是否存在或是否为普通文件而非目录
    if (!exists(path) || !is_regular_file(path)) {
        return std::nullopt;
    }
    return file_size(path);
}

int
main(int argc, const char** argv) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    std::filesystem::path path = "/Users/liuyifeng/Desktop/code/NET_CPP_Learn/test.cpp";
    auto size                  = getFileSize(path);
    if (size.has_value()) {
        m_print(size.value());
    }
    PrintTool::print_args("-------------lyf--------------");
    return 0;
}
