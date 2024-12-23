

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

bool isHexadecimal(const std::string &str)
{
    size_t start = 0;

    // Check if the string starts with "0x" or "0X"
    if (str.size() > 2 && (str[0] == '0') && (str[1] == 'x' || str[1] == 'X'))
    {
        start = 2;
    }

    // Ensure the rest of the string contains only hexadecimal digits
    for (size_t i = start; i < str.size(); ++i)
    {
        if (!std::isxdigit(str[i]))
        {
            return false;
        }
    }

    // The string is a valid hexadecimal number
    return (start != str.size()); // Ensure it's not just "0x" without digits
}

unsigned int sizeOfLiteral(string lit, bool v)
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
        if (v)
            cout << "\33[92msize of \33[0m" << lit << " is " << x << endl;
        return (unsigned)x;
    }
    else
    {
        lit.erase(std::remove(lit.begin(), lit.end(), '\''), lit.end());
        int x = stoi(lit, 0, 10);
        x = (int)ceil(log2(x + 1) / 8);
        if (v)
            cout << "\33[92msize of \33[0m" << lit << " is " << x << endl;
        return x;
    }
}

unsigned int calculateProgramLength(const list<Instruction> &instructions, bool v)
{
    if (instructions.empty())
    {
        return 0; // No instructions in the list
    }

    unsigned int minAddress = UINT_MAX; // Start with the largest possible value
    unsigned int maxAddress = 0;        // Start with the smallest possible value

    for (const auto &inst : instructions)
    {
        unsigned int currentLocation = inst.location;

        // Find the size of the instruction or data
        unsigned int size = 0;
        if (holds_alternative<Data>(inst.inst))
        {
            if (get<Data>(inst.inst).type == "RESB")
            {
                if (get<Data>(inst.inst).value[0] == 'X' || get<Data>(inst.inst).value[0] == 'x')
                    size = 1 * stoi(get<Data>(inst.inst).value, 0, 16);
                else
                    size = 1 * stoi(get<Data>(inst.inst).value, 0, 10);
            }
            else if (get<Data>(inst.inst).type == "RESW")
            {
                if (get<Data>(inst.inst).value[0] == 'X' || get<Data>(inst.inst).value[0] == 'x')
                    size = 3 * stoi(get<Data>(inst.inst).value, 0, 16);
                else
                    size = 3 * stoi(get<Data>(inst.inst).value, 0, 10);
            }
            else
            {
                size = ceil((double)inst.objectCode.length() / 2);
            }
        }
        else
        {
            size = ceil((double)inst.objectCode.length() / 2);
        }

        // Update the min and max addresses
        minAddress = min(minAddress, currentLocation);
        maxAddress = max(maxAddress, currentLocation + size);
    }

    // The length is the difference between max and min addresses
    if (v)
        cout << "\33[92mprogram size was calculated to be: \33[0m" << maxAddress - minAddress << endl;
    return maxAddress - minAddress;
}

void preProcess(string &line, bool v)
{

    line = skipLeadingNumbers(line); // skip line number if it exists
    size_t pos = line.find('.');     // search for a dot
    if (pos != string::npos)
    {
        line.erase(pos); // remove everything in the line after the dot (the comment) if it exists
    }
    if (v)
        cout << "\33[92mpreprocessed line: \33[0m" << line << endl;
}

