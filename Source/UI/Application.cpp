#include "Application.h"

#include <WebView2EnvironmentOptions.h>

#include <wrl.h>

#include <cmrc/cmrc.hpp>

#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_core.h>

#define WM_WEBVIEW_REQUEST WM_USER + 0x01

using namespace UI;

CMRC_DECLARE(UI_RESOURCES);

CApplication::CApplication(const std::string& name, stSize size, stOptions options) : m_name(name), m_size(size), m_options(options)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = name.c_str();
    wc.hIcon = LoadIcon(wc.hInstance, "APP_ICON");
    wc.hIconSm = LoadIcon(wc.hInstance, "APP_ICON");

    RegisterClassExA(&wc);
}

int CApplication::Run()
{
    stSize windowSize((GetSystemMetrics(SM_CXSCREEN) - m_size.width) / 2, (GetSystemMetrics(SM_CYSCREEN) - m_size.height) / 2);

    DWORD flags =
        (m_options.title ? WS_CAPTION : 0) |
        (m_options.close ? WS_SYSMENU : 0) |
        (m_options.minimize ? WS_MINIMIZEBOX : 0) |
        (m_options.maximize ? WS_MAXIMIZEBOX : 0) |
        (m_options.resize ? WS_THICKFRAME : 0);

    if (!m_options.frame)
    {
        flags = WS_POPUP;
    }

    m_hWnd = CreateWindowA(
        m_name.c_str(),
        m_name.c_str(),
        flags,
        windowSize.width,
        windowSize.height,
        m_size.width,
        m_size.height,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr
    );

    if (m_options.borderRadius > 0)
    {
        SetWindowRgn(
            m_hWnd,
            CreateRoundRectRgn(
                0,
                0,
                m_size.width + 1,
                m_size.height + 1,
                m_options.borderRadius,
                m_options.borderRadius
            ),
            TRUE
        );
    }

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    if (Initialize())
    {
        MSG message;

        while (GetMessage(&message, nullptr, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        return (int)message.wParam;
    }
    else
    {
        MessageBoxA(
            0,
            "WebView2 Initialize Error",
            m_name.c_str(),
            MB_OK | MB_ICONERROR
        );
    }

    return 1;
}

void CApplication::SetMessageHandler(MessageHandlerFunction handler)
{
    m_messageHandler = handler;
}

void CApplication::Exit()
{
    PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void CApplication::Minimize()
{
    ShowWindow(m_hWnd, SW_MINIMIZE);
}

void CApplication::Maximize()
{
    ShowWindow(m_hWnd, SW_MAXIMIZE);
}

void CApplication::SetSize(stSize size)
{
    m_size = size;

    SetWindowPos(
        m_hWnd,
        nullptr,
        0,
        0,
        m_size.width,
        m_size.height,
        SWP_NOMOVE | SWP_NOZORDER
    );

    if (m_options.borderRadius > 0)
    {
        SetWindowRgn(
            m_hWnd,
            CreateRoundRectRgn(
                0,
                0,
                m_size.width + 1,
                m_size.height + 1,
                m_options.borderRadius,
                m_options.borderRadius
            ),
            TRUE
        );
    }
}

bool CApplication::Initialize()
{
    static bool initialized = false;

    if (initialized)
    {
        return true;
    }

    char userData[MAX_PATH];

    SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, userData);

    auto pOptions = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();

    pOptions->put_AdditionalBrowserArguments(
        L"--enable-features=msWebView2EnableDraggableRegions "
        L"--disable-web-security "
        L"--disk-cache-size=1 "
        L"--media-cache-size=1 "
        L"--disable-extensions "
        L"--disable-background-networking "
        L"--disable-sync "
        L"--disable-features=TranslateUI"
    );

    CreateCoreWebView2EnvironmentWithOptions(nullptr, std::wstring(std::begin(userData), std::end(userData)).c_str(), pOptions.Get(),
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [&](HRESULT result, ICoreWebView2Environment* pEnvironment) -> HRESULT
            {
                if (FAILED(result) || pEnvironment == nullptr)
                {
                    return S_OK;
                }

                initialized = true;

                pEnvironment->CreateCoreWebView2Controller(m_hWnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [&](HRESULT result, ICoreWebView2Controller* pController) -> HRESULT
                        {
                            m_pWebViewController = pController;

                            m_pWebViewController->get_CoreWebView2(&m_pWebView);

                            wil::com_ptr<ICoreWebView2Settings> settings;

                            m_pWebView->get_Settings(&settings);

                            settings->put_AreDevToolsEnabled(m_options.devTools);
                            settings->put_AreDefaultContextMenusEnabled(m_options.devTools);

                            m_pWebView->add_WebMessageReceived(
                                Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [this](ICoreWebView2* pWebView, ICoreWebView2WebMessageReceivedEventArgs* pArgs)
                                    {
                                        return OnMessage(pWebView, pArgs);
                                    }
                                ).Get(),
                                nullptr
                            );
                            m_pWebView->AddWebResourceRequestedFilter(L"*", COREWEBVIEW2_WEB_RESOURCE_CONTEXT_ALL);
                            m_pWebView->add_WebResourceRequested(
                                Microsoft::WRL::Callback<ICoreWebView2WebResourceRequestedEventHandler>(
                                    [this](ICoreWebView2* pWebView, ICoreWebView2WebResourceRequestedEventArgs* pArgs)
                                    {
                                        return OnResource(pWebView, pArgs);
                                    }
                                ).Get(),
                                nullptr
                            );

                            RECT bounds;

                            GetClientRect(m_hWnd, &bounds);

                            m_pWebViewController->put_Bounds(bounds);

                            if (m_options.backgroundTransparent)
                            {
                                m_pWebViewController.query<ICoreWebView2Controller2>()->put_DefaultBackgroundColor({ 0 });
                            }

                            if (m_options.url.empty())
                            {
                                auto index = cmrc::UI_RESOURCES::get_filesystem().open("index.html");
                                std::wstring content(index.begin(), index.end());

                                m_pWebView->NavigateToString(content.c_str());
                            }
                            else
                            {
                                std::wstring url(m_options.url.begin(), m_options.url.end());

                                m_pWebView->Navigate(url.c_str());
                            }

                            return S_OK;
                        }
                    ).Get()
                );

                return S_OK;
            }
        ).Get()
    );

    return initialized;
}

