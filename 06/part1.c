#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include <algds/vec.h>
#include <algds/str.h>

#define PANIC do { \
    fprintf(stderr, "panic at %s:%d\n", __FILE__, __LINE__); \
    abort(); \
} while (0)

void expect_char(char e) {
    int c = fpeek(stdin);
    if (c != e) {
        PANIC;
    }
    fgetc(stdin);
}

void skip_space() {
    while (1) {
        int c = fpeek(stdin);
        if (c == EOF) return;
        if (c == ' ') {
            fgetc(stdin);
        } else {
            return;
        }
    }
}

bool string2long(const char *s, long *num) {
    char *end;
    errno = 0;
    *num = strtol(s, &end, 10);
    if (end == s) {
        return false;
    }
    if (errno != 0) {
        return false;
    }
    return true;
}

int64_t parse_integer() {
    long ret;
    StrBuilder sb;
    StrBuilder_init(&sb);
    while (1) {
        int c = fpeek(stdin);
        if (isdigit(c)) {
            fgetc(stdin);
            StrBuilder_append_char(&sb, c);
        } else {
            break;
        }
    }
    if (strlen(sb.buf) == 0) {
        PANIC;
    }
    StrBuilder_append_char(&sb, '\0');
    if (!string2long(sb.buf, &ret)) {
        PANIC;
    }
    free(sb.buf);
    return ret;
}

IntVector parse_ints() {
    IntVector ints;
    IntVector_init(&ints);
    while (1) {
        int c = fpeek(stdin);
        if (isdigit(c)) {
            int n = parse_integer();
            IntVector_push_back(&ints, n);
            skip_space();
        } else if (c == '\n' || c == EOF) {
            fgetc(stdin);
            return ints;
        } else {
            PANIC;
        }
    }
    return ints;
}

CharVector parse_syms() {
    CharVector syms;
    CharVector_init(&syms);
    while (1) {
        int c = fpeek(stdin);
        if (c == '+' || c == '*') {
            fgetc(stdin);
            CharVector_push_back(&syms, c);
            skip_space();
        } else if (c == '\n' || c == EOF) {
            return syms;
        } else {
            PANIC;
        }
    }
    return syms;
}

VECTOR_DEF(IntVector)
VECTOR_IMPL(IntVector)

int main() {
    IntVectorVector nums;
    CharVector syms;
    IntVectorVector_init(&nums);
    while (1) {
        skip_space();
        int c = fpeek(stdin);
        if (isdigit(c)) {
            IntVector line = parse_ints();
            IntVectorVector_push_back(&nums, line);
        } else {
            syms = parse_syms();
            break;
        }
    }

    int height = IntVectorVector_len(&nums);
    long ret = 0;
    for (int i = 0; i < CharVector_len(&syms); i++) {
        if (syms.buffer[i] == '+') {
            long r = 0;
            for (int j = 0; j < height; j++) {
                r += nums.buffer[j].buffer[i];
            }
            ret += r;
        } else if (syms.buffer[i] == '*') {
            long r = 1;
            for (int j = 0; j < height; j++) {
                r *= nums.buffer[j].buffer[i];
            }
            ret += r;
        } else {
            PANIC;
        }
    }
    printf("%ld\n", ret);

    for (int j = 0; j < height; j++) {
        IntVector_free(nums.buffer + j);
    }
    IntVectorVector_free(&nums);
    CharVector_free(&syms);
    return 0;
}