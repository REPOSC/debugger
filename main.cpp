#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>
#include <wait.h>
#include <sys/user.h>
#include <cstdlib>
#include <vector>
#include <beaengine/BeaEngine.h>
#include <memory.h>
#include <sys/reg.h>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

void run_target(const char* programname)
{
    std::cout << "target started. will run " << programname << std::endl;
    /* Allow tracing of this process */
    if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        std::cerr << "ptrace failed" << std::endl;
        exit(-1);
    }
    /* Replace this process's image with the given program */
    execl(programname, programname, 0);
}

void print_result(int icounter, const DISASM & disasm_info, int len, const long * instruction){    
    std::cout << std::dec << icounter << ":" ;
    std::cout << std::hex << " addr = " << disasm_info.VirtualAddr;
    std::cout << std::dec << " len = " << len;
    std::cout << " " << disasm_info.CompleteInstr << std::endl;
}

void print_hexes(const std::vector<unsigned char> & hexes){
    std::cout.fill('0');
    for (int i = 0; i < hexes.size(); ++i){
        std::cout.width(2);	
		std::cout << std::hex << (short)hexes[i] << " ";
    }
    std::cout << std::endl;
}

void print_hexes(const std::vector<unsigned long long> & hexes){
    std::cout.fill('0');
    for (int i = 0; i < hexes.size(); ++i){
        std::cout.width(16);	
		std::cout << std::hex << hexes[i] << " ";
    }
    std::cout << std::endl;
}

void write_to_file(std::vector<std::pair<std::string, std::string> > & src, const char * filename){
    std::ofstream ofs(filename);
    for (std::vector<std::pair<std::string, std::string> >::iterator it = src.begin(); it != src.end(); ++it){
        ofs << it->first << " :::::::::::: " << it->second << std::endl;
    }
    ofs.close();
}

void register_map(std::map<unsigned long long, unsigned long long> & registers, const struct user_regs_struct & regs){
    registers[REG0] = regs.rax;
    registers[REG1] = regs.rcx;
    registers[REG2] = regs.rdx;
    registers[REG3] = regs.rbx;
    registers[REG4] = regs.rsp;
    registers[REG5] = regs.rbp;
    registers[REG6] = regs.rsi;
    registers[REG7] = regs.rdi;
    registers[REG8] = regs.r8;
    registers[REG9] = regs.r9;
    registers[REG10] = regs.r10;
    registers[REG11] = regs.r11;
    registers[REG12] = regs.r12;
    registers[REG13] = regs.r13;
    registers[REG14] = regs.r14;
    registers[REG15] = regs.r15;
}

