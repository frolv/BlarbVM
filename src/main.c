#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"
#include "vm.h"
#include "debugger.h"

// Flags
#define DEBUG 1
#define DEBUGGER 2

static BlarbVM *vm;

void abortWithUsage(char *arg0) {
    fprintf(stderr, "Usage: %s [--debug|--debugger] path/to/code.blarb\n", arg0);
    exit(1);
}

int main(int argc, char **argv) {
    int c;
    int flags = 0;
    char *fileName = NULL;

    static struct option long_opts[] = {
        { "debugger", no_argument, NULL, 'D' },
        { "debug", no_argument, NULL, 'd' },
        { 0, 0, 0, 0 }
    };

    while ((c = getopt_long(argc, argv, "Dd", long_opts, NULL)) != EOF) {
        switch (c) {
        case 'D':
            flags |= DEBUGGER;
            break;
        case 'd':
            flags |= DEBUG;
            break;
        default:
            abortWithUsage(argv[0]);
            break;
        }
    }

    if (optind != argc - 1)
            abortWithUsage(argv[0]);
    fileName = argv[optind];

    vm = BlarbVM_init();
    if ((flags & DEBUG) && (flags & DEBUGGER)) {
        printf("Invalid options: Can't have both --debug and --debugger\n");
        abortWithUsage(argv[0]);
    }

    BlarbVM_loadFile(vm, fileName);

    if (flags & DEBUGGER) {
        BlarbVM_debugger(vm);
    } else {
        BlarbVM_execute(vm);
        if (flags & DEBUG) {
            BlarbVM_dumpDebug(vm);
        }
    }
    BlarbVM_destroy(vm);

    return 0;
}

void terminateVM() {
    fprintf(stderr, "Terminating VM.\n");
    BlarbVM_dumpDebug(vm);
    BlarbVM_destroy(vm);
    exit(1);
}

