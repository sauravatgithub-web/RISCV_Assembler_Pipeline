#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <unordered_map>
#include "assembler.hpp"
using namespace std;

#define DM dataMemory
#define IM instructionMemory

int pc = 0;
vector<pair<int, bool>> registerSet(32, {0, false});
vector<string> instructionMemory;
vector<int> dataMemory(64);

unordered_map<string, bool> controller = {
    { "ALU_Src", false }, { "RegRead",  false }, { "RegWrite", false }, { "ALU_Op1", false },
    { "MemRead", false }, { "MemWrite", false }, { "Mem2Reg",  false }, { "ALU_Op0", false },
    { "Branch",  false }, { "Jump",     false }, { "JumpR",    false }, { "UppImm",  false }
};

bool fetchActive   = true;
bool decodeActive  = false;
bool executeActive = false;
bool memoryActive  = false;
bool writeActive   = false;

class _IFID_ {
public:
    string IR;
    int direct_pc, new_pc;
};

class _IDEX_ {
public:
    int direct_pc, new_pc, jump_pc, branchImm, memoryImm, rs1, rs2, rd;
    pair<int, bool> sr1, sr2;
    string func;
    unordered_map<string, bool> controlWord;
};

class _EXMO_ {
public:
    int ALU_OUT, rs2, rd;
    unordered_map<string, bool> controlWord;
};

class _MOWB_ {
public:
    int LD_OUT, ALU_OUT, rd, output;
    unordered_map<string, bool> controlWord;
};

_IFID_ IFID;
_IDEX_ IDEX;
_EXMO_ EXMO;
_MOWB_ MOWB;

int forwardA = 0, forwardB = 0, hazardDetector = 0;

int binToNum(string binary, bool notReg = true) {
    int number = 0;
    int n = binary.length();
    
    for(int i = 0; i < n; ++i) {
        if(binary[i] == '1') {
            if(i == 0 && notReg) number -= std::pow(2, n - 1);
            else number += std::pow(2, n - 1 - i); 
        }
    }
    
    return number;
}

string ALU_Control(string func, bool control_1, bool control_2) {
    if(!control_1 && !control_2) return "0000";
    else if(!control_1 && control_2) {
        if      (func.substr(1) == "000") return "1111";  // beq
        else if (func.substr(1) == "001") return "1110";  // bne
        else if (func.substr(1) == "100") return "1101";  // blt
        else if (func.substr(1) == "101") return "1100";  // bge
        else if (func.substr(1) == "110") return "1011";  // bltu
        else if (func.substr(1) == "111") return "1010";  // bgeu  
        else                              return "0000";
    }
    else if(control_1 && !control_2) {
        if      (func == "1000") return "0001";     // subtract
        else if (func == "0111") return "0010";     // and
        else if (func == "0110") return "0011";     // or
        else if (func == "0100") return "0100";     // xor
        else if (func == "0001") return "0101";     // sll
        else if (func == "0101") return "0110";     // srl
        else if (func == "1101") return "0111";     // sra
        else if (func == "0010") return "1000";     // slt
        else if (func == "0011") return "1001";     // sltu
        else                     return "0000";
    }
    else return "0000";
}

int ALU(int input1, int input2, string select) {
    if      (select == "0000") return input1 + input2;
    else if (select == "0001") return input1 - input2;
    else if (select == "0010") return input1 & input2;
    else if (select == "0011") return input1 | input2;
    else if (select == "0100") return input1 ^ input2;
    else if (select == "0101") return (unsigned int)input1 << input2;
    else if (select == "0110") return (unsigned int)input1 >> input2;
    else if (select == "0111") return input1 >> input2;
    else if (select == "1000") return (input1 < input2) ? 1 : 0;
    else if (select == "1001") return (input1 < (unsigned int)input2) ? 1 : 0;
}

