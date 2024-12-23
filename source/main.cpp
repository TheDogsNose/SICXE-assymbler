#include <iostream>
#include <fstream>
#include <string>
#include "Assembler.cpp"
#include <list>
#include <variant>
#include "formats.cpp"
#include <tuple>
#include <iomanip>

#ifdef _WIN32
#include <io.h>
#define access _access_s
#else
#include <unistd.h>
#endif

using namespace std;

void displayHelp()
{
    cout << "\033[1;32mUsage:\033[0m" << endl;
    cout << "  assembler File [options]..." << endl;
    cout << "\n\033[1;32mOptions:\033[0m" << endl;
    cout << "  \033[1;36m--INTERMEDIATE\033[0m\tOutput the intermediate (intermediate.txt) file or no." << endl;
    cout << "  \033[1;36m--PASS1\033[0m\tOutput the instructions after calculating the location counter (pass1.txt)" << endl;
    cout << "  \033[1;36m--SYMBOLS\033[0m\tOutput the symbol table (symbTable.txt)" << endl;
    cout << "  \033[1;36m--PASS2\033[0m\tOutput the instructions after resolving the blocks and calculating the object code (pass2.txt)" << endl;
    cout << "  \033[1;36m--HTME\033[0m\tOutput the HTME file (HTME.txt)" << endl;
    cout << "  \033[1;36m--ALL / -A\033[0m\tOutput all files" << endl;
    cout << "  \033[1;36m--HELP / -H\033[0m\tDisplay this help message (Use without providing a file path)" << endl;
}

int main(int argc, char *argv[])
{
    int x;

    bool v = false, pass1 = false, pass2 = false, symbol = false, all = false, inter = false, htme = false;
    string path;
    list<string> lines;
    list<sicx::Instruction> insts;
    list<Symbol> symbolslist;
    BlockMap blockmap;
    list<string> htmelist;

    if (argc == 2)
    {
        if (toUpperCase(argv[1]) == "-H" || toUpperCase(argv[1]) == "--HELP")
        {
            displayHelp();
            return 0;
        }
        else
        {
            path = argv[1];
            all = true;
        }
    }
    if (argc > 1)
    {
        path = argv[1];
        for (int i = 2; i < argc; i++)
        {
            if (toUpperCase(argv[i]) == "-V")
                v = true;
            else if (toUpperCase(argv[i]) == "--PASS1")
                pass1 = true;
            else if (toUpperCase(argv[i]) == "--PASS2")
                pass2 = true;
            else if (toUpperCase(argv[i]) == "--SYMBOLS")
                symbol = true;
            else if (toUpperCase(argv[i]) == "--INTERMEDIATE")
                inter = true;
            else if (toUpperCase(argv[i]) == "--HTME")
                htme = true;
            else if (toUpperCase(argv[i]) == "-A" || toUpperCase(argv[i]) == "--ALL")
                all = true;
            else
            {
                cout << "\33[31mUnexpected flag: \33[0m" << argv[i];
                return -1;
            }
        }
    }
    else
    {
        path = "in.txt";
        all = true;
    }
    if (all)
    {
        pass1 = true;
        pass2 = true;
        symbol = true;
        inter = true;
        htme = true;
    }

    if (access(path.c_str(), 0) != 0)
    {
        cout << "\33[31mCouldn't find file: \33[0m" << path;
        return -1;
    }

    if (pass1 || pass2 || symbol || inter || htme)
    {
        ReadLines("", "in.txt", lines, v);
        for (string &line : lines)
        {
            preProcess(line, v);
        }

        for (string &line : lines)
        {
            sicx::Instruction inst;
            int r = inst.getInstObj(line);
            if (r == 0)
                insts.push_back(inst);
            else
            {
                cout << "\33[31mError parsing line: " << line << endl;
                cout << "\33[31mTerminating.";
                return -1;
            }
        }
        lines.clear();
        if (inter)
        {
            for (sicx::Instruction inst : insts)
            {
                lines.push_back(inst.toString());
            }
            WriteLines("", "intermediate.txt", lines, v);
            lines.clear();
        }
    }
    if (pass1 || pass2 || symbol || htme)
    {
        if (LOCCTRCalc(insts, symbolslist, blockmap, v) != 0)
        {
            cout << "\33[31mError Processing the Location Counter; terminating.";
            return -1;
        }
        if (symbol)
        {
            symbolsToString(lines, symbolslist);
            WriteLines("", "symbTable.txt", lines, v);
            lines.clear();
        }
        if (pass1)
        {
            for (sicx::Instruction inst : insts)
            {

                stringstream ss;

                // Set the formatting for 4-digit hexadecimal with leading zeros
                ss << setw(4) << setfill('0') << hex << uppercase << inst.location - blockmap[inst.block].addr;

                string loc = ss.str();

                string str;
                str = loc + "\t\t\t" + inst.toString();
                lines.push_back(str);
            }
            WriteLines("", "pass1.txt", lines, v);
            lines.clear();
        }
    }
    if (pass2 || htme)
    {
        objCodeCalc(insts, symbolslist, v);
        // for (sicx::Instruction inst : insts)
        // {
        //     cout << "Location: " << hex << inst.location << ", Object Code: " << inst.objectCode << endl;
        // }
        lines.clear();
        if (pass2)
        {
            for (sicx::Instruction inst : insts)
            {
                stringstream ss;
                ss << setw(4) << setfill('0') << hex << uppercase << inst.location << "\t\t" << inst.toString() << "\t\t\t" << uppercase << inst.objectCode;
                lines.push_back(ss.str());
            }
            WriteLines("", "pass2.txt", lines, v);
            lines.clear();
        }
    }
    if (htme)
    {
        list<string> htme;
        generateHTMERecords(insts, lines, v);
        WriteLines("", "HTME.txt", lines, v);

        // cout << calculateProgramLength(insts);
    }

    return 0;
}