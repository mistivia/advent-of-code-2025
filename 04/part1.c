#include <algds/str.h>
#include <algds/vec.h>

StringVector readmap() {
    StringVector map;
    StringVector_init(&map);
    while (1) {
        char *l = fgetline(stdin);
        if (!l) {
            return map;
        }
        char *nl = str_strip(l);
        free(l);
        l = nl;
        if (strlen(l) == 0) {
            return map;
        }
        StringVector_push_back(&map, l);
    }
    return map;
}

char map_read(StringVector *map, int x, int y) {
    int height = StringVector_len(map);
    if (height == 0) {
        return '\0';
    }
    int width = strlen(*StringVector_ref(map, 0));
    if (x >= width || x < 0) {
        return '\0';
    }
    if (y >= height || y < 0) {
        return '\0';
    }
    return (*StringVector_ref(map, y))[x];
}


bool is_paper(StringVector *map, int x, int y) {
    return map_read(map, x, y) == '@';
}

int count_adjacent(StringVector *map, int x, int y) {
    int count = 0;
    if (is_paper(map, x - 1, y - 1)) {
        count++;
    }
    if (is_paper(map, x - 1, y)) {
        count++;
    }
    if (is_paper(map, x, y - 1)) {
        count++;
    }
    if (is_paper(map, x + 1, y + 1)) {
        count++;
    }
    if (is_paper(map, x + 1, y)) {
        count++;
    }
    if (is_paper(map, x, y + 1)) {
        count++;
    }
    if (is_paper(map, x + 1, y - 1)) {
        count++;
    }
    if (is_paper(map, x - 1, y + 1)) {
        count++;
    }
    return count;
}

void free_map(StringVector *map) {
    for (int i = 0; i < StringVector_len(map); i++) {
        free((void*)*StringVector_ref(map, i));
    }
    StringVector_free(map);
}

int main() {
    StringVector map = readmap();
    int height = StringVector_len(&map);
    int width = strlen(*StringVector_ref(&map, 0));
    int res = 0;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (map_read(&map, x, y) != '@') {
                continue;
            }
            if (count_adjacent(&map, x, y) < 4) {
                res++;
            }
        }
    }
    printf("%d\n", res);

    free_map(&map);
    return 0;
}