#include <algds/sort.h>
#include <errno.h>
#include <ctype.h>

#include <algds/vec.h>
#include <algds/str.h>
#include <stdio.h>

typedef struct {
    int x;
    int y;
    int z;
} Vec3;

long distance(Vec3 a, Vec3 b) {
    long dx = a.x - b.x;
    long dy = a.y - b.y;
    long dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
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

Vec3 parse_vec3() {
    int x = parse_integer();
    expect_char(',');
    int y = parse_integer();
    expect_char(',');
    int z = parse_integer();
    return (Vec3){.x = x, .y = y, .z = z};
}

void Vec3_show(Vec3 self, FILE *fp) {
    fprintf(fp, "(%d,%d,%d)", self.x, self.y, self.z);
}

VECTOR_DEF(Vec3)
VECTOR_IMPL(Vec3)

typedef struct {
    int ia;
    int ib;
    long distance;
} DistanceEntry;

void DistanceEntry_show(DistanceEntry self, FILE *fp) {
    fprintf(fp, "(%d->%d: %ld)", self.ia, self.ib, self.distance);
}

VECTOR_DEF(DistanceEntry)
VECTOR_IMPL(DistanceEntry)

int DistanceEntry_cmp(DistanceEntry a, DistanceEntry b) {
    if (a.distance < b.distance) return -1;
    if (a.distance > b.distance) return 1;
    return 0;
}

QSORT_DEF(DistanceEntry)
QSORT_IMPL(DistanceEntry)

VECTOR_DEF(IntVector);
VECTOR_IMPL(IntVector);

void mark_circuit(IntVector *circuit, IntVectorVector *neighbors, int nid, int cid) {
    circuit->buffer[nid] = cid;
    for (int i = 0; i < neighbors->buffer[nid].size; i++) {
        int next = neighbors->buffer[nid].buffer[i];
        if (circuit->buffer[next] != cid) {
            mark_circuit(circuit, neighbors, next, cid);
        }
    }
}

int main() {
    Vec3Vector junctions;
    Vec3Vector_init(&junctions);
    while (1) {
        int c = fpeek(stdin);
        if (!isdigit(c)) {
            break;
        }
        Vec3 v = parse_vec3();
        c = fpeek(stdin);
        if (c == '\n') {
            fgetc(stdin);
        }
        Vec3Vector_push_back(&junctions, v);
    }
    DistanceEntryVector distances;
    DistanceEntryVector_init(&distances);
    for (int i = 0; i < Vec3Vector_len(&junctions); i++) {
        for (int j = i + 1; j < Vec3Vector_len(&junctions); j++) {
            long d = distance(junctions.buffer[i], junctions.buffer[j]);
            DistanceEntryVector_push_back(&distances, (DistanceEntry){.ia = i, .ib = j, .distance = d});
        }
    }
    DistanceEntry_qsort(distances.buffer, distances.size);
    IntVectorVector neighbors;
    IntVectorVector_init(&neighbors);
    for (int i = 0; i < junctions.size; i++) {
        IntVector v;
        IntVector_init(&v);
        IntVectorVector_push_back(&neighbors, v);
    }
    for (int i = 0; i < 1000; i++) {
        int ida = distances.buffer[i].ia;
        int idb = distances.buffer[i].ib;
        IntVector_push_back(&neighbors.buffer[ida], idb);
        IntVector_push_back(&neighbors.buffer[idb], ida);
    }
    IntVector circuit;
    IntVector_init(&circuit);
    for (int i = 0; i < junctions.size; i++) {
        IntVector_push_back(&circuit, i);
    }
    for (int i = 0; i < junctions.size; i++) {
        if (circuit.buffer[i] == i) {
            mark_circuit(&circuit, &neighbors, i, i);
        }
    }
    IntVector circuit_size;
    IntVector_init(&circuit_size);
    for (int i = 0; i < junctions.size; i++) {
        IntVector_push_back(&circuit_size, 0);
    }
    for (int i = 0; i < circuit.size; i++) {
        int cid = circuit.buffer[i];
        circuit_size.buffer[cid]++;
    }
    Int_qsort(circuit_size.buffer, circuit_size.size);
    int len = circuit_size.size;
    long ret = circuit_size.buffer[len - 1] * circuit_size.buffer[len - 2] * circuit_size.buffer[len - 3];
    printf("%ld\n", ret);

    IntVector_free(&circuit_size);
    IntVector_free(&circuit);
    for (int i = 0; i < neighbors.size; i++) {
        IntVector_free(&neighbors.buffer[i]);
    }
    IntVectorVector_free(&neighbors);
    DistanceEntryVector_free(&distances);
    Vec3Vector_free(&junctions);
    return 0;
}