HRESULT CApplication::OnMessage(ICoreWebView2* pWebView, ICoreWebView2WebMessageReceivedEventArgs* pArgs)
{
    if (m_messageHandler)
    {
        wil::unique_cotaskmem_string raw;

        pArgs->get_WebMessageAsJson(&raw);

        nlohmann::json message = nlohmann::json::parse(Utils::ConvertToUTF8(raw.get()));
        nlohmann::json args = message["args"];
        std::string name = message["name"];

        m_messageHandler(this, pWebView, name, args);
    }

    return S_OK;
}

HRESULT CApplication::OnResource(ICoreWebView2* pWebView, ICoreWebView2WebResourceRequestedEventArgs* pArgs)
{
    wil::com_ptr<ICoreWebView2WebResourceRequest> pRequest;

    pArgs->get_Request(&pRequest);

    wil::unique_cotaskmem_string uri;

    pRequest->get_Uri(&uri);

    std::string source = Utils::ConvertToUTF8(uri.get());

    auto prefix = source.find("res://");

    if (prefix != std::string::npos)
    {
        auto resources = cmrc::UI_RESOURCES::get_filesystem();

        source = source.substr(prefix + 6);

        if (resources.exists(source))
        {
            auto file = resources.open(source);
            std::string content(file.begin(), file.end());

            wil::com_ptr<IStream> pStream(SHCreateMemStream(reinterpret_cast<const BYTE*>(content.data()), content.size()));

            wil::com_ptr<ICoreWebView2_2> pWebView2;

            pWebView->QueryInterface(IID_PPV_ARGS(&pWebView2));

            wil::com_ptr<ICoreWebView2Environment> pEnvironment;

            pWebView2->get_Environment(&pEnvironment);

            std::string mime = GetMime(source);
            std::wstring wMime(mime.begin(), mime.end());

            wil::com_ptr<ICoreWebView2WebResourceResponse> pResponse;

            pEnvironment->CreateWebResourceResponse(pStream.get(), 200, L"OK", wMime.c_str(), &pResponse);

            wil::com_ptr<ICoreWebView2HttpResponseHeaders> pHeaders;

            pResponse->get_Headers(&pHeaders);

            std::wstring contentLength = std::to_wstring(content.size());

            pHeaders->AppendHeader(L"Content-Type", wMime.c_str());
            pHeaders->AppendHeader(L"Content-Length", contentLength.c_str());
            pHeaders->AppendHeader(L"Access-Control-Allow-Origin", L"*");

            pArgs->put_Response(pResponse.get());
        }
    }

    return S_OK;
}

