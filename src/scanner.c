#include <stdint.h>

#include "main.h"
#include "vm.h"
#include "../obj/blarb.yy.c"

void BlarbVM_addLabelPointer(BlarbVM *vm, char *name, int line) {
    LabelPointer *newLabel = malloc(sizeof(LabelPointer));
    strncpy(newLabel->name, name, sizeof(newLabel->name));
    newLabel->line = line;
    HASH_ADD_STR(vm->labelPointers, name, newLabel);
}

void addStringToToken(token *t, char *str, size_t len) {
    t->str = malloc(len + 1);
    t->str[len] = '\0';
    strncpy(t->str, str, len);
}

// Scans a line of tokens using yylex
token* BlarbVM_scanLine(BlarbVM *vm) {
    int tokenCount = 0;
    // 1024 tokens should be way more than enough.
    token *line = malloc(sizeof(token) * 1024);
    token_t tokenType;

    while ((tokenType = yylex())) {
        token *t = &line[tokenCount++];
        // In general, we just care about the type.
        t->type = tokenType;

        if (tokenType == INTEGER) {
            t->val = strtoull(yytext, 0, 10);
        } else if (tokenType == FUNCTION_CALL) {
            addStringToToken(t, yytext, strlen(yytext));
        } else if (tokenType == LABEL) {
            // Exclude the hash
            addStringToToken(t, yytext + 1, strlen(yytext) - 1);
            BlarbVM_addLabelPointer(vm, t->str, vm->lineCount);
        } else if (tokenType == STR) {
            // Exclude quotes
            addStringToToken(t, yytext + 1, strlen(yytext) - 2);
        } else if (tokenType == NEWLINE) {
            break;
        }
    }

    // If this is the EOF (no more tokens)
    if (tokenCount == 0) {
        free(line);
        return 0;
    }

    // Resize the memory chunk so we don't waste memory on small lines
    return realloc(line, sizeof(token) * tokenCount);
}

void BlarbVM_loadFile(BlarbVM *vm, char *fileName) {
	FILE *fp = fopen(fileName, "r");
	if ( ! fp) {
		fprintf(stderr, "Failed to open '%s'\n", fileName);
		perror("fopen");
		terminateVM();
	}
    yyin = fp;
    yyfilename = fileName;

	token *line;
    while ((line = BlarbVM_scanLine(vm))) {
        BlarbVM_addLine(vm, line);
    }

    yyin = stdin;
	fclose(fp);
}

void BlarbVM_addLine(BlarbVM *vm, token *line) {
	if (vm->lineCount % 2 == 0) {
		// Double the size when space runs out - amortized time is O(1)
		vm->lines = vm->lineCount == 0
			? malloc(sizeof(token_t *) * 1)
			: realloc(vm->lines, sizeof(token_t *) * vm->lineCount * 2);
	}
	vm->lines[vm->lineCount++] = line;
}
