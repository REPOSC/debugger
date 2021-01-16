#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>
#include <wait.h>
#include <sys/user.h>
#include <cstdlib>
#include <vector>
#include <beaengine/BeaEngine.h>
#include <memory.h>

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

void run_debugger(pid_t child_pid)
{
    int wait_status;
    unsigned icounter = 0;
    std::cout << "debugger started" << std::endl;
    /* Wait for child to stop on its first instruction */
    wait(&wait_status);

    std::vector<unsigned long long int> rip_storages;
    while(WIFSTOPPED(wait_status)) {
        icounter++;
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);

        long instruction[16];

        for (int i = 0; i < sizeof(instruction) / sizeof(long); ++i){
            *((long *)((char *)instruction + i * sizeof(long))) = ptrace(PTRACE_PEEKTEXT, child_pid, regs.rip, 0);
        }
        
        //rip_storages.push_back(regs.rip);
        DISASM disasm_info;
        memset(&disasm_info, 0, sizeof(disasm_info));
        disasm_info.VirtualAddr = regs.rip;
        disasm_info.SecurityBlock = sizeof(instruction);
        disasm_info.EIP = (UInt64)(instruction);
        int len = Disasm(&disasm_info);
        if (len >= sizeof(instruction)){
            std::cerr << "Instruction too long" << std::endl;
            exit(-1);
        }

        std::cout << "addr = " << std::hex << disasm_info.VirtualAddr << ", len = " << std::dec << len << " : " << disasm_info.CompleteInstr << std::endl;

        //unsigned instr = ptrace(PTRACE_PEEKTEXT, child_pid, regs.rip, 0);
        //std::cout << "icounter = " << icounter << ", RIP = " << regs.rip << " instr = " << instr << std::endl;

        /* Make the child execute another instruction */
        if(ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
            perror("ptrace");
            return;
        }
        /* Wait for child to stop on its next instruction */
        wait(&wait_status);
    }
    std::cout << "the child executed " << icounter << " instructions" << std::endl;
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