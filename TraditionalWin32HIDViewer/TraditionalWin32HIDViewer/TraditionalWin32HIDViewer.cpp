#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <dbt.h>
#include "HIDInfoExtractor.h"

HIDInfoExtractor extractor;

// This GUID is for all USB serial host PnP drivers, but you can replace it 
// with any valid device class guid.
GUID wceusbsh_guid = { 0x25dbce51, 0x6c8f, 0x4a72,
                      0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };

// For informational messages and window titles.
PWSTR g_psz_app_name;

void Output_Message(
    HWND h_out_wnd,
    WPARAM w_param,
    LPARAM l_param
)
// Routine Description:
//     Support routine.
//     Send text to the output window, scrolling if necessary.

// Parameters:
//     hOutWnd - Handle to the output window.
//     wParam  - Standard windows message code, not used.
//     lParam  - String message to send to the window.

// Return Value:
//     None

// Note:
//     This routine assumes the output window is an edit control
//     with vertical scrolling enabled.

//     This routine has no error-checking.
{
    LRESULT   l_result;
    LONG      buffer_len;
    LONG      num_lines;
    LONG      first_vis;

    // Make writable and turn off redraw.
    l_result = SendMessage(h_out_wnd, EM_SETREADONLY, FALSE, 0L);
    l_result = SendMessage(h_out_wnd, WM_SETREDRAW, FALSE, 0L);

    // Obtain current text length in the window.
    buffer_len = SendMessage(h_out_wnd, WM_GETTEXTLENGTH, 0, 0L);
    num_lines = SendMessage(h_out_wnd, EM_GETLINECOUNT, 0, 0L);
    first_vis = SendMessage(h_out_wnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
    l_result = SendMessage(h_out_wnd, EM_SETSEL, buffer_len, buffer_len);

    // Write the new text.
    l_result = SendMessage(h_out_wnd, EM_REPLACESEL, 0, l_param);

    // See whether scrolling is necessary.
    if (num_lines > (first_vis + 1))
    {
        int        line_len = 0;
        int        line_count = 0;
        int        char_pos;

        // Find the last nonblank line.
        num_lines--;
        while (!line_len)
        {
            char_pos = SendMessage(
                h_out_wnd, EM_LINEINDEX, (WPARAM)num_lines, 0L);
            line_len = SendMessage(
                h_out_wnd, EM_LINELENGTH, char_pos, 0L);
            if (!line_len)
                num_lines--;
        }
        // Prevent negative value finding min.
        line_count = num_lines - first_vis;
        line_count = (line_count >= 0) ? line_count : 0;

        // Scroll the window.
        l_result = SendMessage(
            h_out_wnd, EM_LINESCROLL, 0, (LPARAM)line_count);
    }

    // Done, make read-only and allow redraw.
    l_result = SendMessage(h_out_wnd, WM_SETREDRAW, TRUE, 0L);
    l_result = SendMessage(h_out_wnd, EM_SETREADONLY, TRUE, 0L);
}

void Error_Handler(
    LPCTSTR lpsz_function
)
// Routine Description:
//     Support routine.
//     Retrieve the system error message for the last-error code
//     and pop a modal alert box with usable info.

// Parameters:
//     lpszFunction - String containing the function name where 
//     the error occurred plus any other relevant data you'd 
//     like to appear in the output. 

// Return Value:
//     None

// Note:
//     This routine is independent of the other windowing routines
//     in this application and can be used in a regular console
//     application without modification.
{

    LPVOID lp_msg_buf;
    LPVOID lp_display_buf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lp_msg_buf,
        0, NULL);

    // Display the error message and exit the process.

    lp_display_buf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lp_msg_buf)
            + lstrlen((LPCTSTR)lpsz_function) + 40)
        * sizeof(TCHAR));
    if (!lp_display_buf) return;
    StringCchPrintf((LPTSTR)lp_display_buf,
        LocalSize(lp_display_buf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpsz_function, dw, (LPCTSTR)lp_msg_buf);
    MessageBox(NULL, (LPCTSTR)lp_display_buf, g_psz_app_name, MB_OK);

    LocalFree(lp_msg_buf);
    LocalFree(lp_display_buf);
}

BOOL Register_Device_Interface_To_Hwnd(
    IN GUID Interface_class_guid,
    IN HWND hwnd,
    OUT HDEVNOTIFY* h_device_notify
)
// Routine Description:
//     Registers an HWND for notification of changes in the device interfaces
//     for the specified interface class GUID. 

// Parameters:
//     InterfaceClassGuid - The interface class GUID for the device 
//         interfaces. 

