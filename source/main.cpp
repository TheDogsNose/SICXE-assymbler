#include <iostream>
#include <fstream>
#include <string>
#include "Assembler.cpp"
#include <list>
#include <variant>
#include "formats.cpp"
#include <tuple>
#include <iomanip>

using namespace std;

void displayMenu() {
    cout << "\n\033[1;34m===== SIC/XE Assembler =====\033[0m" << endl;
    cout << "\033[32m1. Preprocess Input File\033[0m" << endl;
    cout << "\033[33m2. Generate Intermediate File\033[0m" << endl;
    cout << "\033[34m3. Perform Pass 1 (Symbol Table and Locations)\033[0m" << endl;
    cout << "\033[36m4. Perform Pass 2 (Generate Object Code)\033[0m" << endl;
    cout << "\033[35m5. Generate HTME Records\033[0m" << endl;
    cout << "\033[37m6. View Output Files\033[0m" << endl;
    cout << "\033[31m7. Exit\033[0m" << endl;
    cout << "\033[1;34m============================\033[0m" << endl;
    cout << "\033[1;33mEnter your choice: \033[0m";
}

void viewFiles(const string &fileName) {
    ifstream file(fileName);
    if (file.is_open()) {
        string line;
        cout << "\n\033[1;36mContents of " << fileName << ":\033[0m\n";
        cout << "\033[1;34m--------------------------------\033[0m" << endl;
        while (getline(file, line)) {
            cout << line << endl;
        }
        cout << "\033[1;34m--------------------------------\033[0m" << endl;
        file.close();
    } else {
        cout << "\033[31mUnable to open file: " << fileName << "\033[0m" << endl;
    }
}

int main(int argc, char *argv[]) {
    list<string> lines;
    list<sicx::Instruction> insts;
    list<Symbol> symbols;
    BlockMap blockmap;
    list<string> htme;

    bool running = true;

    while (running) {
        displayMenu();
        int choice;
        cin >> choice;

        switch (choice) {
            case 1: {
                ReadLines("", "in.txt", lines);
                for (string &line : lines) {
                    preProcess(line);
                }
                cout << "\033[32mPreprocessing completed!\033[0m" << endl;
                break;
            }

            case 2: {
                lines.clear();
                ReadLines("", "in.txt", lines);
                for (string &line : lines) {
                    sicx::Instruction inst;
                    int r = inst.getInstObj(line);
                    if (r == 0)
                        insts.push_back(inst);
                    else
                        cout << "\033[33mError parsing line: \033[0m" << line << endl;
                }
                lines.clear();
                for (sicx::Instruction inst : insts) {
                    lines.push_back(inst.toString());
                }
                WriteLines("", "intermediate.txt", lines);
                cout << "\033[32mIntermediate file generated: intermediate.txt\033[0m" << endl;
                break;
            }

            case 3: {
                cout << "\033[34mPerforming Pass 1...\033[0m" << endl;
                if (LOCCTRCalc(insts, symbols, blockmap) != 0)
                {
                    cout << "\033[31mError during Pass 1; terminating.\033[0m" << endl;
                    break;
                }

                // Write symbol table to symbTable.txt
                list<string> symbolTXT;
                symbolsToString(symbolTXT, symbols);
                WriteLines("", "symbTable.txt", symbolTXT);

                // Write Pass 1 details to pass1.txt
                list<string> pass1Lines;
                for (const sicx::Instruction &inst : insts)
                {
                    stringstream ss;

                    // Set the formatting for 4-digit hexadecimal with leading zeros
                    ss << setw(4) << setfill('0') << hex << uppercase << inst.location - blockmap[inst.block].addr;

                    string loc = ss.str();

                    // Use const_cast to call non-const toString method
                    string instDetails = loc + "\t\t\t" + const_cast<sicx::Instruction &>(inst).toString();

                    pass1Lines.push_back(instDetails);
                }

                WriteLines("", "pass1.txt", pass1Lines);
                cout << "\033[32mPass 1 completed successfully. Symbol table and pass1.txt generated.\033[0m" << endl;
                break;
            }

            case 4: {
                objCodeCalc(insts, symbols);
                list<string> lines2;
                for (sicx::Instruction inst : insts) {
                    stringstream ss;
                    ss << setw(4) << setfill('0') << hex << uppercase << inst.location << "\t\t" 
                       << inst.toString() << "\t\t\t" << uppercase << inst.objectCode;
                    lines2.push_back(ss.str());
                }
                WriteLines("", "pass2.txt", lines2);
                cout << "\033[32mPass 2 completed: Object code generated in pass2.txt\033[0m" << endl;
                break;
            }

            case 5: {
                generateHTMERecords(insts, htme);
                WriteLines("", "HTME.txt", htme);
                cout << "\033[32mHTME records generated: HTME.txt\033[0m" << endl;
                break;
            }

            case 6: {
                cout << "\n\033[36mSelect a file to view:\033[0m" << endl;
                cout << "\033[32m1. Intermediate File (intermediate.txt)\033[0m" << endl;
                cout << "\033[33m2. Symbol Table (symbTable.txt)\033[0m" << endl;
                cout << "\033[34m3. Pass 1 Output (pass1.txt)\033[0m" << endl;
                cout << "\033[36m4. Pass 2 Output (pass2.txt)\033[0m" << endl;
                cout << "\033[35m5. HTME Records (HTME.txt)\033[0m" << endl;
                cout << "\033[1;33mEnter your choice: \033[0m";
                int fileChoice;
                cin >> fileChoice;
                switch (fileChoice) {
                    case 1: viewFiles("intermediate.txt"); break;
                    case 2: viewFiles("symbTable.txt"); break;
                    case 3: viewFiles("pass1.txt"); break;
                    case 4: viewFiles("pass2.txt"); break;
                    case 5: viewFiles("HTME.txt"); break;
                    default: cout << "\033[31mInvalid choice!\033[0m" << endl; break;
                }
                break;
            }

            case 7: {
                running = false;
                cout << "\033[1;31mExiting the assembler. Goodbye!\033[0m" << endl;
                break;
            }

            default:
                cout << "\033[31mInvalid choice! Please try again.\033[0m" << endl;
        }
    }

    return 0;
}
