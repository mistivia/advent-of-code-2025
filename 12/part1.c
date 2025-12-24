#include <algds/vec.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

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

void skip_to_line_end() {
    while (1) {
        int c = getchar();
        if (c != '\n' && c != EOF) {
            continue;
        } else {
            break;
        }
    }
}

int parse_piece() {
    skip_to_line_end();
    int count = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int c = getchar();
            if (c == '#') count++;
        }
        expect_char('\n');
    }
    expect_char('\n');
    return count;
}

typedef struct {
    int width;
    int height;
    int pieces[6];
} Case;

Case parse_case() {
    Case ret;
    ret.width = parse_integer();
    expect_char('x');
    ret.height = parse_integer();
    expect_char(':');
    expect_char(' ');
    ret.pieces[0] = parse_integer();
    expect_char(' ');
    ret.pieces[1] = parse_integer();
    expect_char(' ');
    ret.pieces[2] = parse_integer();
    expect_char(' ');
    ret.pieces[3] = parse_integer();
    expect_char(' ');
    ret.pieces[4] = parse_integer();
    expect_char(' ');
    ret.pieces[5] = parse_integer();
    int c = fpeek(stdin);
    if (c == '\n') {
        getchar();
    } else if (c == EOF) {
        return ret;
    } else {
        PANIC;
    }
    return ret;
}

int main () {
    int pieces[6];
    for (int i = 0; i < 6; i++) {
        pieces[i] = parse_piece();
    }
    int ret = 0;
    while (1) {
        int c = fpeek(stdin);
        if (c == EOF) {
            break;
        }
        Case cur = parse_case();
        int need = 0;
        for (int i = 0; i < 6; i++) {
            need += cur.pieces[i] * pieces[i];
        }
        if (cur.height * cur.width > need) {
            ret++;
        }
    }
    printf("%d\n", ret);
}