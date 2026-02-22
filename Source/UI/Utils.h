#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#include <string>

namespace UI::Utils
{
    std::string ConvertToUTF8(const std::wstring& wStr);
    std::wstring ConvertToWideString(const std::string& str);
}

#endif // __UI_UTILS_H__