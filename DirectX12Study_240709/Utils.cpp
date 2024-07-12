#include "pch.h"

#include "Utils.h"

namespace Utils
{
void reverse(char *arr)
{
    uint32_t len = uint32_t(strlen(arr));
    for (uint32_t i = 0; i < len / 2; i++)
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
    reverse((char *)ans);
    return ans;
}

uint8_t *get_full_directory(const char *filepath, const char *filename)
{
    auto l1 = strlen(filepath);
    auto l2 = strlen(filename);
    uint8_t *absPath = (uint8_t *)malloc(l1 + l2 + 1);
    assert(absPath);

    strcpy_s((char *)absPath, l1 + 1, filepath);
    strcat_s((char *)absPath, l1 + l2 + 1, filename);

    return absPath;
}
} // namespace Uitls
