#define MAX_ACCESS 10000

void constant_access(){
    int p;
    for (int i = 0; i < MAX_ACCESS; ++i){
        p = 0x55aa;
    }
}

void linear_access(){
    int p[MAX_ACCESS];
    for (int i = 0; i < MAX_ACCESS; ++i){
        p[i] = 0x55aa;
    }
}

void _start(){
    constant_access();
    //linear_access();
/* Exit code */
#if defined(__x86_64__)
    asm("mov $60, %rax;" "mov $0, %rdi;" "syscall");
#elif defined(__i386__)
    asm("movl $1,%eax;" "xorl %ebx,%ebx;" "int $0x80");
#endif
}