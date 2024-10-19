#include <filesystem>

#include "lyf.h"
#include "stopwatch.h"
#include "memCheck.h"

using namespace lyf;

void tree(std::filesystem::path path, int depth = 0) {
    using namespace std::filesystem;
    auto dir = directory_entry{ path };

    for (auto const& entry : directory_iterator{ dir }) {
        if (entry.is_regular_file()) {
            for (int i = 0; i < depth; i++) { cout << "\t"; }
            std::cout << entry.path() << '\n';
        }
        else if (entry.is_directory()) {
            for (int i = 0; i < depth; i++) { cout << "\t"; }
            std::cout << entry.path() << "\n";
            tree(entry.path(), depth + 1);
        }
    }
}

void cp(std::filesystem::path from, std::filesystem::path to) {
    std::ifstream in{ from };
    std::ofstream out{ to };
    out << in.rdbuf();
}


int main(int agrc, char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    TRACE_OFF();
    MEM_ON();

    assure(agrc == 2, "agrc != 2");
    tree(argv[1]);

    // assure(agrc == 3, "agrc != 3");
    // cp(argv[1], argv[2]);

    MEM_OFF();
    PrintTool::print_args("------------lyf--------------");
    return 0;
}
