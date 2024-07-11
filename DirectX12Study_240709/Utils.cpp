#include "pch.h"

#include "Utils.h"

namespace Uitls
{
void reverse(char *arr)
{
    uint32_t len = strlen(arr);
    for (int32_t i = 0; i < len/2; i++)
    {
        std::swap(arr[i], arr[len - i - 1]);
    }
}

uint8_t *get_extension(const char *filename)
{

#define EXT_MAX_SIZE 4
    uint8_t *ans = (uint8_t *)malloc(EXT_MAX_SIZE);
    assert(ans);
    memset(ans, 0, EXT_MAX_SIZE);

    uint32_t len = uint32_t(strlen(filename));
    for (int32_t i = len - 1; i >= 0; i--)
    {
        if (filename[i] == '.')
            break;
        ans[len - i - 1] = filename[i];
    }
    reverse((char*)ans);
    return ans;
}
} // namespace Uitls
