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

main(int argc, char *argv[])
{

  list<string> lines;
  list<sicx::Instruction> insts;

  ReadLines("", "in.txt", lines);
  for (string &line : lines)
  {
    preProcess(line);
  }

  // lines.clear();
  // ReadLines("", "intermediate.txt", lines);

  for (string &line : lines)
  {
    sicx::Instruction inst;
    int r = inst.getInstObj(line);
    if (r == 0)
      insts.push_back(inst);
    else
      cout << "Error parsing line: " << line << endl;
  }
  lines.clear();

  for (sicx::Instruction inst : insts)
  {
    lines.push_back(inst.toString());
  }
  WriteLines("", "intermediate.txt", lines);

  list<Symbol> symbols;
  BlockMap blockmap;
  if (LOCCTRCalc(insts, symbols, blockmap) != 0)
  {
    cout << "Error; terminating.";
    return -1;
  }

  list<string> symbolTXT;
  symbolsToString(symbolTXT, symbols);
  WriteLines("", "symbTable.txt", symbolTXT);

  list<string> lines2;
  for (sicx::Instruction inst : insts)
  {

    stringstream ss;

    // Set the formatting for 4-digit hexadecimal with leading zeros
    ss << setw(4) << setfill('0') << hex << uppercase << inst.location - blockmap[inst.block].addr;

    string loc = ss.str();

    string str;
    str = loc + "\t\t\t" + inst.toString();
    lines2.push_back(str);
  }
  WriteLines("", "pass1.txt", lines2);

  objCodeCalc(insts, symbols);
  for (sicx::Instruction inst : insts)
  {
    cout << "Location: " << hex << inst.location << ", Object Code: " << inst.objectCode << endl;
  }
  lines2.clear();

  for (sicx::Instruction inst : insts)
  {
    stringstream ss;
    ss << setw(4) << setfill('0') << hex << uppercase << inst.location << "\t\t" << inst.toString() << "\t\t\t" << uppercase << inst.objectCode;
    lines2.push_back(ss.str());
  }
  WriteLines("", "pass2.txt", lines2);

  list<string> htme;
  generateHTMERecords(insts, htme);
  WriteLines("", "HTME.txt", htme);

  cout << calculateProgramLength(insts);
  cin.get();
}