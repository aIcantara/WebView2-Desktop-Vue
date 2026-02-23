#include "UI/Application.h"

#include <iostream>

void OnMessage(UI::CApplication* pApplication, ICoreWebView2* pWebView, const std::string& name, const nlohmann::json& args)
{
    if (name == "Console:Attach")
    {
        if (AllocConsole())
        {
            FILE* file;

            freopen_s(&file, "CONOUT$", "w", stdout);
            freopen_s(&file, "CONOUT$", "w", stderr);
            freopen_s(&file, "CONIN$", "r", stdin);
        }
    }
    else if (name == "Console:Print")
    {
        std::cout << args.at(0).get<std::string>() << std::endl;
    }
    else if (name == "Math:Sum")
    {
        int a = args.at(0).get<int>();
        int b = args.at(1).get<int>();

        std::string result = std::to_string(a + b);

        pWebView->PostWebMessageAsString(UI::Utils::ConvertToWideString(result).c_str());
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UI::stOptions options;

#ifndef NDEBUG
    options.devTools = true;
    options.url = "http://localhost:5050";
#endif
    options.resize = false;

    UI::CApplication app("WebView2-Desktop-Vue", { 330, 380 }, options);

    app.SetMessageHandler(OnMessage);

    return app.Run();
}