//     hWnd - Window handle to receive notifications.

//     hDeviceNotify - Receives the device notification handle. On failure, 
//         this value is NULL.

// Return Value:
//     If the function succeeds, the return value is TRUE.
//     If the function fails, the return value is FALSE.

// Note:
//     RegisterDeviceNotification also allows a service handle be used,
//     so a similar wrapper function to this one supporting that scenario
//     could be made from this template.
{
    DEV_BROADCAST_DEVICEINTERFACE notification_filter;

    ZeroMemory(&notification_filter, sizeof(notification_filter));
    notification_filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    notification_filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    notification_filter.dbcc_classguid = Interface_class_guid;

    *h_device_notify = RegisterDeviceNotification(
        hwnd,                       // events recipient
        &notification_filter,        // type of device
        DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
    );

    if (NULL == *h_device_notify)
    {
        Error_Handler(L"RegisterDeviceNotification");
        return FALSE;
    }

    return TRUE;
}

void Pump_Message(
    HWND hwnd
)
// Routine Description:
//     Simple main thread message pump.
//

// Parameters:
//     hWnd - handle to the window whose messages are being dispatched

// Return Value:
//     None.
{
    MSG msg;
    int ret_val;

    // Get all messages for any window that belongs to this thread,
    // without any filtering. Potential optimization could be
    // obtained via use of filter values if desired.

    while ((ret_val = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (ret_val == -1)
        {
            Error_Handler(L"GetMessage");
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void Print_HID(HWND hwnd, WPARAM w_param, bool init_print)
{
    if (extractor.Extract_HID_Info() == 0)
    {
        std::string info_string = extractor.Get_HID_Info_String();
        std::wstring info_wstring(info_string.begin(), info_string.end());

        if (init_print)
        {
            SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)(info_wstring.c_str()));
        }
        else
        {
            Output_Message(hwnd, w_param, (LPARAM)(info_wstring.c_str()));
        }
    }
    else
    {
        SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)TEXT("Extract HID info error...\r\n\r\n"));
    }
}

INT_PTR WINAPI Win_Proc_Callback(
    HWND hwnd,
    UINT message,
    WPARAM w_param,
    LPARAM l_param
)
// Routine Description:
//     Simple Windows callback for handling messages.
//     This is where all the work is done because the example
//     is using a window to process messages. This logic would be handled 
//     differently if registering a service instead of a window.

// Parameters:
//     hWnd - the window handle being registered for events.

//     message - the message being interpreted.

//     wParam and lParam - extended information provided to this
//          callback by the message sender.

//     For more information regarding these parameters and return value,
//     see the documentation for WNDCLASSEX and CreateWindowEx.
{
    LRESULT l_ret = 1;
    static HDEVNOTIFY h_device_notify;
    static HWND h_edit_wnd;
    static ULONGLONG msg_count = 0;

    switch (message)
    {
    case WM_CREATE:
        //
        // This is the actual registration., In this example, registration 
        // should happen only once, at application startup when the window
        // is created.
        //
        // If you were using a service, you would put this in your main code 
        // path as part of your service initialization.
        //
        if (!Register_Device_Interface_To_Hwnd(
            wceusbsh_guid,
            hwnd,
            &h_device_notify))
        {
            // Terminate on failure.
            Error_Handler(L"DoRegisterDeviceInterfaceToHwnd");
            ExitProcess(1);
        }

        //
        // Make the child window for output.
        //
        h_edit_wnd = CreateWindow(TEXT("EDIT"),// predefined class 
            NULL,        // no window title 
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            0, 0, 0, 0,  // set size in WM_SIZE message 
            hwnd,        // parent window 
            (HMENU)1,    // edit control ID 
            (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
            NULL);       // pointer not needed 

        if (h_edit_wnd == NULL)
        {
            // Terminate on failure.
            Error_Handler(L"CreateWindow: Edit Control");
            ExitProcess(1);
        }
        
        Print_HID(h_edit_wnd, w_param, true);

        break;

    case WM_SETFOCUS:
        SetFocus(h_edit_wnd);

        break;

    case WM_SIZE:
        // Make the edit control the size of the window's client area. 
        MoveWindow(h_edit_wnd,
            0, 0,                  // starting x- and y-coordinates 
            LOWORD(l_param),        // width of client area 
            HIWORD(l_param),        // height of client area 
            TRUE);                 // repaint window 

        break;

    case WM_DEVICECHANGE:
    {
        //
        // This is the actual message from the interface via Windows messaging.
        // This code includes some additional decoding for this particular device type
        // and some common validation checks.
        //
        // Note that not all devices utilize these optional parameters in the same
        // way. Refer to the extended information for your particular device type 
        // specified by your GUID.
        //
        PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE)l_param;
        TCHAR str_buff[256];

        // Output some messages to the window.
        switch (w_param)
        {
        case DBT_DEVICEARRIVAL:
            msg_count++;
            StringCchPrintf(
                str_buff, 256,
                TEXT("Message %d: DBT_DEVICEARRIVAL\n"), (int)msg_count);

            break;
        case DBT_DEVICEREMOVECOMPLETE:
            msg_count++;
            StringCchPrintf(
                str_buff, 256,
                TEXT("Message %d: DBT_DEVICEREMOVECOMPLETE\n"), (int)msg_count);

            break;
        case DBT_DEVNODES_CHANGED:
            msg_count++;
            StringCchPrintf(
                str_buff, 256,
                TEXT("Message %d: DBT_DEVNODES_CHANGED\n"), (int)msg_count);

            break;
        default:
            msg_count++;
            StringCchPrintf(
                str_buff, 256,
                TEXT("Message %d: WM_DEVICECHANGE message received, value %d unhandled.\n"),
                (int)msg_count, w_param);

            break;
        }

        Output_Message(h_edit_wnd, w_param, (LPARAM)str_buff);
        Print_HID(h_edit_wnd, w_param, false);

        break;
    }  
    case WM_CLOSE:
        if (!UnregisterDeviceNotification(h_device_notify))
        {
            Error_Handler(L"UnregisterDeviceNotification");
        }
        DestroyWindow(hwnd);

        break;

    case WM_DESTROY:
        PostQuitMessage(0);

        break;

    default:
        // Send all other messages on to the default windows handler.
        l_ret = DefWindowProc(hwnd, message, w_param, l_param);

        break;
    }

    return l_ret;
}

