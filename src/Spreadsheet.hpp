#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <vector>
#include <string>

// Converts input file to 2d string spreadsheet. Returns status of operation.
bool fileToSpreadsheet(const char filename[], std::vector<std::vector<std::string>>& spreadsheet);

// Parse spreadsheet and process expressions contained in the spreadsheet. Returns false if any
// input is invalid.
bool processSpreadsheet(std::vector<std::vector<std::string>>& spreadsheet);

// Prints spreadsheet to default output stream
void printSpreadsheet(const std::vector<std::vector<std::string>>& spreadsheet);

#endif