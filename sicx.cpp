#include <iostream>
#include <fstream>
#include <string>
#include "FileProcessor.cpp"
#include <list>
#include <variant>
#include "Sicx.h"

using namespace std;

main(int argc, char *argv[])
{
  // 1. Parse command line arguments
  // 2. Initialize the SICX library
  // 3. Start the SICX server
  // 4. Wait for incoming connections
  // 5. Handle incoming connections
  // 6. Clean up and exit

  // std::list<std::variant<sicx::Instruction<sicx::Format1>, sicx::Instruction<sicx::Format2>, sicx::Instruction<sicx::Format34>, sicx::Instruction<sicx::Format4f>, sicx::Instruction<sicx::Data>, sicx::Instruction<sicx::Directive>>> lines;

  list<string> lines;
  ReadLines("", "in.txt", lines);
  for (string &line : lines)
  {
    preProcess(line);
  }
  WriteLines("", "intermediate.txt", lines);

  // readFromFile("in.txt", lines);
  /*auto it = lines.begin();

   for (int i = 0; i < lines.size(); i++)
   {
     if (holds_alternative<sicx::Instruction<sicx::Format1>>(*it)){

       std::cout << "Format1: " << (get<sicx::Instruction<sicx::Format1>>(*it)).label << std::endl;

     }else if (holds_alternative<sicx::Instruction<sicx::Format2>>(*it)){
       std::cout << "Format2: " << (get<sicx::Instruction<sicx::Format2>>(*it)).label << std::endl;

     }else if (holds_alternative<sicx::Instruction<sicx::Format34>>(*it)){

       std::cout << "Format3/4: " << (get<sicx::Instruction<sicx::Format34>>(*it)).label << std::endl;

     }else if (holds_alternative<sicx::Instruction<sicx::Format4f>>(*it)){

       std::cout << "Format4f: " << (get<sicx::Instruction<sicx::Format4f>>(*it)).label << std::endl;

     }else if (holds_alternative<sicx::Instruction<sicx::Data>>(*it)){

       std::cout << "FormatD: " <<( get<sicx::Instruction<sicx::Data>>(*it)).label << std::endl;

     }else if (holds_alternative<sicx::Instruction<sicx::Directive>>(*it)){

       std::cout << "Format#: " << (get<sicx::Instruction<sicx::Directive>>(*it)).label << std::endl;

     }else{

       std::cout << "Error" << std::endl;
   }

     std::advance(it, 1);
   }*/

  cin.get();
}