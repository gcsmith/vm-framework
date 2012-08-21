#include "defs.h"
#include "ulib.h"

extern unsigned int _start;
extern unsigned int __bss_start;
extern unsigned int _end;

unsigned int bss_mem[8];
unsigned int dat_mem[] = {1, 2, 4, 8, 16, 32, 64, 128};

void fork_user3(void) {
    char *pargs[] = {"user3", "user1"};
    int pid = fork();
    if (pid < 0) {
        puts("user1: can't fork user 'user3'\n");
    }
    else if (pid == 0) {
        execve("0x60000", pargs);
        puts("user1: can't execve user 'user3'\n");
        exit();
    }
}

int main(int argc, char *argv[]) {
    int i = 0;

    printf("External process 'USER 1' running, argc = %x argv = %x\n",
           argc, (int)argv);

    /* verify our linker symbols */
    printf("[_start] %x ", (int)&_start);
    printf("[__bss_start] %x ", (int)&__bss_start);
    printf("[_end] %x\n", (int)&_end);

    /* this should print out all 0 if our BSS is cleared */
    puts("BSS mem = {");
    for (i = 0; i < 8; i++) {
        printf(" %x ", bss_mem[i]);
    }
    puts("}\n");

    /* this should print out 1-N if everything is sane */
    puts("DATA mem = {");
    for (i = 0; i < 8; i++) {
        printf(" %x ", dat_mem[i]);
    }
    puts("}\n\n");

    for (i = 0; i < 5; i++) {
        sleep(10);
        write('+');
    }

    fork_user3();
    puts("External process 'USER 1' exiting\n");

    return 0;
}

