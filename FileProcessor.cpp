




//
//
//  Start takes three tokens not only two while still being a directive
//  Base takes two tokens not only one while still being a directive
//
//


#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <variant>
#include <sstream> // For stringstream
#include "Sicx.h"
#include <algorithm> // For transform

// Function to preprocess the line by replacing commas with spaces
std::string preprocessLine(const std::string& line) {
    std::string processedLine = line;
    for (char& ch : processedLine) {
        if (ch == ',') {
            ch = ' '; // Replace comma with space
        }
    }
    return processedLine;
}
std::string skipLeadingNumbers(const std::string& line) {
    std::size_t pos = 0;
    while (pos < line.size() && std::isdigit(line[pos])) {
        ++pos; // Skip numeric characters
    }
    // Skip any spaces following the numbers
    while (pos < line.size() && std::isspace(line[pos])) {
        ++pos;
    }
    return line.substr(pos); // Return the line starting from the first non-numeric character
}
std::string toUpperCase(const std::string& str) {
    std::string upperStr = str;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr;
}

// Function to read from a file and populate the list
void readFromFile(
    std::string filename,
    std::list<std::variant<
        sicx::Instruction<sicx::Format1>,
        sicx::Instruction<sicx::Format2>,
        sicx::Instruction<sicx::Format34>,
        sicx::Instruction<sicx::Format4f>,
        sicx::Instruction<sicx::Data>,
        sicx::Instruction<sicx::Directive>>>& lines) 
{
    std::string line;
    std::ifstream MyFile(filename);

    if (!MyFile) {
        return;
    }

    int lineNumber = 0;

    while (std::getline(MyFile, line)) {
        ++lineNumber;

        // Skip empty or comment lines
        if (line.empty() || line[0] == '.' || line[0] == ';') {
            continue;
        }

        // Skip leading numbers
        line = skipLeadingNumbers(line);

        // Preprocess the line to replace commas with spaces
        std::string processedLine = preprocessLine(line);

        std::istringstream lineStream(processedLine);
        std::string token1, token2, token3, token4, token5;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Tokenize the line (e.g., LABEL MNEMONIC OPERAND1 OPERAND2 OPERAND3)
        lineStream >> token1;
        token1 = toUpperCase(token1);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Handle format 4 plus sign
        std::string token1x;
        if (token1[0] == '+'){
            token1x = token1.substr(1);
        }else{
            token1x = token1;
        }


        auto it = sicx::opcodeTable.find(token1x);
        if (it != sicx::opcodeTable.end()) {
            const auto& opcodeDetails = it->second;

            // Create an appropriate Instruction object based on format
            switch (opcodeDetails.format) {
                case 1: {
                    sicx::Instruction<sicx::Format1> inst;
                    inst.label = "";
                    inst.inst = {lineNumber, 0, token1};
                    lines.push_back(inst);
                    std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                    break;
                }
                case 2: {
                    sicx::Instruction<sicx::Format2> inst;
                    inst.label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    lineStream >> token3;
                    token3 = toUpperCase(token3);
                    inst.inst = {lineNumber, 0, token1, token2, token3};
                    lines.push_back(inst);
                    std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                    break;
                }
                case 3: {
                    sicx::Instruction<sicx::Format34> inst;
                    inst.label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    inst.inst = {lineNumber, 0, 0, token1, token2};
                    lines.push_back(inst);
                    std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                    break;
                }
                case 4: {
                    sicx::Instruction<sicx::Format4f> inst;
                    inst.label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    lineStream >> token3;
                    token3 = toUpperCase(token3);
                    lineStream >> token4;
                    token4 = toUpperCase(token4);
                    inst.inst = {lineNumber, 0, token1, token2, token3, token4};
                    lines.push_back(inst);
                    std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                    break;
                }
            }
        }
        else{
            auto it = sicx::directiveTable.find(token1x);
            if (it != sicx::directiveTable.end()) {
                const auto& opcodeDetails = it->second;
                switch (opcodeDetails) {
                    case 0: {
                        sicx::Instruction<sicx::Directive> inst;
                        inst.label = "";
                        inst.inst = {lineNumber, token1};
                        lines.push_back(inst);
                        std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                        break;
                    }
                    case 1: {
                        sicx::Instruction<sicx::Data> inst;
                        inst.label = "";
                        lineStream >> token2;
                        token2 = toUpperCase(token2);
                        inst.inst = {lineNumber, 0, token1, token2};
                        lines.push_back(inst);
                        std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                        break;
                    }
                }

            }
            else{
                // Handle format 4 plus sign
                lineStream >> token2;
                token2 = toUpperCase(token2);
                std::string token2x;
                if (token2[0] == '+'){
                    token2x = token2.substr(1);
                }else{
                    token2x = token2;
                }
                auto it = sicx::opcodeTable.find(token2x);
                if (it != sicx::opcodeTable.end()) {
                    const auto& opcodeDetails = it->second;

                    // Create an appropriate Instruction object based on format
                    switch (opcodeDetails.format) {
                        case 1: {
                            sicx::Instruction<sicx::Format1> inst;
                            inst.label = token1;
                            inst.inst = {lineNumber, 0, token2};
                            lines.push_back(inst);
                            std::cerr << " -- " << token2 << " on line " << lineNumber << std::endl;
                            break;
                        }
                        case 2: {
                            sicx::Instruction<sicx::Format2> inst;
                            inst.label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);
                            lineStream >> token4;
                            token4 = toUpperCase(token4);
                            inst.inst = {lineNumber, 0, token2, token3, token4};
                            lines.push_back(inst);
                            std::cerr << " -- " << token2 << " on line " << lineNumber << std::endl;
                            break;
                        }
                        case 3: {
                            sicx::Instruction<sicx::Format34> inst;
                            inst.label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);
                            inst.inst = {lineNumber, 0, 0, token2, token3};
                            lines.push_back(inst);
                            std::cerr << " -- " << token1 << " on line " << lineNumber << std::endl;
                            break;
                        }
                        case 4: {
                            sicx::Instruction<sicx::Format4f> inst;
                            inst.label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);
                            lineStream >> token4;
                            token4 = toUpperCase(token4);
                            lineStream >> token5;
                            token5 = toUpperCase(token5);
                            inst.inst = {lineNumber, 0, token2, token3, token4, token5};
                            lines.push_back(inst);
                            std::cerr << " -- " << token2 << " on line " << lineNumber << std::endl;
                            break;
                        }
                    }
                }
                else{
                    auto it = sicx::directiveTable.find(token2x);
                    if (it != sicx::directiveTable.end()) {
                        const auto& opcodeDetails = it->second;
                        switch (opcodeDetails) {
                            case 0: {
                                sicx::Instruction<sicx::Directive> inst;
                                inst.label = token1;
                                inst.inst = {lineNumber, token2};
                                lines.push_back(inst);
                                std::cerr << " -- " << token2 << " on line " << lineNumber << std::endl;
                                break;
                            }
                            case 1: {
                                sicx::Instruction<sicx::Data> inst;
                                inst.label = token1;
                                lineStream >> token3;
                                token3 = toUpperCase(token3);
                                inst.inst = {lineNumber, 0, token2, token3};
                                lines.push_back(inst);
                                std::cerr << " -- " << token2 << " on line " << lineNumber << std::endl;
                                break;
                            }
                        }
                    }
                    else {
                        std::cerr << "Error: unexpected token " << " on line " << lineNumber << std::endl;
                    }
                }
            }
        }

        // Check for extra tokens
        std::string extraToken;
        
        if ((lineStream >> extraToken) && (extraToken[0] != ';' || extraToken[0] != '.')) {
            std::cerr << "Error: Unexpected extra token '" << extraToken
                      << "' on line " << lineNumber << ": " << processedLine << std::endl;
        }
    }

        

    MyFile.close();
}

