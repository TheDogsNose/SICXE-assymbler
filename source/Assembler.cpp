

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
#include "formats.cpp"
#include <algorithm> // For transform
#include <cmath>
#include <iomanip>

using namespace std;
using namespace sicx;
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

unsigned int sizeOfLiteral(string lit)
{
    if (lit[1] == 'C' || lit[1] == 'c')
    {
        return lit.length() - 4;
    }
    else if (lit[1] == 'x' || lit[1] == 'X')
    {
        lit.erase(std::remove(lit.begin(), lit.end(), '\''), lit.end());
        int x = lit.length() - 2;
        x = (int)ceil((double)x / 2.0);
        return (unsigned)x;
    }
    else
    {
        lit.erase(std::remove(lit.begin(), lit.end(), '\''), lit.end());
        int x = stoi(lit, 0, 10);
        return (int)ceil(log2(x + 1) / 8);
    }
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

        size_t pos = 0;
        // Skip any spaces at the start
        while (pos < line.size() && isspace(line[pos]))
        {
            ++pos;
        }
        line = line.substr(pos);

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

int LOCCTRCalc(list<Instruction> &instructions, list<Symbol> &symbols, BlockMap &blockmap)
{
    unsigned int defloc, defbloc, dataloc, blksloc;
    unsigned int *loc;
    unsigned int startAddr;
    BLOCK bloc = DEFAULT;
    list<string> literals;
    loc = &defloc;
    list<tuple<unsigned int, string, BLOCK>> symbls;
    blockmap = {
        {
            DEFAULT,
            {0, 0},
        },
        {
            DEFAULTB,
            {0, 0},
        },
        {
            CDATA,
            {0, 0},
        },
        {
            CBLKS,
            {0, 0},
        },
    };

    for (sicx::Instruction &inst : instructions)
    {
        cout << hex << *loc << endl;
        if (holds_alternative<Directive>(inst.inst))
        {
            if (get<Directive>(inst.inst).dir == "START")
            {
                unsigned int x = stoi(get<Directive>(inst.inst).T, 0, 16);
                *loc = x;
                defloc = x;
                defbloc = x;
                dataloc = x;
                blksloc = x;
                startAddr = x;
            }
            else if (get<Directive>(inst.inst).dir == "ORG")
            {
                *loc = stoi(get<Directive>(inst.inst).T, 0, 16);
            }
            else if (get<Directive>(inst.inst).dir == "LTORG")
            {
                for (string literal : literals)
                {
                    int x = sizeOfLiteral(literal);
                    symbls.push_back({*loc, literal, bloc});
                    *loc += x;
                }
                literals.clear();
            }
            else if (get<Directive>(inst.inst).dir == "USE")
            {
                if (get<Directive>(inst.inst).T == "DEFAULT" || get<Directive>(inst.inst).T.empty())
                {
                    loc = &defloc;
                    bloc = DEFAULT;
                }
                else if (get<Directive>(inst.inst).T == "DEFAULTB")
                {
                    loc = &defbloc;
                    bloc = DEFAULTB;
                }
                else if (get<Directive>(inst.inst).T == "CDATA")
                {
                    loc = &dataloc;
                    bloc = CDATA;
                }
                else if (get<Directive>(inst.inst).T == "CBLKS")
                {
                    loc = &blksloc;
                    bloc = CBLKS;
                }
                else
                {
                    cout << "Unkown Block name: USE\t" << get<Directive>(inst.inst).T << endl;
                    return -5;
                }
            }
        }
        else if (holds_alternative<Data>(inst.inst))
        {
            if (get<Data>(inst.inst).type == "BYTE")
            {
                if (!inst.label.empty())
                {
                    tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                    symbls.push_back(x);
                }
                inst.block = bloc;
                inst.location = *loc;
                *loc += 1;
            }
            else if (get<Data>(inst.inst).type == "WORD")
            {
                if (!inst.label.empty())
                {
                    tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                    symbls.push_back(x);
                }
                inst.block = bloc;
                inst.location = *loc;
                *loc += 3;
            }
            else if (get<Data>(inst.inst).type == "RESB")
            {
                if (!inst.label.empty())
                {
                    tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                    symbls.push_back(x);
                }
                inst.location = *loc;
                if (get<Data>(inst.inst).value[0] == 'X' || get<Data>(inst.inst).value[0] == 'x')
                    *loc += 1 * stoi(get<Data>(inst.inst).value, 0, 16);
                else
                    *loc += 1 * stoi(get<Data>(inst.inst).value, 0, 10);
                inst.block = bloc;
            }
            else if (get<Data>(inst.inst).type == "RESW")
            {
                if (!inst.label.empty())
                {
                    tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                    symbls.push_back(x);
                }
                inst.location = *loc;
                if (get<Data>(inst.inst).value[0] == 'X' || get<Data>(inst.inst).value[0] == 'x')
                    *loc += 3 * stoi(get<Data>(inst.inst).value, 0, 16);
                else
                    *loc += 3 * stoi(get<Data>(inst.inst).value, 0, 10);
                inst.block = bloc;
            }
        }
        else if (holds_alternative<Format1>(inst.inst))
        {
            if (!inst.label.empty())
            {
                tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                symbls.push_back(x);
            }
            inst.block = bloc;
            inst.location = *loc;
            *loc += 1;
        }
        else if (holds_alternative<Format2>(inst.inst))
        {
            if (!inst.label.empty())
            {
                tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                symbls.push_back(x);
            }
            inst.block = bloc;
            inst.location = *loc;
            *loc += 2;
        }
        else if (holds_alternative<Format34>(inst.inst))
        {
            if (!inst.label.empty())
            {
                tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                symbls.push_back(x);
            }
            if (get<Format34>(inst.inst).memory[0] == '=')
            {
                auto it = std::find(literals.begin(), literals.end(), get<Format34>(inst.inst).memory);
                if (it == literals.end())
                    literals.push_back(get<Format34>(inst.inst).memory);
            }
            inst.block = bloc;
            inst.location = *loc;
            if (get<Format34>(inst.inst).nixbpe & 1)
                *loc += 4;
            else
                *loc += 3;
        }
        else if (holds_alternative<Format4f>(inst.inst))
        {
            if (!inst.label.empty())
            {
                tuple<unsigned int, string, BLOCK> x(*loc, inst.label, bloc);
                symbls.push_back(x);
            }
            if (get<Format4f>(inst.inst).memory[0] == '=')
            {
                auto it = std::find(literals.begin(), literals.end(), get<Format4f>(inst.inst).memory);
                if (it == literals.end())
                    literals.push_back(get<Format4f>(inst.inst).memory);
            }
            inst.block = bloc;
            inst.location = *loc;
            *loc += 4;
        }
    }
    blockmap[DEFAULT] = {startAddr, defloc};
    blockmap[DEFAULTB] = {startAddr + defloc, defbloc};
    blockmap[CDATA] = {defbloc + startAddr + defloc, dataloc};
    blockmap[CBLKS] = {defbloc + startAddr + defloc + dataloc, blksloc};

    for (Instruction &inst : instructions)
    {
        if (!holds_alternative<Directive>(inst.inst))
        {
            inst.location += blockmap[inst.block].addr;
        }
    }
    for (auto &symbl : symbls)
    {
        get<0>(symbl) += blockmap[get<2>(symbl)].addr;
        symbols.push_back({get<0>(symbl), get<1>(symbl)});
    }

    return 0;
}

void symbolsToString(list<string> &strings, list<Symbol> &symbols)
{

    for (auto symb : symbols)
    {
        stringstream ss;
        ss << setw(4) << setfill('0') << hex << uppercase << get<0>(symb);

        string hexStr = ss.str();

        string x = hexStr + "\t" + get<1>(symb);
        cout << x << endl;
        strings.push_back(x);
    }
}

/*
 variant<sicx::Instruction<sicx::Format1>,
         sicx::Instruction<sicx::Format2>,
         sicx::Instruction<sicx::Format34>,
         sicx::Instruction<sicx::Format4f>,
         sicx::Instruction<sicx::Data>,
         sicx::Instruction<sicx::Directive>, int>
 getInstObj(string line)
 {

     istringstream lineStream(line);
     string token1, token2, token3, token4, token5;

     ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     // Tokenize the line (e.g., LABEL MNEMONIC OPERAND1 OPERAND2 OPERAND3)
     lineStream >> token1;
     token1 = toUpperCase(token1);
     ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

     // Handle format 4 plus sign
     string token1x;
     bool ext;
     if (token1[0] == '+')
     {
         token1x = token1.substr(1);
         ext = true;
     }
     else
     {
         token1x = token1;
         ext = false;
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
             inst.inst = {token1};
             return inst;
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
             inst.inst = {token1, token2, token3};
             return inst;
             break;
         }
         case 3:
         {
             sicx::Instruction<sicx::Format34> inst;
             inst.label = "";
             lineStream >> token2;
             token2 = toUpperCase(token2);
             inst.inst = {0, token1, token2};
             return inst;
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
             inst.inst = {token1, token2, token3, token4};
             return inst;
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
                 inst.inst = {token1};
                 return inst;
                 break;
             }
             case 1:
             {
                 sicx::Instruction<sicx::Data> inst;
                 inst.label = "";
                 lineStream >> token2;
                 token2 = toUpperCase(token2);
                 inst.inst = {token1, token2};
                 return inst;
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
                 ext = true;
             }
             else
             {
                 token2x = token2;
                 ext = false;
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
                     inst.inst = {token2};
                     return inst;
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
                     inst.inst = {token2, token3, token4};
                     return inst;
                     break;
                 }
                 case 3:
                 {
                     sicx::Instruction<sicx::Format34> inst;
                     inst.label = token1;
                     lineStream >> token3;
                     token3 = toUpperCase(token3);
                     inst.inst = {0, token2, token3};
                     return inst;
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
                     inst.inst = {token2, token3, token4, token5};
                     return inst;
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
                         inst.inst = {token2};
                         return inst;
                         break;
                     }
                     case 1:
                     {
                         sicx::Instruction<sicx::Data> inst;
                         inst.label = token1;
                         lineStream >> token3;
                         token3 = toUpperCase(token3);
                         inst.inst = {token2, token3};
                         return inst;
                         break;
                     }
                     }
                 }
                 else
                 {
                     return -1;
                 }
             }
         }
     }

     // Check for extra tokens
     string extraToken;

     if ((lineStream >> extraToken) && (extraToken[0] != ';' || extraToken[0] != '.'))
     {
         return -2;
     }
 }
 */
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