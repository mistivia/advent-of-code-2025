#include <algds/str.h>
#include <algds/vec.h>
#include <algds/hash_table.h>
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

int main() {
    mat = read_mat();
    height = mat.size;
    width = strlen(mat.buffer[0]);
    int ret = 0;
    Int2IntHashTable beams;
    Int2IntHashTable_init(&beams);
    for (int i = 0; i < width; i++) {
        if (mat.buffer[0][i] == 'S') {
            Int2IntHashTable_insert(&beams, i, 0xff);
        }
    }
    for (int i = 1; i < height; i++) {
        beams = split(beams, mat.buffer[i], &ret);
    }
    printf("%d\n", ret);

    for (int i = 0; i < height; i++) {
        free((void*)mat.buffer[i]);
    }
    StringVector_free(&mat);
    Int2IntHashTable_free(&beams);
    return 0;
}