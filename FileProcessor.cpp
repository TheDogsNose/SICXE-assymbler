

//
//
//  Start takes three tokens not only two while still being a directive
//  Base takes two tokens not only one while still being a directive
//  Handel ascii spaces in string literals
//
//

//
//
//  we have three things to keep in mind while LOCCTR:
//  inst format, data resv size, and directive such as ORG, LTROG.
//  also keep in mind EQU while symbol table.
//
//

#ifndef _fproc
#define _fproc

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <variant>
#include <sstream> // For stringstream
#include "Sicx.h"
#include <algorithm> // For transform

using namespace std;

// Function to preprocess the line by replacing commas with spaces
string preprocessLine(const string &line)
{
    string processedLine = line;
    for (char &ch : processedLine)
    {
        if (ch == ',')
        {
            ch = ' '; // Replace comma with space
        }
    }
    return processedLine;
}
string skipLeadingNumbers(const string &line)
{
    size_t pos = 0;
    while (pos < line.size() && isdigit(line[pos]))
    {
        ++pos; // Skip numeric characters
    }
    // Skip any spaces following the numbers
    while (pos < line.size() && isspace(line[pos]))
    {
        ++pos;
    }
    return line.substr(pos); // Return the line starting from the first non-numeric character
}
string toUpperCase(const string &str)
{
    string upperStr = str;
    transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr;
}

void preProcess(string &line)
{

    line = skipLeadingNumbers(line); // skip line number if it exists
    size_t pos = line.find('.');     // search for a dot
    if (pos != string::npos)
    {
        line.erase(pos); // remove everything in the line after the dot (the comment) if it exists
    }
}

// read lines from file
int ReadLines(string path, string fileName, list<string> &lines)
{
    string line;
    ifstream file(path + fileName);

    if (!file)
    {
        return -1;
    }

    int lineNumber = 0;
    while (getline(file, line))
    {
        ++lineNumber;

        // Skip empty lines
        if (line.empty())
        {
            continue;
        }

        lines.push_back(line);
    }
    file.close();
    return lineNumber;
}

// write list of lines into a file
int WriteLines(string path, string fileName, list<string> &lines)
{
    ofstream file(path + fileName);
    if (!file)
    {
        return 0;
    }

    for (string line : lines)
    {
        if (!line.empty())
            file << line << endl;
    }
    file.close();
    return 1;
}

