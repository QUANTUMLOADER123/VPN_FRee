#include <windows.h>
#include <shellapi.h>

#include <cwchar>

namespace {
constexpr int kIconResource = 101;
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kTrayId = 1;
constexpr UINT kExitCommand = 1001;
constexpr wchar_t kClassName[] = L"ChromeBackgroundDemo";

HINSTANCE instance;
HICON app_icon;
bool in_background = false;
UINT taskbar_created;

NOTIFYICONDATAW tray_data(HWND window) {
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = window;
    data.uID = kTrayId;
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    data.uCallbackMessage = kTrayMessage;
    data.hIcon = app_icon;
    constexpr wchar_t tooltip[] = L"Chrome - \u0444\u043e\u043d\u043e\u0432\u043e\u0435 \u0434\u0435\u043c\u043e-\u043f\u0440\u0438\u043b\u043e\u0436\u0435\u043d\u0438\u0435";
    std::wcsncpy(data.szTip, tooltip, sizeof(data.szTip) / sizeof(data.szTip[0]) - 1);
    return data;
}

bool add_tray_icon(HWND window) {
    auto data = tray_data(window);
    return Shell_NotifyIconW(NIM_ADD, &data) != FALSE;
}

void remove_tray_icon(HWND window) {
    auto data = tray_data(window);
    Shell_NotifyIconW(NIM_DELETE, &data);
}

void show_tray_menu(HWND window) {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;
    AppendMenuW(menu, MF_STRING, kExitCommand, L"\u0412\u044b\u0445\u043e\u0434");
    POINT position{};
    GetCursorPos(&position);
    SetForegroundWindow(window);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, position.x, position.y, 0, window, nullptr);
    PostMessageW(window, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (taskbar_created != 0 && message == taskbar_created && in_background) {
        add_tray_icon(window);
        return 0;
    }
    switch (message) {
    case WM_CREATE: {
        HWND icon = CreateWindowExW(0, L"STATIC", nullptr, WS_CHILD | WS_VISIBLE | SS_ICON,
                                    22, 22, 32, 32, window, nullptr, instance, nullptr);
        SendMessageW(icon, STM_SETICON, reinterpret_cast<WPARAM>(LoadIconW(nullptr, IDI_ERROR)), 0);
        HWND text = CreateWindowExW(0, L"STATIC", L"\u041f\u0440\u043e\u0433\u0440\u0430\u043c\u043c\u0430 \u0443\u0436\u0435 \u0443\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0430.",
                                    WS_CHILD | WS_VISIBLE, 72, 27, 300, 26,
                                    window, nullptr, instance, nullptr);
        HWND button = CreateWindowExW(0, L"BUTTON", L"OK",
                                      WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                      160, 85, 80, 26, window,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDOK)), instance, nullptr);
        auto font = reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT));
        SendMessageW(text, WM_SETFONT, font, TRUE);
        SendMessageW(button, WM_SETFONT, font, TRUE);
        SetFocus(button);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wparam) == IDOK || LOWORD(wparam) == kExitCommand) {
            DestroyWindow(window);
            return 0;
        }
        break;
    case WM_CLOSE:
        if (add_tray_icon(window)) {
            in_background = true;
            ShowWindow(window, SW_HIDE);
        } else {
            MessageBoxW(window, L"\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c \u0434\u043e\u0431\u0430\u0432\u0438\u0442\u044c \u0437\u043d\u0430\u0447\u043e\u043a \u0432 \u043e\u0431\u043b\u0430\u0441\u0442\u044c \u0443\u0432\u0435\u0434\u043e\u043c\u043b\u0435\u043d\u0438\u0439.", L"Chrome", MB_OK | MB_ICONERROR);
        }
        return 0;
    case kTrayMessage:
        if (lparam == WM_RBUTTONUP || lparam == WM_CONTEXTMENU) show_tray_menu(window);
        return 0;
    case WM_DESTROY:
        if (in_background) remove_tray_icon(window);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
} // namespace

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE, PWSTR, int show_command) {
    instance = hinstance;
    taskbar_created = RegisterWindowMessageW(L"TaskbarCreated");
    app_icon = LoadIconW(instance, MAKEINTRESOURCEW(kIconResource));

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hIcon = app_icon;
    window_class.hIconSm = app_icon;
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    window_class.lpszClassName = kClassName;
    if (!RegisterClassExW(&window_class)) return 1;

    constexpr int width = 400;
    constexpr int height = 170;
    RECT work_area{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0);
    int x = work_area.left + (work_area.right - work_area.left - width) / 2;
    int y = work_area.top + (work_area.bottom - work_area.top - height) / 2;
    HWND window = CreateWindowExW(WS_EX_DLGMODALFRAME, kClassName, L"Chrome",
                                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                                  x, y, width, height, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, show_command);
    UpdateWindow(window);

    MSG message{};
    BOOL result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
