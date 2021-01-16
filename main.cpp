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
    freopen("test.txt", "w", stdout);
    for (int i = 0; i < hexes.size(); ++i){
        std::cout.width(2);	
		std::cout << std::hex << (short)hexes[i] << " ";
    }
    std::cout << std::endl;
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
        if (iflag > 0){
            std::cout << disasm_info.CompleteInstr << " ";
            std::cout.fill('0');
            for (int i = 0; i < len / sizeof(unsigned char); ++i){
                std::cout.width(2);	
                std::cout << std::hex << (short)(((unsigned char *)(instruction))[i]) << " ";
            }
            std::cout << "after: " << std::hex << regs.rip << std::endl;
            --iflag;
        }
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

        /* Make the child execute another instruction */
        if(ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return;
        }
        /* Wait for child to stop on its next instruction */
        wait(&wait_status);
    }
    // print_hexes(rip_storages);
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