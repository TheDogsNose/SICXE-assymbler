
#ifndef _frmttt
#define _frmttt

#include <Array>
#include <string>
#include <unordered_map>
#include <variant>
#include <sstream>
#include <algorithm>
#include "formats.h"

using namespace std;
namespace sicx
{
    string toUpperCase(const string &str)
    {
        string upperStr = str;
        transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
        return upperStr;
    }
    struct Directive
    {
        string dir;
        string T;
    };
    struct Data
    {
        string type;
        string value;
    };

    struct Format1
    {
        string instruction;
    };
    struct Format2
    {
        string instruction;
        string register1;
        string register2;
    };
    struct Format34
    {
        char nixbpe; // 00-000000
        string instruction;
        string memory;
    };
    struct Format4f
    {
        string instruction;
        string register1;
        string memory;
        string condition;
    };

    typedef variant<Format1, Format2, Format34, Format4f, Data, Directive> Inst;

    class Instruction
    {
    private:
    public:
        string label;
        Inst inst;

        unsigned int location = 0;
        unsigned int objectCode = 0;
        BLOCK block = DEFAULT;

        Instruction(/* args */) {}
        ~Instruction() {}

        string toString()
        {
            // find type of instruction and concat variables into a single string
            if (holds_alternative<Format1>(inst))
            {
                return (label.empty() ? "\t" : label) + "\t" + get<Format1>(inst).instruction;
            }
            else if (holds_alternative<Format2>(inst))
            {
                return (label.empty() ? "\t" : label) + "\t" + get<Format2>(inst).instruction + "\t\t" + get<Format2>(inst).register1 + (get<Format2>(inst).register2.empty() ? "" : "," + get<Format2>(inst).register2);
            }
            else if (holds_alternative<Format34>(inst))
            {
                // find if '+', '@' or '#' should be added
                bool ind = false, imm = false, regx = false, ext = false;
                if (get<Format34>(inst).nixbpe & 0b110000 || (~(get<Format34>(inst).nixbpe) & 0b110000))
                {
                    ind = false;
                    imm = false;
                }
                else if (get<Format34>(inst).nixbpe & 0b100000)
                {
                    ind = true;
                }
                else if (get<Format34>(inst).nixbpe & 0b010000)
                {
                    imm = true;
                }

                if (get<Format34>(inst).nixbpe & 0b001000)
                {
                    regx = true;
                }

                if (get<Format34>(inst).nixbpe & 0b000001)
                {
                    ext = true;
                }
                return (label.empty() ? "\t" : label) + "\t" + (ext ? "+" : "") + get<Format34>(inst).instruction + "\t\t" + (ind ? "@" : imm ? "#"
                                                                                                                                              : "") +
                       get<Format34>(inst).memory + (regx ? ",X" : "");
            }
            else if (holds_alternative<Format4f>(inst))
            {
                return (label.empty() ? "\t" : label) + "\t" + get<Format4f>(inst).instruction + "\t" + get<Format4f>(inst).register1 + ", " + get<Format4f>(inst).memory + ", " + get<Format4f>(inst).condition;
            }
            else if (holds_alternative<Data>(inst))
            {
                return (label.empty() ? "\t" : label) + "\t" + get<Data>(inst).type + "\t\t" + get<Data>(inst).value;
            }
            else if (holds_alternative<Directive>(inst))
            {
                return (label.empty() ? "\t" : label) + "\t" + get<Directive>(inst).dir + (get<Directive>(inst).T.empty() ? "" : "\t\t" + get<Directive>(inst).T);
            }
            else
                return label;
        }

