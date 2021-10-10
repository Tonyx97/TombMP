#include "3dglodef.h"

#include <cstring>
#include <cstdint>
#include <utility>

#pragma warning(disable : 4146)

struct matrix
{
    int32_t m00;
    int32_t m01;
    int32_t m02;
    int32_t m03;

    int32_t m10;
    int32_t m11;
    int32_t m12;
    int32_t m13;

    int32_t m20;
    int32_t m21;
    int32_t m22;
    int32_t m23;
};

static_assert(sizeof(matrix) == 48);

inline matrix* get_current_matrix()
{
    return (matrix*)phd_mxptr;
}

inline void set_current_matrix(matrix* m)
{
    phd_mxptr = (int32_t*)m;
}

void phd_PushMatrix()
{
    matrix* current_matrix = get_current_matrix();
    matrix* next_matrix = current_matrix + 1;

    std::memcpy(next_matrix, current_matrix, sizeof(matrix));

    set_current_matrix(next_matrix);
}

void phd_PushUnitMatrix()
{
    set_current_matrix(get_current_matrix() + 1);
    phd_UnitMatrix();
}

void phd_UnitMatrix()
{
    matrix* m = get_current_matrix();

    *m = matrix
    {
        .m00 = W2V_SCALE,
        .m11 = W2V_SCALE,
        .m22 = W2V_SCALE,
    };
}

constexpr uint32_t deg_360 = 65536;
constexpr uint32_t deg_180 = deg_360 / 2;
constexpr uint32_t deg_90 = deg_360 / 4;
constexpr uint32_t deg_45 = deg_360 / 8;
constexpr uint32_t steps = 2048;

extern uint16_t sin_table[];
extern uint16_t tan_table[];
extern int32_t octant_table[];

int32_t phd_sin(int32_t angle)
{
    uint32_t a = uint32_t(angle) & (deg_360 - 1);

    bool negate = false;

    if (a >= deg_180)
    {
        a -= deg_180;
        negate = true;
    }

    if (a > deg_90)
        a = -a + deg_180;

    uint32_t result = sin_table[a >> 4];

    if (negate)
        result = -result;

    return int(result);
}

int32_t phd_cos(int32_t angle)
{
    return phd_sin(angle + deg_90);
}

uint32_t phd_sqrt(uint32_t n)
{
    uint32_t result = 0,
             x = 0x40000000;

    while (x != 0)
    {
        const uint32_t y = result >> 1;

        result += x;

        if (result > n)
            result = y;
        else
        {
            n -= result;
            result = x | y;
        }

        x >>= 2;
    }

    return result;
}

int16_t phd_atan(int32_t x, int32_t y)
{
    uint32_t octant = 0;

    if (x == 0 && y == 0)
        return 0;

    if (x < 0)
    {
        octant += 4;
        x = -x;
    }

    if (y < 0)
    {
        octant += 2;
        y = -y;
    }

    if (y > x)
    {
        octant += 1;
        std::swap(x, y);
    }

    int result = tan_table[(y * steps) / x] + octant_table[octant];
    if (result < 0)
        result = -result;

    return result;
}
