#ifndef __UI_STRUCTS_H__
#define __UI_STRUCTS_H__

#include <string>

namespace UI
{
    struct stSize
    {
        int width;
        int height;
    };

    struct stOptions
    {
        bool devTools = false;
        bool backgroundTransparent = false;

        bool frame = true;
        bool title = true;
        bool close = true;
        bool minimize = true;
        bool maximize = true;
        bool resize = true;

        int borderRadius = 0;

        std::string url = "";
    };
};

#endif // __UI_STRUCTS_H__