void reInitialize(string opCode) {
    for(auto& pair : controller) pair.second = false;
    if(opCode == "0110011") {                                     // REGISTER
        controller["RegRead"]  = true;
        controller["RegWrite"] = true;
        controller["ALU_Op1"]  = true;
    }
    else if((opCode == "0010011") || (opCode == "1100111")) {         // IMMEDIATE  
        controller["ALU_Src"]  = true;
        controller["RegRead"]  = true;
        controller["RegWrite"] = true;
        controller["ALU_Op1"]  = true;
        if(opCode == "1100111") controller["JumpR"] = true;
    }
    else if(opCode == "0000011") {                                // LOAD
        controller["ALU_Src"]  = true;
        controller["RegRead"]  = true;
        controller["RegWrite"] = true;
        controller["MemRead"]  = true;
        controller["Mem2Reg"]  = true;
    }
    else if(opCode == "0100011") {                                // STORE
        controller["ALU_Src"]  = true;
        controller["RegRead"]  = true;
        controller["MemWrite"] = true;
    }
    else if(opCode == "1100011") {                                // BRANCH
        controller["Branch"]   = true;
        controller["RegRead"]  = true;
        controller["ALU_Op0"]  = true;
    } 
    else if(opCode == "1101111") {                                // JUMP
        controller["Jump"]     = true;
        controller["RegWrite"] = true;
        controller["ALU_Op1"]  = true;
    }
    else if(opCode == "0110111" || opCode == "0010111") {          // UPPER IMMEDIATE
        controller["UppImm"]   = true;
        controller["ALU_Op1"]  = true;
        controller["RegWrite"] = true;
    }
}

inline int MUX(int in1, int in2, bool d_factor) {
    return (d_factor) ? in1 : in2;
}

void INSTRUCTION_FETCH() {
    if(hazardDetector == 1) {
        hazardDetector = 0;
        IFID.IR = "00000000000000000000000000000000";
        IFID.direct_pc = 1000000;
        IFID.new_pc = 1000000;
        return;
    }

    if(pc < 4*IM.size()) {
        IFID.IR = IM[pc/4];
        IFID.direct_pc = pc;
        IFID.new_pc = pc + 4;

        pc = IFID.new_pc;
        decodeActive = true;
    }
    else {
        fetchActive = false;
    }
}

void INSTRUCTION_DECODE() { 
    if(!fetchActive) {
        decodeActive = false;
        return;
    }

    IDEX.direct_pc = IFID.direct_pc;
    IDEX.new_pc = IFID.new_pc;
    string preJumpImm = IFID.IR.substr(0, 20);
    string jumpImm = preJumpImm[0] + preJumpImm.substr(12) + preJumpImm[11] + preJumpImm.substr(1, 10);
    jumpImm.push_back('0');
    string postJumpImm = ((jumpImm[0] == '0') ? "00000000000" : "11111111111") + jumpImm;
    IDEX.jump_pc = IFID.direct_pc + binToNum(postJumpImm);

    string preBranchImm = string(1, IFID.IR[0]) + string(1, IFID.IR[24]) + IFID.IR.substr(1, 6) + IFID.IR.substr(20, 4);
    IDEX.branchImm = binToNum(preBranchImm);

    string opCode = IFID.IR.substr(25, 7);
    reInitialize(opCode);
    IDEX.controlWord = controller;
 
    if(IDEX.controlWord["RegRead"] || IDEX.controlWord["JumpR"]) {
        IDEX.sr1.first = binToNum(IFID.IR.substr(12, 5), false);
        IDEX.sr2.first = binToNum(IFID.IR.substr(7, 5), false);
        IDEX.rs1 = registerSet[IDEX.sr1.first].first;
        IDEX.rs2 = registerSet[IDEX.sr2.first].first;
        if(registerSet[IDEX.sr1.first].second) IDEX.sr1.second = true;
        if(registerSet[IDEX.sr2.first].second) IDEX.sr2.second = true;
        if(IDEX.controlWord["JumpR"]) hazardDetector = 1;
    }
    if(IDEX.controlWord["Jump"]) {
        IDEX.rs1 = IFID.new_pc;
        IDEX.rs2 = 0;
        IDEX.sr1.second = false;
        IDEX.sr2.second = false;
        hazardDetector  = 1;
    }
    if(IDEX.controlWord["UppImm"]) {
        IDEX.rs1 = (opCode[1] != '0') ? 0 : IFID.direct_pc;
        IDEX.rs2 = binToNum(IFID.IR.substr(0, 20)) << 12;
        IDEX.sr1.second = false;
        IDEX.sr2.second = false;
    }
    if(IDEX.controlWord["Branch"]) {
        hazardDetector = 1;
    }
    
    string storeImm = IFID.IR.substr(0, 7) + IFID.IR.substr(20, 5);
    string loadImm = IFID.IR.substr(0, 12);
    
    IDEX.memoryImm = binToNum(controller["MemWrite"] ? storeImm : loadImm);
    IDEX.func = string(1, IFID.IR[1]) + IFID.IR.substr(17, 3);
    if(opCode == "0010011") {
        if(IDEX.func == "1000") IDEX.func = "0000";
    }
    if(IDEX.controlWord["UppImm"]) IDEX.func = "0000";

    IDEX.rd = binToNum(IFID.IR.substr(20, 5), false);
    registerSet[IDEX.rd].second = true;

    executeActive = true;
}