        int getInstObj(string line)
        {
            for (char &ch : line)
            {
                if (ch == ',')
                {
                    ch = ' '; // Replace comma with space
                }
            }

            istringstream lineStream(line);
            string token1, token2, token3, token4, token5;

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Tokenize the line (e.g., LABEL MNEMONIC OPERAND1 OPERAND2 OPERAND3)
            lineStream >> token1;
            token1 = toUpperCase(token1);
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // Handle format 4 plus sign
            bool ext;
            if (token1[0] == '+')
            {
                token1 = token1.substr(1);
                ext = true;
            }
            else
            {
                token1 = token1;
                ext = false;
            }

            auto it = opcodeTable.find(token1);
            if (it != opcodeTable.end())
            {
                const auto &opcodeDetails = it->second;

                // Create an appropriate Instruction object based on format
                switch (opcodeDetails.format)
                {
                case 1:
                {
                    Format1 x;
                    label = "";
                    x = {token1};
                    inst = x;
                    break;
                }
                case 2:
                {
                    Format2 x;
                    label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    lineStream >> token3;
                    token3 = toUpperCase(token3);
                    x = {token1, token2, token3};
                    inst = x;
                    break;
                }
                case 3:
                {
                    Format34 x;
                    label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);

                    bool regx = false, ind = false, imm = false;
                    if (token2[0] == '#')
                    {
                        imm = true;
                    }
                    else if (token2[0] == '@')
                    {
                        ind = true;
                    }

                    if (lineStream >> token3)
                    {
                        if (toUpperCase(token3) == "X")
                        {
                            regx = true;
                        }
                        else
                        {
                            return -1;
                        }
                    }

                    x = {(char)(0 | (ext) | (ind << 5) | (imm << 4) | (regx << 3)), token1, token2};
                    inst = x;
                    break;
                }
                case 4:
                {
                    Format4f x;
                    label = "";
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    lineStream >> token3;
                    token3 = toUpperCase(token3);
                    lineStream >> token4;
                    token4 = toUpperCase(token4);
                    x = {token1, token2, token3, token4};
                    inst = x;
                    break;
                }
                default:
                {
                    return -1;
                }
                }
            }
            else
            {
                auto it = directiveTable.find(token1);
                if (it != directiveTable.end())
                {
                    const auto &opcodeDetails = it->second;
                    switch (opcodeDetails)
                    {
                    case 0:
                    {
                        Directive x;
                        label = "";
                        x = {token1, ""};
                        inst = x;
                        break;
                    }
                    case 1:
                    {
                        Directive x;
                        label = "";
                        lineStream >> token2;
                        token2.erase(std::remove_if(token2.begin(), token2.end(), ::isspace), token2.end());
                        x = {token1, token2};
                        inst = x;
                        break;
                    } /*
                     case 2:
                     {
                         Data x;
                         label = "";
                         lineStream >> token2;
                         x = {token1, token2};
                         inst = x;
                         break;
                     }*/
                    default:
                    {
                        return -1;
                    }
                    }
                }
                else
                {
                    // Handle format 4 plus sign
                    lineStream >> token2;
                    token2 = toUpperCase(token2);
                    if (token2[0] == '+')
                    {
                        token2 = token2.substr(1);
                        ext = true;
                    }
                    else
                    {
                        token2 = token2;
                        ext = false;
                    }
                    auto it = opcodeTable.find(token2);
                    if (it != opcodeTable.end())
                    {
                        const auto &opcodeDetails = it->second;

                        // Create an appropriate Instruction object based on format
                        switch (opcodeDetails.format)
                        {
                        case 1:
                        {
                            Format1 x;
                            label = token1;
                            x = {token2};
                            inst = x;
                            break;
                        }
                        case 2:
                        {
                            Format2 x;
                            label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);
                            lineStream >> token4;
                            token4 = toUpperCase(token4);
                            x = {token2, token3, token4};
                            inst = x;
                            break;
                        }
                        case 3:
                        {
                            Format34 x;
                            label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);

                            bool regx = false, ind = false, imm = false;
                            if (token3[0] == '#')
                            {
                                imm = true;
                            }
                            else if (token3[0] == '@')
                            {
                                ind = true;
                            }

                            if (lineStream >> token4)
                            {
                                if (toUpperCase(token4) == "X")
                                {
                                    regx = true;
                                }
                                else
                                {
                                    return -1;
                                }
                            }

                            x = {(char)(0 | (ext) | (ind << 5) | (imm << 4) | (regx << 3)), token2, token3};
                            inst = x;
                            break;
                        }
                        case 4:
                        {
                            Format4f x;
                            label = token1;
                            lineStream >> token3;
                            token3 = toUpperCase(token3);
                            lineStream >> token4;
                            token4 = toUpperCase(token4);
                            lineStream >> token5;
                            token5 = toUpperCase(token5);
                            x = {token2, token3, token4, token5};
                            inst = x;
                            break;
                        }
                        default:
                        {
                            return -1;
                        }
                        }
                    }
                    else
                    {
                        auto it = directiveTable.find(token2);
                        if (it != directiveTable.end())
                        {
                            const auto &opcodeDetails = it->second;
                            switch (opcodeDetails)
                            {
                            case 3:
                            {
                                Directive x;
                                label = token1;
                                lineStream >> token3;
                                x = {token2, token3};
                                inst = x;
                                break;
                            }
                            case 2:
                            {
                                Data x;
                                label = token1;
                                lineStream >> token3;
                                x = {token2, token3};
                                inst = x;
                                break;
                            }
                            default:
                            {
                                return -1;
                            }
                            }
                        }
                        else
                        {
                            return -1;
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
            return 0;
        }
    };

}
#endif