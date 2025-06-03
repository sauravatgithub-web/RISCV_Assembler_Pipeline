#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <vector>
#include <unordered_map>
using namespace std;

#define REG "REGISTER"
#define IMM "IMMEDIATE"
#define LOD "LOAD"
#define SRE "STORE"
#define BCH "BRANCH"
#define JMP "JUMP"
#define UIMM "UPPER_IMMEDIATE"

unordered_map<string, string> funcToAddrMode = {
    { "ADD",  REG }, { "SUB",  REG }, { "XOR",  REG }, { "OR",   REG }, { "AND",  REG }, { "SLL",  REG  }, { "SRL",   REG }, { "SRA",  REG }, { "SLT",   REG }, { "SLTU", REG },
    { "ADDI", IMM }, { "XORI", IMM }, { "ORI",  IMM }, { "ANDI", IMM }, { "SLLI", IMM }, { "SRLI", IMM  }, { "SRAI",  IMM }, { "SLTI", IMM }, { "SLTIU", IMM }, { "JALR", IMM },
    { "LB",   LOD }, { "LH",   LOD }, { "LW",   LOD }, { "LBU",  LOD }, { "LHU",  LOD }, { "SB",   SRE  }, { "SH",    SRE }, { "SW",   SRE }, { "BEQ",   BCH }, { "BNE", BCH  },
    { "BLT",  BCH }, { "BGE",  BCH }, { "BLTU", BCH }, { "BGEU", BCH }, { "JAL",  JMP }, { "LUI",  UIMM }, { "AUIPC", JMP }
};

unordered_map<string, string> registerToBinary = {
    { "x0",   "00000" }, { "x1",  "00001" }, { "x2",  "00010" }, { "x3",  "00011" }, { "x4",  "00100" }, { "x5",  "00101" }, { "x6",  "00110" }, { "x7",  "00111" },
    { "x8",   "01000" }, { "x9",  "01001" }, { "x10", "01010" }, { "x11", "01011" }, { "x12", "01100" }, { "x13", "01101" }, { "x14", "01110" }, { "x15", "01111" },
    { "x16",  "10000" }, { "x17", "10001" }, { "x18", "10010" }, { "x19", "10011" }, { "x20", "10100" }, { "x21", "10101" }, { "x22", "10110" }, { "x23", "10111" },
    { "x24",  "11000" }, { "x25", "11001" }, { "x26", "11010" }, { "x27", "11011" }, { "x28", "11100" }, { "x29", "11101" }, { "x30", "11110" }, { "x31", "11111" },
    { "zero", "00000" }, { "ra",  "00001" }, { "sp",  "00010" }, { "gp",  "00011" }, { "tp",  "00100" }, { "t0",  "00101" }, { "t1",  "00110" }, { "t2",  "00111" },
    { "s0",   "01000" }, { "s1",  "01001" }, { "a0",  "01010" }, { "a1",  "01011" }, { "a2",  "01100" }, { "a3",  "01101" }, { "a4",  "01110" }, { "a5",  "01111" },
    { "a6",   "10000" }, { "a7",  "10001" }, { "s2",  "10010" }, { "s3",  "10011" }, { "s4",  "10100" }, { "s5",  "10101" }, { "s6",  "10110" }, { "s7",  "10111" },
    { "s8",   "11000" }, { "s9",  "11001" }, { "s10", "11010" }, { "s11", "11011" }, { "t3",  "11100" }, { "t4",  "11101" }, { "t5",  "11110" }, { "t6",  "11111" },
    { "fp",   "01000" }
};

unordered_map<char, string> hexToBin = {
    { '0', "0000" }, { '1', "0001" }, { '2', "0010" }, { '3', "0011" }, { '4', "0100" }, { '5', "0101" }, { '6', "0110" }, { '7', "0111" },
    { '8', "1000" }, { '9', "1001" }, { 'a', "1010" }, { 'b', "1011" }, { 'c', "1100" }, { 'd', "1101" }, { 'e', "1110" }, { 'f', "1111" }
};

unordered_map<string, string> labels = {};

