#include "UI/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UI::stOptions options;

#ifndef NDEBUG
    options.devTools = true;
    options.url = "http://localhost:5050";
#endif
    options.resize = false;

    UI::CApplication app("WebView2-Desktop-Vue", { 330, 380 }, options);

    return app.Run();
}