void INSTRUCTION_EXECUTE() {
    if(!decodeActive) {
        executeActive = false;
        return;
    }
    forwardA = 0;
    forwardB = 0;

    if(!IDEX.controlWord["Jump"] && !IDEX.controlWord["JumpR"] && !IDEX.controlWord["UppImm"] && MOWB.controlWord["RegWrite"] && MOWB.rd != 0 && MOWB.rd == IDEX.sr1.first) forwardA = 1;
    if(!IDEX.controlWord["Jump"] && !IDEX.controlWord["JumpR"] && !IDEX.controlWord["UppImm"] && MOWB.controlWord["RegWrite"] && MOWB.rd != 0 && MOWB.rd == IDEX.sr2.first) forwardB = 1;

    int inputData1 = 0, inputData2 = 0, secondInput = 0;

    if(forwardA == 0) inputData1 = (IDEX.sr1.second) ? registerSet[IDEX.sr1.first].first : IDEX.rs1;
    else if(forwardA == 1) inputData1 = MUX(MOWB.ALU_OUT, MOWB.LD_OUT, !MOWB.controlWord["MemRead"]);

    if(forwardB == 0) secondInput = (IDEX.sr2.second) ? registerSet[IDEX.sr2.first].first : IDEX.rs2;
    else if(forwardB == 1) secondInput = MUX(MOWB.ALU_OUT, MOWB.LD_OUT, !MOWB.controlWord["MemRead"]);
    
    inputData2 = MUX(IDEX.memoryImm, secondInput, IDEX.controlWord["ALU_Src"]);

    string ALU_Select = ALU_Control(IDEX.func, IDEX.controlWord["ALU_Op1"], IDEX.controlWord["ALU_Op0"]);
    EXMO.ALU_OUT = ALU(inputData1, inputData2, ALU_Select);
    
    bool zeroFlag = false;
    if      (ALU_Select == "1111") zeroFlag = (inputData1 == inputData2);
    else if (ALU_Select == "1110") zeroFlag = (inputData1 != inputData2);
    else if (ALU_Select == "1101") zeroFlag = (inputData1 <  inputData2);
    else if (ALU_Select == "1100") zeroFlag = (inputData1 >= inputData2);
    else if (ALU_Select == "1011") zeroFlag = (inputData1 <  inputData2);
    else if (ALU_Select == "1010") zeroFlag = (inputData1 >= inputData2);

    EXMO.controlWord = IDEX.controlWord;
    EXMO.rs2 = secondInput;
    EXMO.rd = IDEX.rd;
    bool flag = zeroFlag && IDEX.controlWord["Branch"];

    int branch_offset = IDEX.branchImm << 1;
    int branch_pc = branch_offset + IDEX.direct_pc;
    if(IDEX.controlWord["Branch"] || IDEX.controlWord["Jump"] || IDEX.controlWord["JumpR"]) {
        if(IDEX.controlWord["Branch"]) pc = MUX(branch_pc, IDEX.new_pc, flag);
        else if(IDEX.controlWord["Jump"]) pc = IDEX.jump_pc;
        else {
            pc = EXMO.ALU_OUT;
            EXMO.ALU_OUT = IDEX.new_pc;
        }
    }

    memoryActive = true;
}