string expandBits(string imm, int N, bool neg = false) {
    if(N < imm.size()) {
        imm = imm.substr(imm.size() - N);
        return imm;
    }

    string newImm = "";
    int n = N - imm.size();
    for(int i = 0; i < n; i++) {
        if(!neg) newImm.push_back('0');
        else newImm.push_back(imm[0] == '0' ? '0' : '1');
    }
    newImm += imm;
    return newImm;
}

string numberToBinary(const string& numStr) {
    if(numStr[1] == 'b') return numStr.substr(2);
    if(numStr[1] == 'x') {
        string binary = "";
        for(int i = 2; i < numStr.size(); i++) {
            binary += hexToBin[numStr[i]];
        }
        return binary;
    }
    if(numStr[1] == 'o') {
        string binary = "";
        for(int i = 2; i < numStr.size(); i++) {
            binary += hexToBin[numStr[i]].substr(1);
        }
        return binary;
    }

    int number = stoi(numStr);
    if(number == 0) return "000000000000";
    if(number < 0) number = (((1 << 12) - 1) ^ abs(number)) + 1;
    
    string binary = "";
    while(number > 0) {
        binary = (number % 2 == 0 ? "0" : "1") + binary;
        number /= 2;
    }
    binary = expandBits(binary, 12);

    return binary;
}

class components {
protected:
    string rd, rs1, rs2, imm, shamt, offset;
public: 
    void getData(string type, vector<string>& instruction, int lineCount, int extra = 0) {
        if(type == REG) {
            rd = registerToBinary[instruction[1]];
            rs1 = registerToBinary[instruction[2]];
            rs2 = registerToBinary[instruction[3]];
        }
        else if(type == IMM) {
            rd = registerToBinary[instruction[1]];
            rs1 = registerToBinary[instruction[2]];
            imm = (extra == 0) ? numberToBinary(instruction[3]) : "";
            shamt = (extra == 1) ? expandBits(numberToBinary(instruction[3]), 5) : "";
        }
        else if(type == LOD) {
            rd = registerToBinary[instruction[1]];
            rs1 = registerToBinary[instruction[2]];
            offset = numberToBinary(instruction[3]);
        }
        else if(type == SRE) {
            rs1 = registerToBinary[instruction[2]];
            rs2 = registerToBinary[instruction[1]];
            string comboData = numberToBinary(instruction[3]);
            imm = comboData.substr(7, 5);
            offset = comboData.substr(0, 7);
        }
        else if(type == BCH) {
            rs1 = registerToBinary[instruction[1]];
            rs2 = registerToBinary[instruction[2]];
            if(!isdigit(instruction[3][0]) && instruction[3][0] != '-') {
                if(labels.find(instruction[3]) != labels.end()) instruction[3] = to_string(stoi(labels[instruction[3]]) - 4*lineCount);
                else {
                    cout << "Couldn't find " << instruction[3] << " !!" << endl;
                    exit(0);
                }
            }
            string comboData = numberToBinary(instruction[3]);
            comboData.pop_back();
            comboData = ((instruction[3][0] != '-') ? "0" : "1") + comboData;
            imm = comboData.substr(8, 4);
            imm.push_back(comboData[1]);
            offset = comboData.substr(0, 8);
            offset.erase(1, 1);
        }
        else if(type == JMP) {
            rd = registerToBinary[instruction[1]];
            if(!isdigit(instruction[2][0]) && instruction[2][0] != '-') {
                if(labels.find(instruction[2]) != labels.end()) instruction[2] = to_string(stoi(labels[instruction[2]]) - 4*lineCount);
                else {
                    cout << "Couldn't find " << instruction[2] << " !!" << endl;
                    exit(0);
                }
            }
            string primary = expandBits(numberToBinary(instruction[2]), 20, true);
            primary = primary.substr(0, 19);
            primary = ((instruction[2][0] != '-') ? "0" : "1") + primary;
            offset = "";
            offset.push_back(primary[0]);
            offset += primary.substr(10, 10);
            offset.push_back(primary[9]);
            offset += primary.substr(1, 8);
        }
        else if(type == UIMM) {
            rd = registerToBinary[instruction[1]];
            offset = expandBits(numberToBinary(instruction[2]), 20, true);
        }
    }
};

