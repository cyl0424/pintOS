#include <stdint.h>

#define F (1 << 14)
#define INT_MAX ((1 << 31) - 1)
#define INT_MIN (-(1 << 31))

int int_to_fixed_point(int n);
int fixed_point_to_int(int x);
int fixed_point_to_int_round(int x);

int add_fixed_point(int x, int y);
int add_mixed(int x, int n);

int sub_fixed_point(int x, int y);
int sub_mixed(int x, int n);

int mult_fixed_point(int x, int y);
int mult_mixed(int x, int y);

int div_fixed_point(int x, int y);
int div_mixed(int x, int n);


int int_to_fixed_point(int n) {
    return n * F;
}

int fixed_point_to_int(int x) {
    return x / F;
}

int fixed_point_to_int_round(int x) {
    if (x >= 0) {
        return (x + F / 2) / F;
    } else {
        return (x - F / 2) / F;
    }
}

int add_fixed_point(int x, int y) {
    return x + y;
}

int add_mixed(int x, int n) {
    return x + n * F;
}

int sub_fixed_point(int x, int y) {
    return x - y;
}

int sub_mixed(int x, int n) {
    return x - n * F;
}

int mult_fixed_point(int x, int y) {
    return ((int64_t)x) * y / F;
}

int mult_mixed(int x, int n) {
    return x * n;
}

int div_fixed_point(int x, int y) {
    return ((int64_t)x) * F / y;
}

int div_mixed(int x, int n) {
    return x / n;
}
