
#ifndef _insttt
#define _insttt
#include <unordered_map>
#include <string>
using namespace std;
namespace sicx
{
    enum BLOCK
    {
        DEFAULT,
        DEFAULTB,
        CDATA,
        CBLKS
    };
    struct _inst
    {
        string mnemonic;
        string opcode;
        int format;
    };
    struct blockinfo
    {
        unsigned long int addr;
        unsigned int size;
    };

    typedef unordered_map<BLOCK, blockinfo> BlockMap;
    typedef tuple<unsigned int, string> Symbol;

    unordered_map<string, _inst>
        opcodeTable = {
            {"FLOAT", {"FLOAT", "C0", 1}},
            {"FIX", {"FIX", "C4", 1}},
            {"HIO", {"HIO", "F4", 1}},
            {"NORM", {"NORM", "C8", 1}},
            {"SIO", {"SIO", "F0", 1}},
            {"TIO", {"TIO", "F8", 1}},

            {"ADDR", {"ADDR", "90", 2}},
            {"CLEAR", {"CLEAR", "B4", 2}},
            {"COMPR", {"COMPR", "A0", 2}},
            {"DIVR", {"DIVR", "9C", 2}},
            {"MULR", {"MULR", "98", 2}},
            {"RMO", {"RMO", "AC", 2}},
            {"SHIFTL", {"SHIFTL", "A4", 2}},
            {"SHIFTR", {"SHIFTR", "A8", 2}},
            {"SUBR", {"SUBR", "94", 2}},
            {"SVC", {"SVC", "B0", 2}},
            {"TIXR", {"TIXR", "B8", 2}},

            {"ADD", {"ADD", "18", 3}},
            {"ADDF", {"ADDF", "58", 3}},
            {"AND", {"AND", "40", 3}},
            {"COMP", {"COMP", "28", 3}},
            {"COMPF", {"COMPF", "88", 3}},
            {"DIV", {"DIV", "24", 3}},
            {"DIVF", {"DIVF", "64", 3}},
            {"J", {"J", "3C", 3}},
            {"JEQ", {"JEQ", "30", 3}},
            {"JGT", {"JGT", "34", 3}},
            {"JLT", {"JLT", "38", 3}},
            {"JSUB", {"JSUB", "48", 3}},
            {"LDA", {"LDA", "00", 3}},
            {"LDB", {"LDB", "68", 3}},
            {"LDCH", {"LDCH", "50", 3}},
            {"LDF", {"LDF", "70", 3}},
            {"LDL", {"LDL", "08", 3}},
            {"LDS", {"LDS", "6C", 3}},
            {"LDT", {"LDT", "74", 3}},
            {"LDX", {"LDX", "04", 3}},
            {"LPS", {"LPS", "D0", 3}},
            {"MUL", {"MUL", "20", 3}},
            {"MULF", {"MULF", "60", 3}},
            {"OR", {"OR", "44", 3}},
            {"RD", {"RD", "D8", 3}},
            {"RSUB", {"RSUB", "4C", 3}},
            {"SSK", {"SSK", "EC", 3}},
            {"STA", {"STA", "0C", 3}},
            {"STB", {"STB", "78", 3}},
            {"STCH", {"STCH", "54", 3}},
            {"STF", {"STF", "80", 3}},
            {"STI", {"STI", "D4", 3}},
            {"STL", {"STL", "14", 3}},
            {"STS", {"STS", "7C", 3}},
            {"STSW", {"STSW", "E8", 3}},
            {"STT", {"STT", "84", 3}},
            {"STX", {"STX", "10", 3}},
            {"SUB", {"SUB", "1C", 3}},
            {"SUBF", {"SUBF", "5C", 3}},
            {"TD", {"TD", "E0", 3}},
            {"TIX", {"TIX", "2C", 3}},
            {"WD", {"WD", "DC", 3}},

            {"CADD", {"CADD", "BC", 4}},
            {"CSUB", {"CSUB", "8C", 4}},
            {"CLOAD", {"CLOAD", "E4", 4}},
            {"CSTORE", {"CSTORE", "FC", 4}},
            {"CJUMP", {"CJUMP", "CC", 4}},
    };

    unordered_map<string, int> directiveTable = {
        {"START", 3},
        {"END", 1},
        {"BYTE", 2},
        {"WORD", 2},
        {"RESB", 2},
        {"RESW", 2},
        {"BASE", 1},
        {"NOBASE", 0},
        {"EQU", 3},
        {"ORG", 1},
        {"LTORG", 0},
        {"USE", 1},
        //{"EXTDEF", 1},
        //{"EXTREF", 1},
        //{"CSECT", },
    };
}

#endif