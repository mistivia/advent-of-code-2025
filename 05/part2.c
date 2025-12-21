#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include <algds/vec.h>
#include <algds/str.h>
#include <algds/sort.h>
#include <stdlib.h>

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
        if (isspace(c)) {
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

typedef struct {
    int64_t start;
    int64_t end;
} Range;

void Range_show(Range self, FILE *fp) {
    fprintf(fp, "%ld-%ld", self.start, self.end);
}

VECTOR_DEF(Range)
VECTOR_IMPL(Range)

Range parse_range() {
    Range ret;
    ret.start = parse_integer();
    expect_char('-');
    ret.end = parse_integer();
    expect_char('\n');
    return ret;
}


int Range_cmp(Range a, Range b) {
    if (a.start < b.start) {
        return -1;
    } else if (a.start == b.start) {
        return 0;
    } else {
        return 1;
    }
}

QSORT_DEF(Range)
QSORT_IMPL(Range)

int main() {
    RangeVector ranges, unioned;
    RangeVector_init(&ranges);
    RangeVector_init(&unioned);
    int c;
    while (1) {
        c = fpeek(stdin);
        if (c == '\n') {
            break;
        } else if (isdigit(c)) {
            Range r = parse_range();
            RangeVector_push_back(&ranges, r);
        } else {
            PANIC;
        }
    }
    expect_char('\n');
    Range_qsort(ranges.buffer, RangeVector_len(&ranges));
    if (RangeVector_len(&ranges) < 1) {
        PANIC;
    }
    Range current = ranges.buffer[0];
    for (int i = 1; i < RangeVector_len(&ranges); i++) {
        if (ranges.buffer[i].start > current.end) {
            RangeVector_push_back(&unioned, current);
            current = ranges.buffer[i];
            continue;
        } else if (ranges.buffer[i].end > current.end) {
            current.end = ranges.buffer[i].end;
        }
    }
    RangeVector_push_back(&unioned, current);
    long res = 0;
    for (int i = 0; i < RangeVector_len(&unioned); i++) {
        Range cur = unioned.buffer[i];
        res += cur.end - cur.start + 1;
    }
    printf("%ld\n", res);

    RangeVector_free(&ranges);
    RangeVector_free(&unioned);
    return 0;
}