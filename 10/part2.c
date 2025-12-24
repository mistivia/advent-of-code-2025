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


typedef struct {
    int16_t val[16];
} Joltage;

Joltage parse_button() {
    Joltage j;
    memset(&j, 0, sizeof(j));
    expect_char('(');
    while (1) {
        long num = parse_integer();
        j.val[num] = 1;
        int c = fpeek(stdin);
        if (c == ',') {
            getchar();
        } else {
            break;
        }
    }
    expect_char(')');
    return j;
}

void skip_indicator() {
    expect_char('[');
    while (1) {
        int c = getchar();
        if (c == ']') break;
    }
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
void Joltage_show(Joltage self, FILE *fp) {}

VECTOR_DEF(Joltage)
VECTOR_IMPL(Joltage)

JoltageVector parse_buttons() {
    skip_space();
    JoltageVector ret;
    JoltageVector_init(&ret);
    while (1) {
        int c = fpeek(stdin);
        if (c == '(') {
            Joltage btn = parse_button();
            JoltageVector_push_back(&ret, btn);
            skip_space();
        } else {
            break;
        }
    }
    return ret;
}

Joltage parse_joltage(int *targetlen) {
    int len = 0;
    skip_space();
    Joltage j;
    memset(&j, 0, sizeof(j));
    int i = 0;
    expect_char('{');
    while (1) {
        if (i >= 16) PANIC;
        int16_t n = parse_integer();
        j.val[i] = n;
        len++;
        int c = fpeek(stdin);
        if (c == ',') {            
            getchar();
        } else {
            break;
        }
        i++;
    }
    expect_char('}');
    skip_space();
    *targetlen = len;
    return j;
}

typedef struct {
    int a;
    int b;
} Frac;

int gcd(int a, int b) {
    a = abs(a);
    b = abs(b);
    while (b) {
        a %= b;
        int tmp = a;
        a = b;
        b = tmp;
    }
    return a;
}

Frac Frac_new(int a, int b) {
    if (b == 0) {
        PANIC;
    }
    int common = gcd(a, b);
    Frac res;
    res.a = a / common;
    res.b = b / common;
    if (res.b < 0) {
        res.a = -res.a;
        res.b = -res.b;
    }
    return res;
}

Frac Frac_add(Frac a, Frac b) {
    return Frac_new(a.a * b.b + b.a * a.b, a.b * b.b);
}

Frac Frac_sub(Frac a, Frac b) {
    return Frac_new(a.a * b.b - b.a * a.b, a.b * b.b);
}

Frac Frac_mul(Frac a, Frac b) {
    return Frac_new(a.a * b.a, a.b * b.b);
}

Frac frac_div(Frac a, Frac b) {
    if (b.a == 0) {
        PANIC;
    }
    return Frac_new(a.a * b.b, a.b * b.a);
}

int frac_is_zero(Frac f) {
    return f.a == 0;
}

void Frac_show(Frac f, FILE *fp) {
    if (f.b == 1) fprintf(fp, "%d", f.a);
    else fprintf(fp, "%d/%d", f.a, f.b);
}

typedef struct {
    int size;
    Frac *buf;
} FracVec;

typedef struct {
    int height;
    int width;
    Frac *buf;
} FracMat;

FracVec FracVec_new(int size) {
    FracVec v;
    v.size = size;
    v.buf = (Frac *)malloc(size * sizeof(Frac));
    for (int i = 0; i < size; i++) v.buf[i] = Frac_new(0, 1);
    return v;
}

void FracVec_free(FracVec *v) {
    if (v->buf) {
        free(v->buf);
        v->buf = NULL;
    }
    v->size = 0;
}

void FracVec_show(FracVec v, FILE *fp) {
    fprintf(fp, "[ ");
    for (int i = 0; i < v.size; i++) {
        Frac_show(v.buf[i], fp);
        if (i < v.size - 1) printf(", ");
    }
    fprintf(fp, " ]");
}

Frac FracVec_get(FracVec v, int x) {
    if (x < 0 || x >= v.size) PANIC;
    return v.buf[x];
}

void FracVec_set(FracVec v, int x, Frac val) {
    if (x >= v.size || x < 0) PANIC;
    v.buf[x] = val;
}

FracMat FracMat_new(int height, int width) {
    FracMat m;
    m.height = height;
    m.width = width;
    if (height == 0 || width == 0) {
        m.buf = NULL;
        return m;
    }
    m.buf = (Frac *)malloc(height * width * sizeof(Frac));
    for (int i = 0; i < height * width; i++) m.buf[i] = Frac_new(0, 1);
    return m;
}

void FracMat_free(FracMat *m) {
    if (m->buf) {
        free(m->buf);
        m->buf = NULL;
    }
    m->height = m->width = 0;
}

Frac FracMat_get(FracMat m, int row, int col) {
    if (row >= m.height || row < 0) PANIC;
    if (col >= m.width || col < 0) PANIC;
    return m.buf[row * m.width + col];
}

void FracMat_show(FracMat m, FILE *fp) {
    for (int i = 0; i < m.height; i++) {
        fprintf(fp, "| ");
        for (int j = 0; j < m.width; j++) {
            Frac_show(FracMat_get(m, i, j), fp);
            fprintf(fp, "\t");
        }
        fprintf(fp, "|\n");
    }
}

void FracMat_set(FracMat m, int row, int col, Frac val) {
    if (row >= m.height || row < 0) PANIC;
    if (col >= m.width || col < 0) PANIC;
    m.buf[row * m.width + col] = val;
}

void FracMat_swap_rows(FracMat m, int r1, int r2) {
    if (r1 == r2) return;
    for (int j = 0; j < m.width; j++) {
        Frac val1 = FracMat_get(m, r1, j);
        Frac val2 = FracMat_get(m, r2, j);
        FracMat_set(m, r1, j, val2);
        FracMat_set(m, r2, j, val1);
    }
}

FracVec FracMat_muladd(FracMat A, FracVec x, FracVec b) {
    if (A.width != x.size) {
        PANIC;
    }
    if (A.height != b.size) {
        PANIC;
    }
    FracVec res = FracVec_new(A.height);
    for (int i = 0; i < A.height; i++) {
        for (int j = 0; j < A.width; j++) {
            FracVec_set(res, i, Frac_add(res.buf[i], Frac_mul(FracMat_get(A, i, j), x.buf[j])));
        }
    }
    for (int i = 0; i < A.height; i++) {
        FracVec_set(res, i, Frac_add(res.buf[i], b.buf[i]));
    }
    return res;
}

int gauss_elim(FracMat A, FracVec b, FracMat *sA, FracVec *sb) {
    if (A.height != b.size) {
        fprintf(stderr, "Error: Dimension mismatch.\n");
        PANIC;
    }

    FracMat aug = FracMat_new(A.height, A.width + 1);
    for (int i = 0; i < A.height; i++) {
        for (int j = 0; j < A.width; j++) {
            FracMat_set(aug, i, j, FracMat_get(A, i, j));
        }
        FracMat_set(aug, i, A.width, FracVec_get(b, i));
    }
    int pivot_count = 0;
    for (int i = 0; i < A.width; i++) {
        int pivot_row = pivot_count;
        while (pivot_row < A.height && FracMat_get(aug, pivot_row, i).a == 0) {
            pivot_row++;
        }

        if (pivot_row == A.height) {
            continue;
        }
        // FracMat_show(aug, stdout); puts("");
        FracMat_swap_rows(aug, pivot_count, pivot_row);
        // FracMat_show(aug, stdout); puts("");
        Frac pivot = FracMat_get(aug, pivot_count, i);
        for (int j = i; j <= A.width; j++) {
            Frac val = FracMat_get(aug, pivot_count, j);
            FracMat_set(aug, pivot_count, j, frac_div(val, pivot));
        }

        for (int k = 0; k < A.height; k++) {
            if (k != pivot_count) {
                Frac factor = FracMat_get(aug, k, i);
                if (factor.a != 0) {
                    for (int j = i; j <= A.width; j++) {
                        Frac curr = FracMat_get(aug, k, j);
                        Frac subval = Frac_mul(factor, FracMat_get(aug, pivot_count, j));
                        FracMat_set(aug, k, j, Frac_sub(curr, subval));
                    }
                }
            }
        }
        pivot_count++;
    }
    // FracMat_show(aug, stdout); puts("");

    for (int i = 0; i < aug.height; i++) {
        int all_zeros = 1;
        for (int j = 0; j < aug.width - 1; j++) {
            if (!frac_is_zero(FracMat_get(aug, i, j))) {
                all_zeros = 0;
                break;
            }
        }
        if (all_zeros && !frac_is_zero(FracMat_get(aug, i, aug.width - 1))) {
            FracMat_free(&aug);
            return 0;
        }
    }
    int rank = 0;
    for (int i = 0; i < aug.height; i++) {
        int is_zero_row = 1;
        for (int j = 0; j < aug.width - 1; j++) {
            if (!frac_is_zero(FracMat_get(aug, i, j))) {
                is_zero_row = 0;
                break;
            }
        }
        if (!is_zero_row) rank++;
    }
    int free_var_count = A.width - rank;
    if (free_var_count == 0) {
        *sb = FracVec_new(rank);
        for (int i = 0; i < rank; i++) {
            FracVec_set(*sb, i, FracMat_get(aug, i, aug.width - 1));
        }
        return 1;
    }
    *sA = FracMat_new(rank, free_var_count);
    int rank_count = 0;
    free_var_count = 0;
    for (int i = 0; i < A.width; i++) {
        if (rank_count >= aug.height || FracMat_get(aug, rank_count, i).a == 0) {
            for (int j = 0; j < rank; j++) {
                Frac y = FracMat_get(aug, j, i);
                y.a = -y.a;
                FracMat_set(*sA, j, free_var_count, y);
            }
            free_var_count++;
        } else {
            rank_count++;
        }
    }
    *sb = FracVec_new(rank);
    for (int i = 0; i < rank; i++) {
        FracVec_set(*sb, i, FracMat_get(aug, i, aug.width - 1));
    }
    // puts("sA=");
    // FracMat_show(*sA, stdout); puts("");
    // puts("sb=");
    // FracVec_show(*sb, stdout); puts("");
    // fflush(stdout);

    return 2;
}

static void enum_impl(FracMat A, FracVec b, FracVec *x, int range, int cur, int *minsum) {
    if (cur == x->size) {
        FracVec res = FracMat_muladd(A, *x, b);
        int sum = 0;
        for (int i = 0; i < res.size; i++) {
            if (FracVec_get(res, i).b != 1) goto end;
            if (FracVec_get(res, i).a < 0) goto end;
            sum += FracVec_get(res, i).a;
        }
        for (int i = 0; i < x->size; i++) {
            sum += FracVec_get(*x, i).a;
        }
        if (sum < *minsum) {
            // FracVec_show(*x, stdout); FracVec_show(res, stdout); puts("");
            *minsum = sum;
        }
    end:
        FracVec_free(&res);
        return;
    }
    for (int i = 0; i <= range; i++) {
        FracVec_set(*x, cur, Frac_new(i, 1));
        enum_impl(A, b, x, range, cur + 1, minsum);
    }
}

int enum_free_var(FracMat A, FracVec b, int range) {
    int minsum = INT_MAX;
    FracVec x = FracVec_new(A.width);
    enum_impl(A, b, &x, range, 0, &minsum);
    return minsum;
}

int solve(int targetlen, Joltage target, JoltageVector *buttons) {
    int height = targetlen;
    int width = buttons->size;
    FracMat A = FracMat_new(height, width);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            FracMat_set(A, j, i, Frac_new(buttons->buffer[i].val[j], 1));
        }
    }
    FracVec b = FracVec_new(height);
    for (int i = 0; i < height; i++) {
        FracVec_set(b, i, Frac_new(target.val[i], 1));
    }
    FracMat sA;
    FracVec sb;
    int type = gauss_elim(A, b, &sA, &sb);
    if (type == 0) PANIC;
    if (type == 1) {
        int minsum = 0;
        for (int i = 0; i < sb.size; i++) {
            Frac x = FracVec_get(sb, i);
            if (x.b != 1) PANIC;
            if (x.a < 0) PANIC;
            minsum += x.a;
        }
        return minsum;
    }

    int range = 0;
    for (int i = 0; i < 16; i++) {
        if (target.val[i] > range) range = target.val[i];
    }
    int minsum = enum_free_var(sA, sb, range);
    // printf("%d\n", minsum);
    return minsum;
}

int main() {
    int r = 0;
    while (1) {
        int c = fpeek(stdin);
        if (c == '[') {
            skip_indicator();
            JoltageVector buttons = parse_buttons();
            int targetlen;
            Joltage j = parse_joltage(&targetlen);
            int solution = solve(targetlen, j, &buttons);
            // printf("%d\n", solution);
            r += solution;
            // fflush(stdout);
        } else {
            break;
        }
    }
    printf("%d\n", r);
    return 0;
}