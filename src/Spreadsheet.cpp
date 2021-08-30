#include "Spreadsheet.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <optional>
#include <set>
#include <utility>

static std::vector<std::string> tokenize(const std::string& str, const std::string& del);
static std::optional<int> stringToInt(const std::string& str);
static bool processFormula(unsigned int row, unsigned int col, std::vector<std::vector<std::string>>& spreadsheet);
static std::pair<int,int> refToRowCol(const std::string& ref);

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
                // Check if no work is needed
                if (spreadsheet[row][column].empty() ||
                   (0 == spreadsheet[row][column].compare("#ERROR")) ||
                   (0 == spreadsheet[row][column].compare("#NAN"))) {
                       continue;
                }

                // Only valid entry left is a formula
                if (spreadsheet[row][column][0] != '=') {
                    std::cerr <<  "Input is not a valid integer or formula: '" << spreadsheet[row][column] << "'" << std::endl;
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

    // Set of coordinate pairs that are currently being processed
    static std::set<std::pair<int,int>> callStack;

    // Check if there is a circular reference
    auto it = callStack.find(std::pair<int,int>(row,col));
    if (callStack.end() != it) {
        spreadsheet[row][col] = "#ERROR";
        callStack.erase({row, col});
        return true;
    }

    // Add current entry to processing stack
    callStack.insert({row, col});

    // Should always be an odd number of tokens [int(,op,int)+)
    if ((tokens.size() & 1) == 0) {
        std::cerr << "Invalid Formula: '" << spreadsheet[row][col] << "'" << std::endl; 
        callStack.erase({row, col});;
        return false;
    }

    // Recursive process to determine the final value of the formula.  
    auto formulaToInt = [&] (std::optional<int>& entry, int pos) -> std::optional<bool> { 
        auto coord = refToRowCol(tokens[pos]);
        // Make sure references exist and are valid
        if (coord.first < 0 || coord.second < 0) {
            if (0 == spreadsheet[row][col].compare("#NAN")) {
                spreadsheet[row][col] = "#NAN";
                callStack.erase({row, col});
                return true;  
            }
            std::cerr << "Invalid Formula: '" << spreadsheet[row][col] << "'" << std::endl; 
            callStack.erase({row, col});;
            return false;
        } else if (coord.first >= static_cast<int>(spreadsheet.size()) || 
                    coord.second >= static_cast<int>(spreadsheet[coord.first].size()) || 
                    spreadsheet[coord.first][coord.second].empty())
        {
            spreadsheet[row][col] = "#NAN";
            callStack.erase({row, col});
            return true;
        }
        // Evaluate reference
        auto tmp = stringToInt(spreadsheet[coord.first][coord.second]);
        if (tmp.has_value()) {
            entry.emplace(tmp.value());
        } else {
            // Recursively evaluate referenced cell
            if (!processFormula(coord.first, coord.second, spreadsheet)) {
                callStack.erase({row, col});
                return false;
            }
            // Re-evaluate entry after reference processing. Propogate referenced cell ERRORs and NANs
            entry = stringToInt(spreadsheet[coord.first][coord.second]);
            if (!entry.has_value()) {
                spreadsheet[row][col] = spreadsheet[coord.first][coord.second];
                callStack.erase({row, col});
                return true;
            }
        }
        return std::nullopt;
    };

    int result = 0;
    std::optional<int> last = stringToInt(tokens[0]);
    if (!last.has_value()) {
        auto rc = formulaToInt(last, 0);
        if (rc.has_value()) {
            return rc.value();
        }
    }

    // iterate over each operation
    for (unsigned int i = 1; i + 1 < tokens.size(); i+=2) {
        // Make sure next token is valid and evaluate the formula if necessary
        auto next = stringToInt(tokens[i+1]);
        if (!next.has_value()) {
            auto rc = formulaToInt(next, i+1);
            if (rc.has_value()) {
                return rc.value();
            }
        }

        // Perform multiplication and division operations for the current "last" value as those take 
        // precedence. For addition and subtraction add the last value immediately to the result and update
        // the "last" value with the next one
        if (0 == tokens[i].compare("*")) {
            last.emplace(last.value() * next.value());
        } else if (0 == tokens[i].compare("/")) {
            last.emplace(last.value() / next.value());
        } else if (0 == tokens[i].compare("+")) {
            result += last.value();
            last.emplace(next.value());
        } else if (0 == tokens[i].compare("-")) {
            result += last.value();
            last.emplace(-next.value());
        } else {
            std::cerr << "Invalid Formula: '" << spreadsheet[row][col] << "'" << std::endl; 
            callStack.erase({row, col});;
            return false;       
        }
    }
    // Add the last remaining token
    result += last.value();

    spreadsheet[row][col] = std::to_string(result);
    callStack.erase({row, col});
    return true;

}

// Return strings separated by any of the passed in delimiters. The delimiters are included as individual tokens
static std::vector<std::string> tokenize(const std::string& str, const std::string& del) {
    std::vector<std::string> tokens;
    size_t prev = 0;
    size_t pos = str.find_first_of(del);

    while (pos - prev != std::string::npos) {
        tokens.push_back(str.substr(prev,pos-prev));
        tokens.push_back(std::string{str[pos]});
        prev = pos + 1;
        pos = prev + str.substr(prev, std::string::npos).find_first_of(del);
    }


    // Push back last token if not delimiter
    if (prev < str.length()) {
        tokens.push_back(str.substr(prev, std::string::npos));
    }

    return tokens;
}

// Returns <row,col> index pair for the entry that is references, else <-1,-1> if invalid
static std::pair<int,int> refToRowCol(const std::string& ref) {
    size_t pos = 0, col = 0;
    while (pos < ref.length() && ref[pos] >= 'A' && ref[pos] <= 'Z') {
        col = col*26 + ref[pos++] - 'A' + 1;
    }

    auto row = stringToInt(ref.substr(pos, std::string::npos));

    if (col > 0 && row.has_value()) {
        return std::pair<int,int>(row.value() -1 ,col -1);
    } else {
        return std::pair<int,int>(-1, -1);
    }

}
