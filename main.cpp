#include "processor.h"

#include <iostream>

int main(const int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    factory::Processor processor;
    processor.Solve(argc, argv);

    return 0;
}