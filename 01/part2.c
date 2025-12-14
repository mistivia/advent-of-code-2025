#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buf[1024];

char * readline() {
    int i = 0;
    int c;
    while (1) {
        c = fgetc(stdin);
        if (c == EOF) {
            if (i == 0) {
                return NULL;
            }
            goto end;
        }
        if (c == '\n') {
            goto end;
        }
        buf[i] = c;
        i++;
    }
end:
    buf[i] = '\0';
    return strdup(buf);
}

int main() {
    int dial = 50;
    char *line;
    int count = 0;
    while ((line = readline()) != NULL) {
        int negative = 0;
        if (line[0] == 'L') negative = 1;
        char *end;
        long val = strtol(line+1, &end, 10);
        if (val >= 100) {
            count += val / 100;
            val = val % 100;
        }
        if (negative) val = -val;
        if (end == line+1) continue;
        int olddial = dial;
        dial = dial + val;
        if (dial >= 100) {
            count++;
        }
        if (olddial > 0 && dial <= 0) {
            count++;
        }
        dial = dial % 100;
        if (dial < 0) dial += 100;
    }
    printf("%d\n", count);
    return 0;
}