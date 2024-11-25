#ifndef GAME_OF_LIFE_RLE_H
#define GAME_OF_LIFE_RLE_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include "Pattern.h"
#include "accessors.h"

void readDimensions(std::string line, Pattern &result) {
    // parses the dimensions from the file header
    // remove spaces first to improve resiliency
    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

    // dimensions are in the format 'x = 10, y = 10[, rule= ...]'
    auto firstEqual = line.find('=');
    auto secondEqual = line.find('=', firstEqual + 1);
    result.columns = std::stoi(line.substr(firstEqual + 1));
    result.rows = std::stoi(line.substr(secondEqual + 1));
    // TODO: maybe parse the rules and check if they are compatible?
}

void readData(std::ifstream &infile, Pattern &result) {
    // Parses the data in the file after the header has been read
    result.contents.resize(result.columns * result.rows);
    int x = 0;
    int y = 0;

    std::string line;
    while (getline(infile, line)) {
        size_t cursor = 0;
        int count = 1;

        // remove possible line endings
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

        while (cursor < line.length() && line.at(cursor) != '!') {
            line = line.substr(cursor);
            cursor = 0;

            // case 1: a '$' symbol indicates the next row
            if (line.at(cursor) == '$') {
                cursor++;
                x = 0;
                y = y + 1;
                continue;
            }

            // case 2: the cursor points to a digit - we use stoi to parse the number and have
            // cursor pointed to the first non-digit after it
            if (isdigit(line.at(cursor))) {
                count = std::stoi(line, &cursor);

                // 'b' indicates a dead cell, 'o' is a living cell
                // we take a shortcut by assuming those two are the only possible values here
                uint_fast8_t cellState = line.at(cursor) == 'b' ? 0u : 1u;

                for (int n = 0; n < count; n++) {
                    set(result.contents, result.columns, y, x + n, cellState);
                }

                // move x and cursorStart forward
                x += count;
                cursor += 1;
                continue;
            }

            // case 3: the cursor points at a letter - this is a litle easier than the digit case
            if (isalpha(line.at(cursor))) {
                uint_fast8_t cellState = line.at(cursor) == 'b' ? 0u : 1u;
                set(result.contents, result.columns, y, x, cellState);
                x += 1;
                cursor += 1;
                continue;
            }
        }
    }
}

void readComment(std::string line, Pattern &result) {
    auto content = line.substr(2);
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());

    if (line.at(0) == 'N') {
        result.name = content;
    } else if (line.at(0) == 'O') {
        result.author = content;
    } else if (line.at(0) == 'C') {
        result.comments.push_back(content);
    }
}

Pattern readRLE(const std::string &filename) {
    // parses an RLE file and returns a Pattern according to its contents
    std::ifstream infile(filename);

    if (!infile.is_open()) {
        throw std::invalid_argument("Failed to open input file '" + filename + "'");
    }

    Pattern result;
    std::string line;

    // read file until we arrived at the header containing dimensions and rules
    while (getline(infile, line)) {
        // lines starting with # are comments
        if (line.at(0) == '#') {
            readComment(line.substr(1), result);
        }

        // the header has the form 'x = m, y = n'
        // once we found it and got our dimensions we are done here and leave the rest to readData
        if (line.at(0) == 'x') {
            readDimensions(line, result);
            break;
        }
    }

    readData(infile, result);
    infile.close();
    return result;
}

#endif //GAME_OF_LIFE_RLE_H
