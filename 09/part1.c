#include <algds/sort.h>
#include <errno.h>
#include <ctype.h>

#include <algds/vec.h>
#include <algds/str.h>
#include <stdio.h>

typedef struct {
    int x;
    int y;
} Vec2;

long rec_area(Vec2 a, Vec2 b) {
    long dx = a.x - b.x;
    long dy = a.y - b.y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return (dx + 1) * (dy + 1);
}

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

Vec2 parse_vec2() {
    int x = parse_integer();
    expect_char(',');
    int y = parse_integer();
    return (Vec2){.x = x, .y = y};
}

void Vec2_show(Vec2 self, FILE *fp) {
    fprintf(fp, "(%d,%d)", self.x, self.y);
}

VECTOR_DEF(Vec2)
VECTOR_IMPL(Vec2)

int main() {
    Vec2Vector tiles;
    Vec2Vector_init(&tiles);
    while (1) {
        int c = fpeek(stdin);
        if (!isdigit(c)) {
            break;
        }
        Vec2 v = parse_vec2();
        c = fpeek(stdin);
        if (c == '\n') {
            fgetc(stdin);
        }
        Vec2Vector_push_back(&tiles, v);
    }
    long max_area = 0;
    for (int i = 0; i < tiles.size; i++) {
        for (int j = i + 1; j < tiles.size; j++) {
            long area = rec_area(tiles.buffer[i], tiles.buffer[j]);
            if (area > max_area) {
                max_area = area;
            }
        }
    }
    printf("%ld\n", max_area);

    Vec2Vector_free(&tiles);
    return 0;
}