LRESULT CALLBACK CApplication::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CApplication* pApp = reinterpret_cast<CApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (!pApp && uMsg != WM_CREATE)
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
        case WM_CLOSE:
        {
            DestroyWindow(hWnd);

            return 0;
        }

        case WM_DESTROY:
        {
            if (pApp->m_pWebViewController)
            {
                pApp->m_pWebViewController->Close();
                
                pApp->m_pWebViewController = nullptr;
                pApp->m_pWebView = nullptr;
            }

            PostQuitMessage(0);

            break;
        }

        case WM_SIZE:
        {
            if (pApp->m_pWebViewController)
            {
                RECT bounds;

                GetClientRect(hWnd, &bounds);

                pApp->m_pWebViewController->put_Bounds(bounds);
            }

            break;
        }

        case WM_WEBVIEW_REQUEST:
        {
            auto pRequest = reinterpret_cast<std::shared_ptr<std::string>*>(lParam);

            if (pRequest)
            {
                std::string request = **pRequest;

                pRequest->reset();

                wil::com_ptr<ICoreWebView2> pWebView;

                pApp->m_pWebViewController->get_CoreWebView2(&pWebView);

                if (pWebView)
                {
                    std::wstring wRequest(request.begin(), request.end());

                    pWebView->ExecuteScript(wRequest.c_str(), nullptr);
                }
            }

            break;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

std::string CApplication::GetMime(const std::string& path)
{
    static const std::unordered_map<std::string, std::string> mimeMap =
    {
        // images
        { "gif", "image/gif" },
        { "svg", "image/svg+xml" },
        { "png", "image/png" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "webp", "image/webp" },

        // video
        { "mp4", "video/mp4" },
        { "avi", "video/x-msvideo" },
        { "mov", "video/quicktime" },
        { "mkv", "video/x-matroska" },
        { "webm", "video/webm" },
        { "wmv", "video/x-ms-wmv" },
        { "mpeg", "video/mpeg" },
        { "mpg", "video/mpeg" },

        // text
        { "txt", "text/plain" },
        { "xml", "application/xml" },
        { "json", "application/json" },
        { "css", "text/css" },
        { "js", "application/javascript" },
        { "html", "text/html" },

        // audio
        { "mp3", "audio/mpeg" },
        { "aac", "audio/aac" },
        { "ogg", "audio/ogg" },
        { "flac", "audio/flac" },
        { "wma", "audio/x-ms-wma" },
        { "alac", "audio/alac" },
        { "opus", "audio/opus" },
        { "wav", "audio/wav" },
        { "aiff", "audio/aiff" },
        { "m4a", "audio/mp4" },
        { "webm", "audio/webm" },

        // fonts
        { "ttf", "font/truetype" },
        { "otf", "application/font-otf" },
        { "eot", "application/vnd.ms-fontobject" },
        { "woff", "font/woff" },
        { "woff2", "font/woff2" },
        { "sfnt", "font/sfnt" },
        { "woff3", "font/woff3" }
    };

    auto pos = path.find_last_of('.');

    if (pos == std::string::npos)
    {
        return "application/octet-stream";
    }

    std::string extension = path.substr(pos + 1);

    for (auto& c : extension)
    {
        c = static_cast<char>(tolower(c));
    }

    auto it = mimeMap.find(extension);

    if (it != mimeMap.end())
    {
        return it->second;
    }

    return "application/octet-stream";
}