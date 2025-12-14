#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

#include <algds/vec.h>
#include <algds/str.h>

#define PANIC do { \
    fprintf(stderr, "panic at %s:%d\n", __FILE__, __LINE__); \
    abort(); \
} while (0)

typedef struct {
    char *first;
    char *second;
} StrPair;

void StrPair_show(StrPair self, FILE *fp) {
    fprintf(fp, "{%s-%s}", self.first, self.second);
}

VECTOR_DEF(StrPair);
VECTOR_IMPL(StrPair);

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

void sep_or_end() {
    int c = fpeek(stdin);
    if (c == ',') {
        fgetc(stdin);
        skip_space();
        return;
    } else if (c == EOF) {
        return;
    }
    PANIC;
}

char *parse_integer() {
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
    return sb.buf;
}

StrPair parse_product() {
    StrPair ret;
    ret.first = parse_integer();
    expect_char('-');
    ret.second = parse_integer();
    sep_or_end();
    return ret;
}

StrPairVector parse() {
    StrPairVector res;
    StrPairVector_init(&res);
    while(1) {
        int c = fpeek(stdin);
        if (c == EOF) {
            return res;
        } else if (isdigit(c)) {
            StrPair sp = parse_product();
            StrPairVector_push_back(&res, sp);
        } else {
            PANIC;
        }
    }
    return res;
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

static int numlen(long num) {
    if (num < 0) return -1;
    int count = 1;
    while (num >= 10) {
        num /= 10;
        count++;
    }
    return count;
}

static int pow10halflen(long num) {
    int l = numlen(num);
    l /= 2;
    int res = 1;
    for (int i = 0; i < l; i++) {
        res *= 10;
    }
    return res;
}

static int is_invalid_id(long num) {
    long p = pow10halflen(num);
    if (num % p == num / p) {
        return 1;
    }
    return 0;
}

int main() {
    StrPairVector spv = parse();
    StrPairVector_show(spv, stdout);
    putchar('\n');
    StrPair * iter = StrPairVector_begin(&spv);
    int64_t sum = 0;
    for (; iter != StrPairVector_end(&spv); iter++) {
        long start, end;
        if (!string2long(iter->first, &start)) {
            PANIC;
        }
        if (!string2long(iter->second, &end)) {
            PANIC;
        }
        for (long t = start; t <= end; t++) {
            if (is_invalid_id(t)) {
                sum += t;
            } 
        }
    }
    printf("%ld\n", sum);
    return 0;
}