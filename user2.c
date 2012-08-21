#include "defs.h"
#include "ulib.h"

void fork_user3(void) {
    char *pargs[] = {"user3", "user2"};
    int pid = fork();
    if (pid < 0) {
        puts("user2: can't fork user 'user3'\n");
    }
    else if (pid == 0) {
        execve("0x60000", pargs);
        puts("user2: can't execve user 'user3'\n");
        exit();
    }
}

void do_work(char symbol) {
    sleep(300);
    write(symbol);
}

int main(int argc, char *argv[]) {
    int i = 0;

    printf("External process 'USER 2' running, argc = %x argv = %x\n",
           argc, (int)argv);

    /* print out the command line arguments */
    for (i = 0; i < argc; i++) {
        printf("arg %d = %x = %s\n", i, argv[i], argv[i]);
    }
    puts("\n");

    for (i = 0; i < 2; i++) {
        do_work('-');
    }

    fork_user3();
    puts("External process 'USER 2' exiting\n");

    return 0;
}

