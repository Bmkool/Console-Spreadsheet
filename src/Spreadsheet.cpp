#include "Spreadsheet.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <optional>

static std::vector<std::string> tokenize(const std::string& str, const std::string& del);
static std::optional<int> stringToInt(const std::string& str);
static bool processFormula(unsigned int row, unsigned int col, std::vector<std::vector<std::string>>& spreadsheet);

bool fileToSpreadsheet(const char filename[], std::vector<std::vector<std::string>>& spreadsheet)
{
    std::ifstream iFile(filename);
    if (!iFile.is_open()) {
        std::cerr << "Failed to open input file" << std::endl;
        return false;
    }

    std::string line;
    // Read each line and separate strings into columns separated by tabs.
    for (int i = 0; std::getline(iFile, line); i++) {
        auto tokens = tokenize(line,"\t");
        tokens.erase(std::remove(tokens.begin(), tokens.end(), "\t"), tokens.end()); // Remove tabs from row
        spreadsheet.push_back(tokens);

    }

    iFile.close();
    return true;
}

bool processSpreadsheet(std::vector<std::vector<std::string>>& spreadsheet)
{
    for (unsigned int row = 0; row < spreadsheet.size(); row++) {
        for (unsigned int column = 0; column < spreadsheet[row].size(); column++) {
            // Check for valid integer. If valid then no processing needed
            if (std::nullopt == stringToInt(spreadsheet[row][column])) {
                // Check if entry is a formula
                if (spreadsheet[row][column][0] != '=') {
                    std::cerr <<  "Input is not a valid integer or formula: " << spreadsheet[row][column] << std::endl;
                    return false;
                }

                if(!processFormula(row, column, spreadsheet)) {
                    return false;
                };
            }
        }
    }

    return true;
}

void printSpreadsheet(const std::vector<std::vector<std::string>>& spreadsheet) {
    for (auto& row : spreadsheet) {
        for (auto& entry : row) {
            std::cout << entry << "\t";
        }
        std::cout << "\n";
    }
}

// Returns integer if string as able to be converted or nullopt otherwise
static std::optional<int> stringToInt(const std::string& str) {
    int ret = 0;
    try {
        size_t pos = 0;
        ret = std::stoi(str, &pos);
        // If the whole string wasnt read then not an integer
        if (str.length() != pos) {
            return std::nullopt;
        }
    } catch (const std::exception &err) {
        // Unable to be converted to an integer
        return std::nullopt;
    }
    return ret;
}

static bool processFormula(unsigned int row, unsigned int col, std::vector<std::vector<std::string>>& spreadsheet) {
    auto tokens = tokenize(spreadsheet[row][col].substr(1,std::string::npos), "+-/*");
    // Multiplication and Division subcalculations
    auto it = tokens.begin();
    while (it != tokens.end()) {
        if (0 == it->compare("*")) {
            // Do multiplication of prev and next token
        } else if (0 == it->compare("/")) {
            // Do division of prev and next token
        } else {
            it++;
        }
    }

    // Addition and Subtraction subcalculations
    while (it != tokens.end()) {
        if (0 == it->compare("+")) {
            // Do multiplication of prev and next token
        } else if (0 == it->compare("-")) {
            // Do division of prev and next token
        } else {
            it++;
        }
    }
    return true;

}

// Return strings separated by any of the passed in delimiters. The delimiters are included as individual tokens
static std::vector<std::string> tokenize(const std::string& str, const std::string& del) {
    std::vector<std::string> tokens;
    int prev = 0;
    int pos = str.find_first_of(del);

    while (pos != -1) {
        tokens.push_back(str.substr(prev,pos));
        tokens.push_back(std::string{str[pos]});
        prev = pos + 1;
        pos = str.substr(prev, std::string::npos).find_first_of(del);
    }

    // Push back last token
    tokens.push_back(str.substr(prev, std::string::npos));

    return tokens;
}
