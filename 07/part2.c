#include <algds/str.h>
#include <algds/vec.h>
#include <algds/hash_table.h>
#include <algds/mmhash.h>
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

StringVector mat;
int height;
int width;

Int2IntHashTable split(Int2IntHashTable input, const char* line, int *count) {
    Int2IntHashTable new;
    Int2IntHashTable_init(&new);
    Int2IntHashTableIter iter = Int2IntHashTable_begin(&input);
    for (; iter != NULL; iter = Int2IntHashTable_next(&input, iter)) {
        int x = iter->key;
        if (line[x] == '.') {
            if (Int2IntHashTable_find(&new, x) == NULL) {
                Int2IntHashTable_insert(&new, x, 0xff);
            }
        } else if (line[x] == '^') {
            *count += 1;
            if (x - 1 >= 0 && Int2IntHashTable_find(&new, x-1) == NULL) {
                Int2IntHashTable_insert(&new, x-1, 0xff);
            }
            if (x + 1 < width && Int2IntHashTable_find(&new, x+1) == NULL) {
                Int2IntHashTable_insert(&new, x+1, 0xff);
            }
        } else {
            PANIC;
        }
    }
    Int2IntHashTable_free(&input);
    return new;
}

typedef struct {
    int cur;
    int d;
} IntPair;

uint64_t IntPair_hash(IntPair p) {
    uint64_t h = 0;
    h = mmhash(&p.cur, sizeof(int), h);
    h = mmhash(&p.d, sizeof(int), h);
    return h;
}

uint64_t IntPair_eq(IntPair a, IntPair b) {
    return a.cur == b.cur && a.d == b.d;
}

HASH_TABLE_DEF(IntPair, Long)
HASH_TABLE_IMPL(IntPair, Long)

IntPair2LongHashTable cache;

void set_trace_cache(int cur, int d, long value) {
    IntPair2LongHashTable_insert(&cache, (IntPair){.cur = cur, .d = d}, value);
}

bool get_trace_cache(int cur, int d, long *value) {
    Long* it = IntPair2LongHashTable_get(&cache, (IntPair){.cur = cur, .d = d});
    if (it == NULL) {
        return false;
    }
    *value = *it;
    return true;
}

long trace(int cur, int d) {
    long cached_value = 0;
    if (get_trace_cache(cur, d, &cached_value)) {
        return cached_value;
    }
    if (cur < 0 || cur >= width) PANIC;
    if (d >= height) {
        return 1;
    }
    const char *line = mat.buffer[d];
    if (line[cur] == '.') {
        long ret = trace(cur, d+1);
        set_trace_cache(cur, d, ret);
        return ret;
    } else if (line[cur] == '^') {
        long ret = trace(cur+1, d+1) + trace(cur-1, d+1);
        set_trace_cache(cur, d, ret);
        return ret;
    }
    PANIC;
}

int main() {
    mat = read_mat();
    height = mat.size;
    width = strlen(mat.buffer[0]);
    Int2IntHashTable beams;
    Int2IntHashTable_init(&beams);
    IntPair2LongHashTable_init(&cache);
    int start = -1;
    for (int i = 0; i < width; i++) {
        if (mat.buffer[0][i] == 'S') {
            start = i;
            break;
        }
    }
    long ret = 0;
    ret = trace(start, 1);
    printf("%ld\n", ret);
    return 0;
}