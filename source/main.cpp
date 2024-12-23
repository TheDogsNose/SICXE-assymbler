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
    cout << "\n===== SIC/XE Assembler =====" << endl;
    cout << "1. Preprocess Input File" << endl;
    cout << "2. Generate Intermediate File" << endl;
    cout << "3. Perform Pass 1 (Symbol Table and Locations)" << endl;
    cout << "4. Perform Pass 2 (Generate Object Code)" << endl;
    cout << "5. Generate HTME Records" << endl;
    cout << "6. View Output Files" << endl;
    cout << "7. Exit" << endl;
    cout << "============================" << endl;
    cout << "Enter your choice: ";
}

void viewFiles(const string &fileName) {
    ifstream file(fileName);
    if (file.is_open()) {
        string line;
        cout << "\nContents of " << fileName << ":\n";
        cout << "--------------------------------" << endl;
        while (getline(file, line)) {
            cout << line << endl;
        }
        cout << "--------------------------------" << endl;
        file.close();
    } else {
        cout << "Unable to open file: " << fileName << endl;
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
                cout << "Preprocessing completed!" << endl;
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
                        cout << "Error parsing line: " << line << endl;
                }
                lines.clear();
                for (sicx::Instruction inst : insts) {
                    lines.push_back(inst.toString());
                }
                WriteLines("", "intermediate.txt", lines);
                cout << "Intermediate file generated: intermediate.txt" << endl;
                break;
            }

            case 3: {
                cout << "\nPerforming Pass 1...\n";
                if (LOCCTRCalc(insts, symbols, blockmap) != 0)
                {
                    cout << "Error during Pass 1; terminating.\n";
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
                cout << "Pass 1 completed successfully. Symbol table and pass1.txt generated.\n";
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
                cout << "Pass 2 completed: Object code generated in pass2.txt" << endl;
                break;
            }

            case 5: {
                generateHTMERecords(insts, htme);
                WriteLines("", "HTME.txt", htme);
                cout << "HTME records generated: HTME.txt" << endl;
                break;
            }

            case 6: {
                cout << "\nSelect a file to view:" << endl;
                cout << "1. Intermediate File (intermediate.txt)" << endl;
                cout << "2. Symbol Table (symbTable.txt)" << endl;
                cout << "3. Pass 1 Output (pass1.txt)" << endl;
                cout << "4. Pass 2 Output (pass2.txt)" << endl;
                cout << "5. HTME Records (HTME.txt)" << endl;
                cout << "Enter your choice: ";
                int fileChoice;
                cin >> fileChoice;
                switch (fileChoice) {
                    case 1: viewFiles("intermediate.txt"); break;
                    case 2: viewFiles("symbTable.txt"); break;
                    case 3: viewFiles("pass1.txt"); break;
                    case 4: viewFiles("pass2.txt"); break;
                    case 5: viewFiles("HTME.txt"); break;
                    default: cout << "Invalid choice!" << endl; break;
                }
                break;
            }

            case 7: {
                running = false;
                cout << "Exiting the assembler. Goodbye!" << endl;
                break;
            }

            default:
                cout << "Invalid choice! Please try again." << endl;
        }
    }

    return 0;
}
