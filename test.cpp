#include "lyf.h"
#include "stopwatch.h"

using namespace lyf;

int main(int argc, const char** argv) {
    auto_stopwatch sw(stopwatch::TimeType::ms);

    PrintTool::print_args("-------------lyf--------------");
    return 0;
}