class REGISTER : public components {
private: 
    string opCode = "0110011", func3, func7;
    unordered_map<string, vector<string>> classData = {
        { "ADD",   { "000", "0000000" } },
        { "SUB",   { "000", "0100000" } },
        { "XOR",   { "100", "0000000" } },
        { "OR",    { "110", "0000000" } },
        { "AND",   { "111", "0000000" } },
        { "SLL",   { "001", "0000000" } },
        { "SRL",   { "101", "0000000" } },
        { "SRA",   { "101", "0100000" } },
        { "SLT",   { "010", "0000000" } },
        { "SLTU",  { "011", "0000000" } },
    };
public: 
    REGISTER() {}
    REGISTER(vector<string>& instruction, int lineCount) {
        getData(REG, instruction, lineCount);
        func3 = classData[instruction[0]][0];
        func7 = classData[instruction[0]][1];
    }
    
    string getEncoding() {
        return (func7 + rs2 + rs1 + func3 + rd + opCode);
    }
};

class IMMEDIATE : public components {
private: 
    string opCode = "0010011", func3;
    unordered_map<string, vector<string>> classData = {
        { "ADDI",   { "000" } },
        { "XORI",   { "100" } },
        { "ORI",    { "110" } },
        { "ANDI",   { "111" } },
        { "SLLI",   { "001", "0000000" } },
        { "SRLI",   { "101", "0000000" } },
        { "SRAI",   { "101", "0100000" } },
        { "SLTI",   { "010" } },
        { "SLTIU",  { "011" } },
        { "JALR",   { "000" } }
    };
public: 
    IMMEDIATE() {}
    IMMEDIATE(vector<string>& instruction, int lineCount) {
        if(instruction[0] == "SLLI" || instruction[0] == "SRLI" || instruction[0] == "SRAI") {
            getData(IMM, instruction, lineCount, 1);
            imm = classData[instruction[0]][1];
        }
        else getData("IMMEDIATE", instruction, lineCount);
        if(instruction[0] == "JALR") opCode = "1100111";
        func3 = classData[instruction[0]][0];
    }
    
    string getEncoding() {
        return (imm + shamt + rs1 + func3 + rd + opCode);
    }
};

class LOAD : public components {
private: 
    string opCode = "0000011", func3;
    unordered_map<string, string> classData = {
        { "LB",  "000" },
        { "LH",  "001" },
        { "LW",  "010" },
        { "LBU", "100" },
        { "LHU", "101" }
    };
public: 
    LOAD() {}
    LOAD(vector<string>& instruction, int lineCount) {
        getData(LOD, instruction, lineCount);
        func3 = classData[instruction[0]];
    }
    
    string getEncoding() {
        return (offset + rs1 + func3 + rd + opCode);
    }
};

class STORE : public components {
private: 
    string opCode = "0100011", func3;
    unordered_map<string, string> classData = {
        { "SB",  "000" },
        { "SH",  "001" },
        { "SW",  "010" },
    };
public: 
    STORE() {}
    STORE(vector<string>& instruction, int lineCount) {
        getData(SRE, instruction, lineCount);
        func3 = classData[instruction[0]];
    }
    
    string getEncoding() {
        return (offset + rs2 + rs1 + func3 + imm + opCode);
    }
};

class BRANCH : public components {
private: 
    string opCode = "1100011", func3;
    unordered_map<string, string> classData = {
        { "BEQ",  "000" },
        { "BNE",  "001" },
        { "BLT",  "100" },
        { "BGE",  "101" },
        { "BLTU", "110" },
        { "BGEU", "111" },
    };
public: 
    BRANCH() {}
    BRANCH(vector<string>& instruction, int lineCount) {
        getData(BCH, instruction, lineCount);
        func3 = classData[instruction[0]];
    }
    
    string getEncoding() {
        return (offset + rs2 + rs1 + func3 + imm + opCode);
    }
};

class JUMP : public components {
private: 
    string opCode = "1101111";
public: 
    JUMP() {}
    JUMP(vector<string>& instruction, int lineCount) {
        getData(JMP, instruction, lineCount);
    }
    
    string getEncoding() {
        return (offset + rd + opCode);
    }
};

class UPPER_IMMEDIATE : public components {
private: 
    string opCode1 = "0110111", opCode2 = "0010111", opCode;
public: 
    UPPER_IMMEDIATE() {}
    UPPER_IMMEDIATE(vector<string>& instruction, int lineCount) {
        getData(UIMM, instruction, lineCount);
        opCode = (instruction[0] == "LUI") ? opCode1 : opCode2;
    }
    