// read lines from file
int ReadLines(string path, string fileName, list<string> &lines, bool v)
{
    string line;
    ifstream file(path + fileName);

    if (!file)
    {
        if (v)
            cout << "\33[31mcouldn't open file" << endl;
        return -1;
    }
    if (v)
        cout << "\33[92mreading file " << path + fileName << endl;
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
int WriteLines(string path, string fileName, list<string> &lines, bool v)
{
    ofstream file(path + fileName);
    if (!file)
    {
        if (v)
            cout << "\33[31mcouldn't open file" << endl;
        return -1;
    }
    if (v)
        cout << "\33[92mreading file " << path + fileName << endl;
    for (string line : lines)
    {
        if (!line.empty())
            file << line << endl;
    }
    file.close();
    return 1;
}

int LOCCTRCalc(list<Instruction> &instructions, list<Symbol> &symbols, BlockMap &blockmap, bool v)
{
    unsigned int defloc, defbloc, dataloc, blksloc;
    unsigned int *loc;
    unsigned int startAddr;
    BLOCK bloc = DEFAULT;
    list<string> literals;
    list<tuple<int, string>> Literals;
    int index = 0;
    int indexx = 0;
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
                auto instit = instructions.begin();
                advance(instit, index + 1);
                for (string literal : literals)
                {
                    int x = sizeOfLiteral(literal, v);
                    symbls.push_back({*loc, literal, bloc});

                    Instruction w;
                    Directive d = {"*", literal};
                    w.inst = d;
                    w.location = *loc;
                    w.block = bloc;
                    instructions.insert(instit, w);
                    indexx++;

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
                if (get<Data>(inst.inst).value[0] == 'C')
                {
                    *loc += sizeOfLiteral("=" + get<Data>(inst.inst).value, v);
                }
                else
                {
                    *loc += 1;
                }
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
                {
                    literals.push_back(get<Format34>(inst.inst).memory);
                }
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
                {
                    literals.push_back(get<Format4f>(inst.inst).memory);
                }
            }
            inst.block = bloc;
            inst.location = *loc;
            *loc += 4;
        }
        index++;

        if (v)
            cout << "\33[92mcalculated the location of \33[0m" << inst.toString() << "\33[92m" << *loc << endl;
    }

    for (string literal : literals)
    {
        int x = sizeOfLiteral(literal, v);
        symbls.push_back({*loc, literal, bloc});
        *loc += x;
    }
    literals.clear();

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
        else if (get<Directive>(inst.inst).dir == "*")
        {
            inst.location += blockmap[inst.block].addr;
        }
        if (v)
            cout << "\33[92mcalculated the location of \33[0m" << inst.toString() << "\33[92m" << inst.location << endl;
    }
    for (auto &symbl : symbls)
    {
        get<0>(symbl) += blockmap[get<2>(symbl)].addr;
        symbols.push_back({get<0>(symbl), get<1>(symbl)});
        if (v)
            cout << "\33[92mcalculated the location of \33[0m" << get<1>(symbl) << "\33[92m" << get<0>(symbl) << endl;
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
        strings.push_back(x);
    }
}
int objCodeCalc(list<Instruction> &instructions, list<Symbol> &symbols, bool v)
{
    unsigned int baseptr = 0;

    for (Instruction &inst : instructions)
    { // Use reference to modify original instructions
        stringstream ss;

        if (holds_alternative<Format1>(inst.inst))
        {
            // Format 1: Single-byte opcode
            auto it = opcodeTable.find(get<Format1>(inst.inst).instruction);
            if (it != opcodeTable.end())
            {
                inst.objectCode = it->second.opcode; // Opcode is the object code
            }
        }
        else if (holds_alternative<Format2>(inst.inst))
        {
            // Format 2: Opcode + 2 registers
            auto it = opcodeTable.find(get<Format2>(inst.inst).instruction);
            if (it != opcodeTable.end())
            {
                string op = it->second.opcode;
                int reg1 = 0, reg2 = 0;

                // Get register numbers
                auto reg1it = registerTable.find(get<Format2>(inst.inst).register1);
                if (reg1it != registerTable.end())
                {
                    reg1 = reg1it->second;
                }
                auto reg2it = registerTable.find(get<Format2>(inst.inst).register2);
                if (reg2it != registerTable.end())
                {
                    reg2 = reg2it->second;
                }

                ss << op << uppercase << hex << reg1 << reg2;
                inst.objectCode = ss.str();
            }
        }
        else if (holds_alternative<Format34>(inst.inst))
        {
            // Format 3 and 4: Opcode + nixbpe + displacement/address
            auto it = opcodeTable.find(get<Format34>(inst.inst).instruction);
            if (it != opcodeTable.end())
            {
                string op = it->second.opcode;
                string label = get<Format34>(inst.inst).memory;

                unsigned int memaddr = 0;
                long dis = 0;

                // Find memory address of label
                auto memit = find_if(symbols.begin(), symbols.end(), [label](const Symbol &sym)
                                     { return std::get<1>(sym) == label; });

                bool _immf = false;
                if (memit != symbols.end())
                {
                    memaddr = std::get<0>(*memit); // Address from symbol table
                }
                else if (get<Format34>(inst.inst).nixbpe & 0b010000 && isHexadecimal(label))
                {
                    _immf = true;
                    memaddr = stoi(label, 0, 10);
                }
                else
                {
                    cout << "\33[31mUnable to resolve label: \33[0m" << label << endl;
                }

                // Extended (Format 4) or PC/Base relative addressing (Format 3)
                if (get<Format34>(inst.inst).nixbpe & 0b000001)
                { // Extended addressing
                    unsigned int opc = stoi(op, 0, 16) + ((get<Format34>(inst.inst).nixbpe & 0b111000) >> 4);
                    ss << setw(2) << setfill('0') << hex << opc;
                    ss << setw(1) << setfill('0') << hex << (get<Format34>(inst.inst).nixbpe & 0b001111);
                    ss << setw(5) << setfill('0') << hex << memaddr;
                }
                else
                { // PC or Base relative
                    char nixbpe = get<Format34>(inst.inst).nixbpe;
                    if (!_immf)
                    {
                        dis = static_cast<long>(memaddr) - static_cast<long>(inst.location + 3);

                        if (dis >= -2048 && dis <= 2047)
                        {                       // PC-relative
                            nixbpe |= 0b000010; // Set PC-relative flag
                        }
                        else
                        { // Base-relative
                            dis = static_cast<long>(memaddr) - static_cast<long>(baseptr);
                            nixbpe |= 0b000100; // Set Base-relative flag
                        }
                    }
                    else
                    {
                        dis = memaddr;
                    }
                    unsigned int opc = stoi(op, 0, 16) + ((nixbpe & 0b111000) >> 4);
                    ss << setw(2) << setfill('0') << hex << opc;
                    ss << setw(1) << setfill('0') << hex << (nixbpe & 0b001111);
                    ss << setw(3) << setfill('0') << hex << (dis & 0xFFF); // 12-bit displacement
                }

                inst.objectCode = ss.str();
            }
        }
        else if (holds_alternative<Format4f>(inst.inst))
        {
            // Format 4: Extended addressing with full address
            auto it = opcodeTable.find(get<Format4f>(inst.inst).instruction);
            if (it != opcodeTable.end())
            {
                // 6 - 4 - 2 - 20

                string op = it->second.opcode;
                string label = get<Format4f>(inst.inst).memory;

                unsigned int memaddr = 0;
                auto memit = find_if(symbols.begin(), symbols.end(), [label](const Symbol &sym)
                                     { return std::get<1>(sym) == label; });

                if (memit != symbols.end())
                {
                    memaddr = std::get<0>(*memit);
                }
                else
                {
                    cout << "\33[31mUnable to resolve label: \33[0m" << label << endl;
                }
                unsigned int opc = stoi(op, 0, 16);

                char reg = 0;
                auto regit = registerTable.find(get<Format4f>(inst.inst).register1);
                if (regit != registerTable.end())
                {
                    reg = regit->second;
                }
                char x = reg;
                opc += (x >> 2);
                reg = reg & 0b00000011;

                char cond = 0;

                if (toUpperCase(get<Format4f>(inst.inst).condition) == "Z")
                {
                    cond = 0b00;
                }
                else if (toUpperCase(get<Format4f>(inst.inst).condition) == "N")
                {
                    cond = 0b01;
                }
                else if (toUpperCase(get<Format4f>(inst.inst).condition) == "C")
                {
                    cond = 0b10;
                }
                else if (toUpperCase(get<Format4f>(inst.inst).condition) == "V")
                {
                    cond = 0b11;
                }

                cond += (reg << 2);

                ss << setw(2) << setfill('0') << hex << opc;
                ss << setw(1) << setfill('0') << hex << ((unsigned int)cond);
                ss << setw(5) << setfill('0') << hex << memaddr;

                cout << cond << endl;
                inst.objectCode = ss.str();
            }
        }
        else if (holds_alternative<Data>(inst.inst))
        {
            // Data instructions (e.g., BYTE, WORD)
            if (get<Data>(inst.inst).type == "BYTE")
            {
                string value = get<Data>(inst.inst).value;
                if (value[0] == 'X')
                {
                    // Hexadecimal literal
                    inst.objectCode = value.substr(2, value.size() - 3); // Remove X' and '
                }
                else if (value[0] == 'C')
                {
                    // Character literal
                    string chars = value.substr(2, value.size() - 3); // Remove C' and '
                    for (char c : chars)
                    {
                        ss << hex << (int)c;
                    }
                    inst.objectCode = ss.str();
                }
            }
            else if (get<Data>(inst.inst).type == "WORD")
            {
                int wordValue = stoi(get<Data>(inst.inst).value);
                ss << setw(6) << setfill('0') << hex << wordValue;
                inst.objectCode = ss.str();
            }
        }
        else if (holds_alternative<Directive>(inst.inst))
        {
            // Directives (e.g., BASE, NOBASE)
            if (get<Directive>(inst.inst).dir == "BASE")
            {
                string label = get<Directive>(inst.inst).T;
                auto memit = find_if(symbols.begin(), symbols.end(), [label](const Symbol &sym)
                                     { return std::get<1>(sym) == label; });
                if (memit != symbols.end())
                {
                    baseptr = std::get<0>(*memit); // Update base register
                }
                else
                {
                    cout << "\33[31mUnable to resolve label: \33[0m" << label << endl;
                }
            }
            else if (get<Directive>(inst.inst).dir == "*")
            {
                auto lit = get<Directive>(inst.inst).T;
                if (lit[1] == 'C' || lit[1] == 'c')
                {
                    // return lit.length() - 4;
                    stringstream ss;
                    for (int i = 3; i < lit.length() - 1; i++)
                    {
                        ss << setw(2) << setfill('0') << hex << ((unsigned int)lit[i]);
                    }
                    inst.objectCode = ss.str();
                }
                else if (lit[1] == 'x' || lit[1] == 'X')
                {
                    stringstream ss;
                    lit = lit.substr(3);
                    unsigned int q = stoi(lit, 0, 16);
                    ss << hex << q;
                    inst.objectCode = ss.str();

                    // return (unsigned)x;
                }
            }
        }
        if (v)
            cout << "\33[92mcalculated the object code of \33[0m" << inst.toString() << "\33[92m" << inst.objectCode << endl;
    }

    return 0; // Return success
}
void generateHTMERecords(const list<Instruction> &instructions, list<string> &htmeRecords, bool v)
{

    list<string> mRecords;
    stringstream ss;
    string currentTextRecord = "";
    unsigned int startAddress = 0;
    unsigned int currentAddress = 0;
    bool textRecordOpen = false;
    unsigned int textRecordSize = 0; // Track size of current T record

    // Add Header record
    ss << "H^";
    auto firstInst = instructions.begin();
    if (firstInst != instructions.end())
    {
        ss << setw(6) << setfill(' ') << left << (firstInst->label.empty() ? "NONAME" : firstInst->label.substr(0, 6)) << "^";
        ss << setw(6) << setfill('0') << right << hex << uppercase << firstInst->location << "^";
        ss << setw(6) << setfill('0') << right << hex << uppercase << calculateProgramLength(instructions, v); // Approx program length
    }
    htmeRecords.push_back(ss.str());
    ss.str("");
    ss.clear();
    if (v)
        cout << "\33[92mCalculated H Record" << endl;

    for (const auto &inst : instructions)
    {
        // Text Record
        if (!inst.objectCode.empty())
        {
            if (!textRecordOpen)
            {
                // Open a new text record
                startAddress = inst.location;
                currentTextRecord = "T^" + (stringstream() << setw(6) << setfill('0') << uppercase << hex << startAddress).str() + "^";
                textRecordOpen = true;
                textRecordSize = 0;
            }

            // Check if adding the object code exceeds 1E (30 bytes)
            if (textRecordSize + inst.objectCode.size() / 2 > 0x1E)
            {
                // Close the current text record
                unsigned int length = textRecordSize;
                currentTextRecord.insert(9, (stringstream() << setw(2) << setfill('0') << uppercase << hex << length << '^').str());
                htmeRecords.push_back(currentTextRecord);

                // Start a new text record
                startAddress = inst.location;
                currentTextRecord = "T^" + (stringstream() << setw(6) << setfill('0') << uppercase << hex << startAddress).str() + "^";
                textRecordSize = 0;
            }

            currentTextRecord += inst.objectCode + "^";
            currentAddress = inst.location;
            textRecordSize += ceil((double)inst.objectCode.size() / 2); // Increment size in bytes
        }
        else if (holds_alternative<Directive>(inst.inst))
        {
            if (get<Directive>(inst.inst).dir == "USE")
            {
                // Close the current text record if open
                if (textRecordOpen)
                {
                    unsigned int length = textRecordSize;
                    currentTextRecord.insert(9, (stringstream() << setw(2) << setfill('0') << uppercase << hex << length << '^').str());
                    htmeRecords.push_back(currentTextRecord);
                    textRecordOpen = false;
                }
            }
        }
        else if (holds_alternative<Data>(inst.inst))
        {
            if (get<Data>(inst.inst).type == "RESW" || get<Data>(inst.inst).type == "RESB")
            {
                // Close the current text record if open
                if (textRecordOpen)
                {
                    unsigned int length = textRecordSize;
                    currentTextRecord.insert(9, (stringstream() << setw(2) << setfill('0') << uppercase << hex << length << '^').str());
                    htmeRecords.push_back(currentTextRecord);
                    textRecordOpen = false;
                }
            }
        }

        // Modification Record
        if (holds_alternative<Format34>(inst.inst))
        {
            if ((get<Format34>(inst.inst).nixbpe & 0b000001) && !((get<Format34>(inst.inst).nixbpe & 0b010000) && (~(get<Format34>(inst.inst).nixbpe) & 0b100000)))
            { // Extended addressing
                ss << "M^" << setw(6) << setfill('0') << hex << uppercase << inst.location + 1 << "^05";
                mRecords.push_back(ss.str());
                ss.str("");
                ss.clear();
            }
        }
        else if (holds_alternative<Format4f>(inst.inst))
        {
            ss << "M^" << setw(6) << setfill('0') << hex << uppercase << inst.location + 1 << "^05";
            mRecords.push_back(ss.str());
            ss.str("");
            ss.clear();
        }
    }

    // Close any remaining text record
    if (textRecordOpen)
    {
        unsigned int length = textRecordSize;
        currentTextRecord.insert(9, (stringstream() << setw(2) << setfill('0') << uppercase << hex << length << '^').str());
        htmeRecords.push_back(currentTextRecord);
    }

    // add M record
    for (auto m : mRecords)
    {
        htmeRecords.push_back(m);
    }
    // Add End record
    ss << "E^" << setw(6) << setfill('0') << hex << uppercase << instructions.front().location;
    htmeRecords.push_back(ss.str());
}

#endif