#define WND_CLASS_NAME TEXT("SampleAppWindowClass")

BOOL Init_Window_Class()
// Routine Description:
//      Simple wrapper to initialize and register a window class.

// Parameters:
//     None

// Return Value:
//     TRUE on success, FALSE on failure.

// Note: 
//     wndClass.lpfnWndProc and wndClass.lpszClassName are the
//     important unique values used with CreateWindowEx and the
//     Windows message pump.
{
    WNDCLASSEX wnd_class;

    wnd_class.cbSize = sizeof(WNDCLASSEX);
    wnd_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wnd_class.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
    wnd_class.lpfnWndProc = reinterpret_cast<WNDPROC>(Win_Proc_Callback);
    wnd_class.cbClsExtra = 0;
    wnd_class.cbWndExtra = 0;
    wnd_class.hIcon = LoadIcon(0, IDI_APPLICATION);
    wnd_class.hbrBackground = CreateSolidBrush(RGB(192, 192, 192));
    wnd_class.hCursor = LoadCursor(0, IDC_ARROW);
    wnd_class.lpszClassName = WND_CLASS_NAME;
    wnd_class.lpszMenuName = NULL;
    wnd_class.hIconSm = wnd_class.hIcon;


    if (!RegisterClassEx(&wnd_class))
    {
        Error_Handler(L"RegisterClassEx");
        return FALSE;
    }
    return TRUE;
}

int __stdcall _tWinMain(
    _In_ HINSTANCE h_instance_exe,
    _In_opt_ HINSTANCE, // should not reference this parameter
    _In_ PTSTR lpstr_cmd_line,
    _In_ int n_cmd_show)
{
    //
    // To enable a console project to compile this code, set
    // Project->Properties->Linker->System->Subsystem: Windows.
    //

    int n_arg_c = 0;
    PWSTR* pp_arg_v = CommandLineToArgvW(lpstr_cmd_line, &n_arg_c);
    g_psz_app_name = pp_arg_v[0];

    if (!Init_Window_Class())
    {
        // InitWindowClass displays any errors
        return -1;
    }

    // Main app window

    HWND hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE | WS_EX_APPWINDOW,
        WND_CLASS_NAME,
        g_psz_app_name,
        WS_OVERLAPPEDWINDOW, // style
        CW_USEDEFAULT, 0,
        1280, 720,
        NULL, NULL,
        h_instance_exe,
        NULL);

    if (hwnd == NULL)
    {
        Error_Handler(L"CreateWindowEx: main appwindow hWnd");
        return -1;
    }

    // Actually draw the window.
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    // The message pump loops until the window is destroyed.
    Pump_Message(hwnd);

    return 1;
}