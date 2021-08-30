#include <iostream>
#include <vector>
#include <string>

#include "Spreadsheet.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Missing filename" << std::endl;
        return 1;
    }

    std::vector<std::vector<std::string>> spreadsheet;
    if (!fileToSpreadsheet(argv[1], spreadsheet)) {
        return 1;
    }

    if (!processSpreadsheet(spreadsheet)) {
        return 1;
    }

    printSpreadsheet(spreadsheet);

    return 0;
}
