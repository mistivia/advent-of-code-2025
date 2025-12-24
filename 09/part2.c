#include <algds/sort.h>
#include <errno.h>
#include <ctype.h>

#include <algds/vec.h>
#include <algds/str.h>
#include <algds/basic_traits.h>
#include <algds/tree_map.h>
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

typedef struct {
    int64_t start;
    int64_t end;
} Range;

void Range_show(Range self, FILE *fp) {
    fprintf(fp, "%ld-%ld", self.start, self.end);
}

VECTOR_DEF(Range)
VECTOR_IMPL(Range)

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

void merge_ranges(RangeVector *ranges) {
    Range_qsort(ranges->buffer, RangeVector_len(ranges));
    if (RangeVector_len(ranges) < 1) {
        return;
    }
    RangeVector merged;
    RangeVector_init(&merged);
    Range current = ranges->buffer[0];
    for (int i = 1; i < RangeVector_len(ranges); i++) {
        if (ranges->buffer[i].start > current.end) {
            RangeVector_push_back(&merged, current);
            current = ranges->buffer[i];
            continue;
        } else if (ranges->buffer[i].end > current.end) {
            current.end = ranges->buffer[i].end;
        }
    }
    RangeVector_push_back(&merged, current);
    RangeVector_free(ranges);
    *ranges = merged;
}

TREE_MAP_DEF(Int, RangeVector)
TREE_MAP_IMPL(Int, RangeVector)

Int2RangeVectorTreeMap hlines;
Int2RangeVectorTreeMap vlines;

void init_lines() __attribute__((constructor));
void init_lines() {
    Int2RangeVectorTreeMap_init(&hlines);
    Int2RangeVectorTreeMap_init(&vlines);
}

VECTOR_DEF(RangeVector)
VECTOR_IMPL(RangeVector)

RangeVectorVector vsegs;
RangeVectorVector hsegs;

void init_segs() __attribute__((constructor));
void init_segs() {
    RangeVectorVector_init(&vsegs);
    RangeVectorVector_init(&hsegs);
}

int height = -1;
int width = -1;

void add_line(Vec2 a, Vec2 b) {
    int start, end;
    if (a.x == b.x) {
        if (a.y < b.y) {
            start = a.y;
            end = b.y;
        } else {
            start = b.y;
            end = a.y;
        }
        Int2RangeVectorTreeMapIter it = Int2RangeVectorTreeMap_find(&vlines, a.x);
        if (it == NULL) {
            RangeVector rs;
            RangeVector_init(&rs);
            Int2RangeVectorTreeMap_insert(&vlines, a.x, rs);
            it = Int2RangeVectorTreeMap_find(&vlines, a.x);
        }
        RangeVector_push_back(&it->value, (Range){ .start = start, .end = end });
    } else if (a.y == b.y) {
        if (a.x < b.x) {
            start = a.x;
            end = b.x;
        } else {
            start = b.x;
            end = a.x;
        }
        Int2RangeVectorTreeMapIter it = Int2RangeVectorTreeMap_find(&hlines, a.y);
        if (it == NULL) {
            RangeVector rs;
            RangeVector_init(&rs);
            Int2RangeVectorTreeMap_insert(&hlines, a.y, rs);
            it = Int2RangeVectorTreeMap_find(&hlines, a.y);
        }
        RangeVector_push_back(&it->value, (Range){ .start = start, .end = end });
    } else {
        PANIC;
    }
}

bool is_sub_range(Range r, RangeVector *ranges) {
    for (int i = 0; i < ranges->size; i++) {
        Range target = ranges->buffer[i];
        if (target.end >= r.end && target.start <= r.start) {
            return true;
        }
    }
    return false;
}

bool in_open_ranges(int x, RangeVector *ranges) {
    for (int i = 0; i < ranges->size; i++) {
        Range r = ranges->buffer[i];
        if (x < r.end && x >= r.start) {
            return true;
        }
    }
    return false;
}