void MEMORY_ACCESS() {
    if(!executeActive) {
        memoryActive = false;
        return;
    }

    if(EXMO.controlWord["MemRead"]) MOWB.LD_OUT = DM[EXMO.ALU_OUT / 4];
    if(EXMO.controlWord["MemWrite"]) DM[EXMO.ALU_OUT / 4] = EXMO.rs2;
    MOWB.controlWord = EXMO.controlWord;
    MOWB.ALU_OUT = EXMO.ALU_OUT;
    MOWB.rd = EXMO.rd;

    writeActive = true;
}

void WRITE_BACK() {
    if(!memoryActive) {
        writeActive = false;
        return;
    }

    MOWB.output = MUX(MOWB.LD_OUT, MOWB.ALU_OUT, MOWB.controlWord["Mem2Reg"]);
    if(MOWB.controlWord["RegWrite"] && MOWB.rd != 0) {
        registerSet[MOWB.rd].first = MOWB.output;
    }
    registerSet[MOWB.rd].second = false;
}


int main() {
    ifstream input("assembly.txt");
    ofstream output("binary.txt");
    int lineCount = 0;

    if(input.is_open()) {
        string line;
        while(getline(input, line)) {
            if(line == "") continue;
            prepareLabels(line, lineCount);
        }
    }
    else cout << "Unable to locate input file !!" << endl; 

    input.clear();
    input.seekg(0, ios::beg);
    lineCount = 0;

    if(input.is_open() && output.is_open()) {
        string line;
        while(getline(input, line)) {
            if(line == "") continue;
            inputAndOutputHandler(line, output, lineCount);
        }
        input.close();
    }
    else cout << "Unable to locate output file !!" << endl;
    output.close();

    input.open("binary.txt");
    if(input.is_open()) {
        string line;
        while(getline(input, line)) {
            IM.push_back(line);
        }
    } 
    input.close();
    
    while(pc < 4*IM.size() || fetchActive || decodeActive || executeActive || memoryActive || writeActive) {
        if(writeActive) WRITE_BACK();
        if(memoryActive) MEMORY_ACCESS();
        if(executeActive) INSTRUCTION_EXECUTE();
        if(decodeActive) INSTRUCTION_DECODE();
        if(fetchActive) INSTRUCTION_FETCH();
        if(pc >= 4*IM.size() && !decodeActive && !executeActive && !memoryActive && !writeActive) break;
    }

    cout << endl << string(22, ' ') << "REGISTER VALUES" << endl << endl;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 4; j++) {
            int index = (i * 4) + j;
            cout << left << setw(5) << ('x' + to_string(index) + ":")  
                 << setw(10) << registerSet[index].first << "   ";  
        }
        cout << endl << endl;
    }

    cout << endl << string(50, ' ') << "MEMORY VALUES" << endl << endl;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            int index = (i * 8) + j;  
            cout << left << setw(8) << ("DM " + to_string(index) + ":")  
                 << setw(5) << DM[index] << "   ";  
        }
        cout << endl;
    }

    cout << endl << "(Check binary.txt for machine instructions)" << endl << endl;

    return 0;
}