/*
string filePreProcessing(string filename)
{
    string line;
    ifstream file(filename);
    if (!file)
    {
        return;
    }

    line = skipLeadingNumbers(line);
}

// Function to read from a file and populate the list
void readFromFile(
    string filename,
    list<variant<
        sicx::Instruction<sicx::Format1>,
        sicx::Instruction<sicx::Format2>,
        sicx::Instruction<sicx::Format34>,
        sicx::Instruction<sicx::Format4f>,
        sicx::Instruction<sicx::Data>,
        sicx::Instruction<sicx::Directive>>> &lines)
{
    string line;
    ifstream MyFile(filename);

    if (!MyFile)
    {
        return;
    }

    int lineNumber = 0;

    while (getline(MyFile, line))
    {
        ++lineNumber;

        // Skip empty or comment lines
        if (line.empty() || line[0] == '.' || line[0] == ';')
        {
            continue;
        }

        // Skip leading numbers
        line = skipLeadingNumbers(line);

        // Preprocess the line to replace commas with spaces
        string processedLine = preprocessLine(line);

        istringstream lineStream(processedLine);
        string token1, token2, token3, token4, token5;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Tokenize the line (e.g., LABEL MNEMONIC OPERAND1 OPERAND2 OPERAND3)
        lineStream >> token1;
        token1 = toUpperCase(token1);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Handle format 4 plus sign
        string token1x;
        if (token1[0] == '+')
        {
            token1x = token1.substr(1);
        }
        else
        {
            token1x = token1;
        }

        auto it = sicx::opcodeTable.find(token1x);
        if (it != sicx::opcodeTable.end())
        {
            const auto &opcodeDetails = it->second;

            // Create an appropriate Instruction object based on format
            switch (opcodeDetails.format)
            {
            case 1:
            {
                sicx::Instruction<sicx::Format1> inst;
                inst.label = "";
                inst.inst = {lineNumber, 0, token1};
                lines.push_back(inst);
                cerr << " -- " << token1 << " on line " << lineNumber << endl;
                break;
            }
            case 2:
            {
                sicx::Instruction<sicx::Format2> inst;
                inst.label = "";
                lineStream >> token2;
                token2 = toUpperCase(token2);
                lineStream >> token3;
                token3 = toUpperCase(token3);
                inst.inst = {lineNumber, 0, token1, token2, token3};
                lines.push_back(inst);
                cerr << " -- " << token1 << " on line " << lineNumber << endl;
                break;
            }
            case 3:
            {
                sicx::Instruction<sicx::Format34> inst;
                inst.label = "";
                lineStream >> token2;
                token2 = toUpperCase(token2);
                inst.inst = {lineNumber, 0, 0, token1, token2};
                lines.push_back(inst);
                cerr << " -- " << token1 << " on line " << lineNumber << endl;
                break;
            }
            case 4:
            {
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
                cerr << " -- " << token1 << " on line " << lineNumber << endl;
                break;
            }
            }
        }
        else
        {
            auto it = sicx::directiveTable.find(token1x);
            if (it != sicx::directiveTable.end())
            {
                const auto &opcodeDetails = it->second;
                switch (opcodeDetails)
                {
                case 0:
                {
                    sicx::Instruction<sicx::Directive> inst;
                    inst.label = "";
                    inst.inst = {lineNumber, token1};
                    lines.push_back(inst);
                    cerr << " -- " << token1 << " on line " << lineNumber << endl;
                    break;
                }
                case 1:
                {
                    sicx::Instruction<sicx::Data> inst;
                    inst.label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    inst.inst = {lineNumber, 0, token1, token2};
                    lines.push_back(inst);
                    cerr << " -- " << token1 << " on line " << lineNumber << endl;
                    break;
                }
                }
            }
            else
            {
                // Handle format 4 plus sign
                lineStream >> token2;
                token2 = toUpperCase(token2);
                string token2x;
                if (token2[0] == '+')
                {
                    token2x = token2.substr(1);
                }
                else
                {
                    token2x = token2;
                }
                auto it = sicx::opcodeTable.find(token2x);
                if (it != sicx::opcodeTable.end())
                {
                    const auto &opcodeDetails = it->second;

                    // Create an appropriate Instruction object based on format
                    switch (opcodeDetails.format)
                    {
                    case 1:
                    {
                        sicx::Instruction<sicx::Format1> inst;
                        inst.label = token1;
                        inst.inst = {lineNumber, 0, token2};
                        lines.push_back(inst);
                        cerr << " -- " << token2 << " on line " << lineNumber << endl;
                        break;
                    }
                    case 2:
                    {
                        sicx::Instruction<sicx::Format2> inst;
                        inst.label = token1;
                        lineStream >> token3;
                        token3 = toUpperCase(token3);
                        lineStream >> token4;
                        token4 = toUpperCase(token4);
                        inst.inst = {lineNumber, 0, token2, token3, token4};
                        lines.push_back(inst);
                        cerr << " -- " << token2 << " on line " << lineNumber << endl;
                        break;
                    }
                    case 3:
                    {
                        sicx::Instruction<sicx::Format34> inst;
                        inst.label = token1;
                        lineStream >> token3;
                        token3 = toUpperCase(token3);
                        inst.inst = {lineNumber, 0, 0, token2, token3};
                        lines.push_back(inst);
                        cerr << " -- " << token1 << " on line " << lineNumber << endl;
                        break;
                    }
                    case 4:
                    {
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
                        cerr << " -- " << token2 << " on line " << lineNumber << endl;
                        break;
                    }
                    }
                }
                else
                {
                    auto it = sicx::directiveTable.find(token2x);
                    if (it != sicx::directiveTable.end())
                    {
                        const auto &opcodeDetails = it->second;
                        switch (opcodeDetails)
                        {
                        case 0:
                        {
                            sicx::Instruction<sicx::Directive> inst;
                            inst.label = token1;
                            inst.inst = {lineNumber, token2};
                            lines.push_back(inst);
                            cerr << " -- " << token2 << " on line " << lineNumber << endl;
                            break;
                        }
                        case 1:
                        {
                            sicx::Instruction<sicx::Data> inst;
                            inst.label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);
                            inst.inst = {lineNumber, 0, token2, token3};
                            lines.push_back(inst);
                            cerr << " -- " << token2 << " on line " << lineNumber << endl;
                            break;
                        }
                        }
                    }
                    else
                    {
                        cerr << "Error: unexpected token " << " on line " << lineNumber << endl;
                    }
                }
            }
        }

        // Check for extra tokens
        string extraToken;

        if ((lineStream >> extraToken) && (extraToken[0] != ';' || extraToken[0] != '.'))
        {
            cerr << "Error: Unexpected extra token '" << extraToken
                 << "' on line " << lineNumber << ": " << processedLine << endl;
        }
    }

    MyFile.close();
}
*/

#endif