    string getEncoding() {
        return (offset + rd + opCode);
    }
};

vector<string> splitString(const string& str) {
    vector<string> vec;
    int i = 0, j = 0, n = str.size();
    while(i < n && j < n) {
        while((str[i] == ' ' || str[i] == ',' || str[i] == '(' || str[i] == ')')) i++;
        j = i;
        while(!(str[j] == ' ' || str[j] == ',' || str[j] == '(' || str[j] == ')')) j++;
        vec.push_back(str.substr(i, j-i));
        i = j;
        while((str[i] == ' ' || str[i] == ',' || str[i] == '(' || str[i] == ')')) i++;
        j = i;
    }

    int size = vec.size();
    for(int i = 0; i < size; i++) {
        if(vec[i][0] == '#') {
            size = i;
            break;
        }
    }
    while(vec.size() != size) vec.pop_back();
    return vec;
}

void prepareLabels(string& str, int& lineCount) {
    int i = 0, j = str.size() - 1;
    while(str[i] == ' ') i++;
    while(j >= 0 && str[j] == ' ') {
        j--;
        str.pop_back();
    }

    if(str[i] == '#') return;
    else if(str.back() == ':') {
        string label = str.substr(i);
        label.pop_back();
        transform(label.begin(), label.end(), label.begin(), ::tolower);
        if(labels.find(label) != labels.end()) {
            std::cout << "Label ambiguity !!" << endl;
            exit(0);
        }
        else labels[label] = to_string(4*(lineCount + 1));
    }
    else lineCount++;
}

void inputAndOutputHandler(string& str, ofstream& output, int& lineCount) {
    int i = 0, j = str.size() - 1;
    while(str[i] == ' ') i++;
    while(j >= 0 && str[j] == ' ') {
        j--;
        str.pop_back();
    }

    if(str[i] == '#') return;
    lineCount++;

    vector<string> instruction = splitString(str);   
    if(instruction.size() == 0) return;

    for(int i = 0; i < instruction.size(); i++) {
        transform(instruction[i].begin(), instruction[i].end(), instruction[i].begin(), (i == 0) ? ::toupper : ::tolower);
    }
    
    string ADDR_MODE = (funcToAddrMode.find(instruction[0]) != funcToAddrMode.end()) ? funcToAddrMode[instruction[0]] : "NIL";
    
    if(ADDR_MODE == REG && !isdigit(instruction[3][0])) {
        REGISTER R(instruction, lineCount);
        string encodedBinary = R.getEncoding();
        output << encodedBinary << endl;
    }
    else if(ADDR_MODE == IMM || (ADDR_MODE == REG && isdigit(instruction[3][0]))) {
        if(ADDR_MODE == REG) instruction[0].push_back('I');
        IMMEDIATE I(instruction, lineCount);
        string encodedBinary = I.getEncoding();
        output << encodedBinary << endl;
    }
    else if(ADDR_MODE == LOD) {
        swap(instruction[2], instruction[3]);
        LOAD L(instruction, lineCount);
        string encodedBinary = L.getEncoding();
        output << encodedBinary << endl;
    }
    else if(ADDR_MODE == SRE) {
        swap(instruction[2], instruction[3]);
        STORE S(instruction, lineCount);
        string encodedBinary = S.getEncoding();
        output << encodedBinary << endl;
    }
    else if(ADDR_MODE == BCH) {
        BRANCH B(instruction, lineCount);
        string encodedBinary = B.getEncoding();
        output << encodedBinary << endl;
    }
    else if(ADDR_MODE == JMP) {
        JUMP J(instruction, lineCount);
        string encodedBinary = J.getEncoding();
        output << encodedBinary << endl;
    }
    else if(ADDR_MODE == UIMM) {
        UPPER_IMMEDIATE U(instruction, lineCount);
        string encodedBinary = U.getEncoding();
        output << encodedBinary << endl;
    }
    else {
        if(instruction[0].back() != ':') {
            std::cout << "False instruction !!" << endl;
            exit(0);
        }
        lineCount--;
    }
}