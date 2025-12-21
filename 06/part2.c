#include <algds/str.h>
#include <algds/vec.h>
#include <ctype.h>
#include <string.h>

#define PANIC do { \
    fprintf(stderr, "panic at %s:%d\n", __FILE__, __LINE__); \
    abort(); \
} while (0)

StringVector read_mat() {
    StringVector mat;
    StringVector_init(&mat);
    while (1) {
        char *l = fgetline(stdin);
        if (!l) {
            return mat;
        }
        if (strlen(l) == 0) {
            return mat;
        }
        int len = strlen(l);
        if (l[len - 1] == '\n') {
            l[len - 1] = '\0';
        }
        StringVector_push_back(&mat, l);
    }
    return mat;
}

char mat_read(StringVector *mat, int x, int y) {
    const char *line = *StringVector_ref(mat, y);
    return line[x];
}

char scan_column(StringVector *mat, int x, int *n) {
    int out = 0;
    int have_digit = 0;
    for (int y = 0; y < mat->size - 1; y++) {
        char c = mat_read(mat, x, y);
        if (c == ' ') {
            continue;
        } else if (isdigit(c)) {
            out = out * 10 + c - '0';
            have_digit = 1;
        } else {
            PANIC;
        }
    }
    if (have_digit) {
        *n = out;
    }
    return mat_read(mat, x, mat->size - 1);
}

int main() {
    StringVector lines = read_mat();
    long maxlen = 0;
    for (int i = 0; i < lines.size; i++) {
        long len = strlen(lines.buffer[i]);
        if (len > maxlen) {
            maxlen = len;
        }
    }
    for (int i = 0; i < lines.size; i++) {
        long len = strlen(lines.buffer[i]);
        if (len < maxlen) {
            char *spaces = malloc(maxlen - len + 1);
            memset(spaces, '0', maxlen - len);
            spaces[maxlen - len] = '\0';
            char *ns = malloc(maxlen + 1);
            strcat(ns, lines.buffer[i]);
            strcat(ns + len, spaces);
            free(spaces);
            free((void*)lines.buffer[i]);
            lines.buffer[i] = ns;
        }
    }
    long ret = 0;
    IntVector read_nums;
    IntVector_init(&read_nums);
    for (int x = strlen(lines.buffer[0]) - 1; x >= 0; x--) {
        int cur = -1;
        char sym = scan_column(&lines, x, &cur);
        if (cur >= 0) {
            IntVector_push_back(&read_nums, cur);
        }
        if (sym == '+') {
            long sum = 0;
            for (int i = 0; i < read_nums.size; i++) {
                sum += read_nums.buffer[i];
            }
            ret += sum;
            IntVector_free(&read_nums);
            IntVector_init(&read_nums);
        } else if (sym == '*') {
            long prod = 1;
            for (int i = 0; i < read_nums.size; i++) {
                prod *= read_nums.buffer[i];
            }
            ret += prod;
            IntVector_free(&read_nums);
            IntVector_init(&read_nums);
        }
    }
    printf("%ld\n", ret);
    
    IntVector_free(&read_nums);
    for (int i = 0; i < lines.size; i++) {
        free((void*)lines.buffer[i]);
    }
    StringVector_free(&lines);
    return 0;
}