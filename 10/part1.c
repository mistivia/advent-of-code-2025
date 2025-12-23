#include <algds/sort.h>
#include <algds/type_alias.h>
#include <errno.h>
#include <ctype.h>

#include <algds/vec.h>
#include <algds/str.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

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

uint32_t parse_indicator() {
    expect_char('[');
    uint32_t ret = 0;
    int n = 0;
    while (1) {
        int c = fpeek(stdin);
        if (c == '.') {
            getchar();
        } else if (c == '#') {
            getchar();
            // printf("n=%d\n", n);
            ret |= (1 << n);
            // printf("ret=%d\n", ret);
        } else {
            break;
        }
        n++;
    }
    expect_char(']');
    return ret;
}

uint32_t parse_button() {
    uint32_t ret = 0;
    expect_char('(');
    while (1) {
        long num = parse_integer();
        ret |= (1 << num);
        int c = fpeek(stdin);
        if (c == ',') {
            getchar();
        } else {
            break;
        }
    }
    expect_char(')');
    return ret;
}

void skip_space() {
    while (1) {
        int c = fpeek(stdin);
        if (c == EOF) return;
        if (isspace(c)) {
            fgetc(stdin);
        } else {
            return;
        }
    }
}

UIntVector parse_buttons() {
    skip_space();
    UIntVector ret;
    UIntVector_init(&ret);
    while (1) {
        int c = fpeek(stdin);
        if (c == '(') {
            uint32_t btn = parse_button();
            UIntVector_push_back(&ret, btn);
            skip_space();
        } else {
            break;
        }
    }
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

void solve_impl(uint32_t target, UIntVector *buttons, uint32_t curind, int curbtn, int curtimes, int *mintimes) {
    if (target == curind) {
        if (curtimes < *mintimes) {
            *mintimes = curtimes;
        }
    }
    if (curbtn >= buttons->size) {
        return;
    }
    uint32_t btn = buttons->buffer[curbtn];
    solve_impl(target, buttons, curind, curbtn+1, curtimes, mintimes);
    solve_impl(target, buttons, curind ^ btn, curbtn+1, curtimes+1, mintimes);
}

int solve(uint32_t indicator, UIntVector *buttons) {
    int mintimes = INT_MAX;
    solve_impl(indicator, buttons, 0, 0, 0, &mintimes);
    if (mintimes == INT_MAX) {
        PANIC;
    }
    return mintimes;
}

int main() {
    int r = 0;
    while (1) {
        int c = fpeek(stdin);
        if (c == '[') {
            uint32_t indicator = parse_indicator();
            UIntVector buttons = parse_buttons();
            // printf("%d\n", indicator);
            // UIntVector_show(buttons, stdout); puts("");
            fflush(stdout);
            int solution = solve(indicator, &buttons);
            r += solution;
            // printf("solution=%d\n", solution);
            skip_to_line_end();
        } else {
            break;
        }
    }
    printf("%d\n", r);
    return 0;
}