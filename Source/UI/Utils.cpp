#include "Utils.h"

#include <codecvt>

using namespace UI;

std::string Utils::ConvertToUTF8(const std::wstring& wStr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    
    return converter.to_bytes(wStr);
}

std::wstring Utils::ConvertToWideString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    
    return converter.from_bytes(str);
}