void scan_lines() {
    // hsegs
    for (int i = 0; i < height; i++) {
        Int2RangeVectorTreeMapIter it = Int2RangeVectorTreeMap_find(&hlines, i);
        if (it != NULL) {
            for (int j = 0; j < it->value.size; j++) {
                RangeVector_push_back(&hsegs.buffer[i], it->value.buffer[j]);
            }
        }
        int cross_cnt = 0;
        int enter_pos;
        it = Int2RangeVectorTreeMap_min(&vlines);
        for (; it != NULL; it = Int2RangeVectorTreeMap_next(&vlines, it)) {
            if (in_open_ranges(i, &it->value)) {
                if (cross_cnt % 2 == 0) {
                    enter_pos = it->key;
                }
                cross_cnt++;
                if (cross_cnt % 2 == 0) {
                    RangeVector_push_back(&hsegs.buffer[i], (Range){.start = enter_pos, .end = it->key});
                }
            }
        }
        merge_ranges(&hsegs.buffer[i]);
    }
    // vsegs
    for (int i = 0; i < width; i++) {
        Int2RangeVectorTreeMapIter it = Int2RangeVectorTreeMap_find(&vlines, i);
        if (it != NULL) {
            for (int j = 0; j < it->value.size; j++) {
                RangeVector_push_back(&vsegs.buffer[i], it->value.buffer[j]);
            }
        }
        int cross_cnt = 0;
        int enter_pos;
        it = Int2RangeVectorTreeMap_min(&hlines);
        for (; it != NULL; it = Int2RangeVectorTreeMap_next(&hlines, it)) {
            if (in_open_ranges(i, &it->value)) {
                if (cross_cnt % 2 == 0) {
                    enter_pos = it->key;
                }
                cross_cnt++;
                if (cross_cnt % 2 == 0) {
                    RangeVector_push_back(&vsegs.buffer[i], (Range){.start = enter_pos, .end = it->key});
                }
            }
        }
        merge_ranges(&vsegs.buffer[i]);
    }
}

bool is_valid_rec(Vec2 a, Vec2 b) {
    int x1, x2, y1, y2, t;
    x1 = a.x;
    x2 = b.x;
    y1 = a.y;
    y2 = b.y;
    if (x1 > x2) {
        t = x2;
        x2 = x1;
        x1 = t;
    }
    if (y1 > y2) {
        t = y2;
        y2 = y1;
        y1 = t;
    }
    if (!is_sub_range((Range){.start = x1, .end = x2}, &hsegs.buffer[y1])) {
        return false;
    }
    if (!is_sub_range((Range){.start = x1, .end = x2}, &hsegs.buffer[y2])) {
        return false;
    }
    if (!is_sub_range((Range){.start = y1, .end = y2}, &vsegs.buffer[x1])) {
        return false;
    }
    if (!is_sub_range((Range){.start = y1, .end = y2}, &vsegs.buffer[x2])) {
        return false;
    }
    return true;
}

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
    
    for (int i = 0; i < tiles.size; i++) {
        if (tiles.buffer[i].x > width) {
            width = tiles.buffer[i].x;
        }
        if (tiles.buffer[i].y > height) {
            height = tiles.buffer[i].y;
        }
    }
    width++; height++;
    for (int i = 0; i < width; i++) {
        RangeVector segs;
        RangeVector_init(&segs);
        RangeVectorVector_push_back(&vsegs, segs);
    }
    for (int i = 0; i < height; i++) {
        RangeVector segs;
        RangeVector_init(&segs);
        RangeVectorVector_push_back(&hsegs, segs);
    }
    for (int i = 0; i < tiles.size - 1; i++) {
        add_line(tiles.buffer[i], tiles.buffer[i+1]);
    }
    add_line(tiles.buffer[0], tiles.buffer[tiles.size - 1]);
    scan_lines();
    long max_area = 0;
    for (int i = 0; i < tiles.size; i++) {
        for (int j = i + 1; j < tiles.size; j++) {
            long area = rec_area(tiles.buffer[i], tiles.buffer[j]);
            if (area > max_area && is_valid_rec(tiles.buffer[i], tiles.buffer[j])) {
                max_area = area;
            }
        }
    }
    printf("%ld\n", max_area);

    Vec2Vector_free(&tiles);
    Int2RangeVectorTreeMapIter it;
    for (it = Int2RangeVectorTreeMap_min(&hlines); it != NULL; it = Int2RangeVectorTreeMap_next(&hlines, it)) {
        RangeVector_free(&it->value);
    }
    Int2RangeVectorTreeMap_free(&hlines);
    for (it = Int2RangeVectorTreeMap_min(&vlines); it != NULL; it = Int2RangeVectorTreeMap_next(&vlines, it)) {
        RangeVector_free(&it->value);
    }
    Int2RangeVectorTreeMap_free(&vlines);
    for (int i = 0; i < vsegs.size; i++) {
        RangeVector_free(&vsegs.buffer[i]);
    }
    RangeVectorVector_free(&vsegs);
    for (int i = 0; i < hsegs.size; i++) {
        RangeVector_free(&hsegs.buffer[i]);
    }
    RangeVectorVector_free(&hsegs);
    return 0;
}