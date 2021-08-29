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

/*
 * - Read input file
 * - Read through entire file, input into 2d vector
 * - Loop through each element and compute output string
 *  - If formula containing reference, go to reference and complete that one first
 *  - If circular reference then print error
 *  - Check for NAN
 * - Loop through vector to print output string
*/
