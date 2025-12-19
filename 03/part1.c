#include <stdio.h>
#include <string.h>

#include <algds/str.h>
#include <algds/vec.h>


void getlines(StringVector *lines) {
    while (1) {
        char *line = fgetline(stdin);
        if (line == NULL) {
            break;
        }
        StringVector_push_back(lines, line);
    }
}

int find_max(const char *line) {
    int len = strlen(line);
    int max = 0;
    for (int i = 0; i < len - 1; i++) {
        for (int j = i + 1; j < len; j++) {
            int n = (line[i] - '0') * 10 + line[j] - '0';
            if (n > max) {
                max = n;
            }
        }
    }
    return max;
}

int main() {
    StringVector lines;
    StringVector_init(&lines);
    getlines(&lines);
    String *it = StringVector_begin(&lines);
    int sum = 0;
    for (; it != StringVector_end(&lines); it++) {
        sum += find_max(*it);
    }
    printf("%d\n", sum);
    return 0;
}