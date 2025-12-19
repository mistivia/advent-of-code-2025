#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <algds/str.h>
#include <algds/vec.h>


void getlines(StringVector *lines) {
    while (1) {
        char *line = fgetline(stdin);
        if (line == NULL) {
            break;
        }
        char *nline = str_strip(line);
        free(line);
        StringVector_push_back(lines, nline);
    }
}

void find_max_impl(const char *line, int len, int64_t *max, int d, int pos, int64_t res) {
    if (d >= 12) {
        if (res > *max) {
            *max = res;
        }
        return;
    }
    int curmax = -1;
    IntVector maxidx;
    IntVector_init(&maxidx);
    for (int i = pos + 1; i < len - (11-d); i++) {
        int n = line[i] - '0';
        if (n > curmax) {
            IntVector_free(&maxidx);
            IntVector_init(&maxidx);
            curmax = n;
            IntVector_push_back(&maxidx, i);
        } else if (n == curmax) {
            IntVector_push_back(&maxidx, i);
        }
    }
    Int *it = IntVector_begin(&maxidx);
    for (; it < IntVector_end(&maxidx); it++) {
        int i = *it;
        find_max_impl(line, len, max, d + 1, i, res * 10 + curmax);
    }
}

int64_t find_max(const char *line) {
    int64_t len = strlen(line);
    int64_t max = 0;
    find_max_impl(line, len, &max, 0, -1, 0);
    return max;
}


int main() {
    StringVector lines;
    StringVector_init(&lines);
    getlines(&lines);
    String *it = StringVector_begin(&lines);
    int64_t sum = 0;
    for (; it != StringVector_end(&lines); it++) {
        sum += find_max(*it);
    }
    printf("%"PRId64"\n", sum);
    return 0;
}