void run_debugger(pid_t child_pid)
{
    int wait_status;
    unsigned icounter = 0;
    int iflag = -1;
    std::cout << "debugger started" << std::endl;
    /* Wait for child to stop on its first instruction */
    wait(&wait_status);

    std::vector<unsigned char> rip_storages;
    std::vector<unsigned long long> addr_storages;
    // std::vector<std::pair<std::string, std::string> > op10000, op20000, op30000, op4040000, op4030000, op8040000;
    while(WIFSTOPPED(wait_status)) {
        icounter++;
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

        long instruction[4];
        for (int i = 0; i < sizeof(instruction) / sizeof(long); ++i){
            *((long *)((char *)instruction + i * sizeof(long))) = ptrace(PTRACE_PEEKTEXT, child_pid, regs.rip + sizeof(long) * i, 0);
        }
        DISASM disasm_info;
        memset(&disasm_info, 0, sizeof(disasm_info));
        disasm_info.VirtualAddr = regs.rip;
        disasm_info.SecurityBlock = 0;
        disasm_info.EIP = (UInt64)(instruction);
        int len = Disasm(&disasm_info);

        OPTYPE operands[] = {
            disasm_info.Operand1,
            disasm_info.Operand2,
            disasm_info.Operand3,
            disasm_info.Operand4,
            disasm_info.Operand5,
            disasm_info.Operand6,
            disasm_info.Operand7,
            disasm_info.Operand8,
            disasm_info.Operand9
        };

        // if (iflag > 0){
        //     std::cout << "after: " << std::hex << regs.rip << std::endl;
        //     std::cout << disasm_info.CompleteInstr << " ";
        //     std::cout.fill('0');
        //     for (int i = 0; i < len / sizeof(unsigned char); ++i){
        //         std::cout.width(2);	
        //         std::cout << std::hex << (short)(((unsigned char *)(instruction))[i]) << " ";
        //     }
        //     --iflag;
        // }

        if (len < 0){
            std::cerr << "Not supported: ";
            std::cout << disasm_info.CompleteInstr << " ";
            std::cout.fill('0');
            for (int i = 0; i < sizeof(instruction); ++i){
                std::cout.width(2);	
                std::cout << std::hex << (short)(((unsigned char *)(instruction))[i]) << " ";
            }
            std::cout << "before: " << std::hex << regs.rip << std::endl;
            iflag = 4;
        }
        else {
            if (std::string(disasm_info.Instruction.Mnemonic).find("lea") == -1){
                //std::cout << disasm_info.Instruction.Mnemonic << std::endl;
                for (int i = 0; i < sizeof(operands) / sizeof(OPTYPE); ++i){
                    if (operands[i].OpSize > 0){
                        // if (0x10000 == operands[i].OpType) op10000.push_back(std::pair<std::string, std::string>(disasm_info.CompleteInstr, operands[i].OpMnemonic));
                        // else if (0x20000 == operands[i].OpType) op20000.push_back(std::pair<std::string, std::string>(disasm_info.CompleteInstr, operands[i].OpMnemonic));
                        // else if (0x30000 == operands[i].OpType) op30000.push_back(std::pair<std::string, std::string>(disasm_info.CompleteInstr, operands[i].OpMnemonic));
                        // else if (0x4040000 == operands[i].OpType) op4040000.push_back(std::pair<std::string, std::string>(disasm_info.CompleteInstr, operands[i].OpMnemonic));
                        // else if (0x4030000 == operands[i].OpType) op4030000.push_back(std::pair<std::string, std::string>(disasm_info.CompleteInstr, operands[i].OpMnemonic));
                        // else if (0x8040000 == operands[i].OpType) op8040000.push_back(std::pair<std::string, std::string>(disasm_info.CompleteInstr, operands[i].OpMnemonic));
                        // else {
                        //     std::cerr << "Invalid OpType : " << std::hex << operands[i].OpType << std::endl;
                        //     exit(-1);
                        // }
                        if (0x30000 == operands[i].OpType){
                            std::map<unsigned long long, unsigned long long> registers;
                            register_map(registers, regs);
                            unsigned long long base = registers[operands[i].Memory.BaseRegister];
                            unsigned long long index = registers[operands[i].Memory.IndexRegister];
                            unsigned long long scale = operands[i].Memory.Scale;
                            unsigned long long displacement = registers[operands[i].Memory.Displacement];
                            unsigned long long result = base + index * scale + displacement;
                            addr_storages.push_back(result);
                        }
                        else if (0x4030000 == operands[i].OpType) {
                            char copied_str[24];
                            strncpy(copied_str, operands[i].OpMnemonic, strlen(operands[i].OpMnemonic) - 1);
                            unsigned long long result;
                            std::stringstream ss;
                            ss << std::hex << copied_str;
                            ss >> result;
                            addr_storages.push_back(result);
                        }
                    }
                }
            }
            
        }

        /* Make the child execute another instruction */
        if(ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return;
        }
        /* Wait for child to stop on its next instruction */
        wait(&wait_status);
    }
    // print_hexes(rip_storages);
    std::cout << addr_storages.size() << std::endl;
    print_hexes(addr_storages);

    // write_to_file(op10000, "10000.txt");
    // write_to_file(op20000, "20000.txt");
    // write_to_file(op30000, "30000.txt");
    // write_to_file(op4040000, "4040000.txt");
    // write_to_file(op4030000, "4030000.txt");
    // write_to_file(op8040000, "8040000.txt");

    std::cout << std::dec << "the child executed " << icounter << " instructions" << std::endl;
}

int main(int argc, char** argv)
{
    pid_t child_pid;
    if (argc < 2) {
        std::cerr << "Expected a program name as argument" << std::endl;
        exit(-1);
    }
    child_pid = fork();
    if (child_pid == 0)
        run_target(argv[1]);
    else if (child_pid > 0)
        run_debugger(child_pid);
    else {
        perror("fork");
        return -1;
    }
    return 0;
}