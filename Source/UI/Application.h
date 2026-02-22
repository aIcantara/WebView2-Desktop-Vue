#ifndef __UI_APPLICATION_H__
#define __UI_APPLICATION_H__

#include "Structs.h"

#include <WebView2.h>

#include <wil/com.h>

namespace UI
{
    class CApplication
    {
    public:
        CApplication(const std::string& name, stSize size, stOptions options = {});
        ~CApplication() = default;

        int Run();

        void Exit();
        void Minimize();
        void Maximize();
        void SetSize(stSize size);

    private:
        stSize m_size;
        stOptions m_options;

        std::string m_name;

        HWND m_hWnd;

        wil::com_ptr<ICoreWebView2> m_pWebView;
        wil::com_ptr<ICoreWebView2Controller> m_pWebViewController;

        bool Initialize();

        HRESULT OnMessage(ICoreWebView2* pWebView, ICoreWebView2WebMessageReceivedEventArgs* pArgs);
        HRESULT OnResource(ICoreWebView2* pWebView, ICoreWebView2WebResourceRequestedEventArgs* pArgs);

        static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        std::string GetMime(const std::string& path);
    };
};

#endif // __UI_APPLICATION_H__