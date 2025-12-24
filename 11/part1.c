#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>

#include <algds/str.h>
#include <algds/vec.h>
#include <algds/hash_table.h>
#include <algds/mmhash.h>
#include <time.h>

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

typedef struct {
    char name[3];
} Tag;

void Tag_show(Tag self, FILE *fp) {
    printf("%c%c%c", self.name[0], self.name[1], self.name[2]);
}

VECTOR_DEF(Tag)
VECTOR_IMPL(Tag)

uint64_t Tag_hash(Tag self) {
    return mmhash(self.name, 3, 0);
}

bool Tag_eq(Tag a, Tag b) {    
    return a.name[0] == b.name[0]
        && a.name[1] == b.name[1]
        && a.name[2] == b.name[2];
}

HASH_TABLE_DEF(Tag, TagVector);
HASH_TABLE_IMPL(Tag, TagVector);

HASH_TABLE_DEF(Tag, Long);
HASH_TABLE_IMPL(Tag, Long);

Tag parse_tag() {
    Tag t;
    t.name[0] = getchar();
    t.name[1] = getchar();
    t.name[2] = getchar();
    return t;
}

Tag Tag_new(const char *s) {
    Tag t;
    t.name[0] = s[0];
    t.name[1] = s[1];
    t.name[2] = s[2];
    return t;
}

Tag2TagVectorHashTable dag;

Tag2LongHashTable cache;

long search(Tag src) {
    if (Tag_eq(src, Tag_new("out"))) {
        return 1;
    }
    Tag2LongHashTableIter cache_it = Tag2LongHashTable_find(&cache, src);
    if (cache_it != NULL) {
        return cache_it->val;
    }
    long res = 0;
    Tag2TagVectorHashTableIter dag_it = Tag2TagVectorHashTable_find(&dag, src);
    TagVector nexthops = dag_it->val;
    for (int i = 0; i < nexthops.size; i++) {
        res += search(nexthops.buffer[i]);
    }
    Tag2LongHashTable_insert(&cache, src, res);
    return res;
}

int main() {
    Tag2TagVectorHashTable_init(&dag);
    Tag2LongHashTable_init(&cache);
    while (1) {
        int c = fpeek(stdin);
        if (isalpha(c)) {
            Tag t = parse_tag();
            expect_char(':');
            expect_char(' ');
            TagVector next;
            TagVector_init(&next);
            while (1) {
                int c = fpeek(stdin);
                if(!isalpha(c)) {
                    break;
                }
                TagVector_push_back(&next, parse_tag());
                c = fpeek(stdin);
                if (c == ' ') {
                    getchar();
                }
            }
            int c = fpeek(stdin);
            if (c == '\n') {
                getchar();
            }
            Tag2TagVectorHashTable_insert(&dag, t, next);
        } else {
            break;
        }
    }
    long ret = search(Tag_new("you"));
    printf("%ld\n", ret);
    return 0;
}