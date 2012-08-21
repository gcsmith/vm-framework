#include "ulib.h"

void print_head(int argc, char *argv[]) {
    printf("External process 'USER 3' running (from %s)\n", argv[1]);
}

void print_tail(void) {
    puts("External process 'USER 2' exiting\n");
}

void do_print(int argc, char *argv[]) {
    int i;
    print_head(argc, argv);

    for (i = 0; i < 4; i++) {
        sleep(300);
        write('3');
    }

    print_tail();
}

int main(int argc, char *argv[]) {
    do_print(argc, argv);
    return 0;
}

