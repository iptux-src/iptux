#include "UIHelperWindows.h"
#include "iptux-utils/output.h"
#include <windows.h>
#include <shellapi.h>


void iptux_launch_file_windows(const char* utf8_path) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_path, -1, NULL, 0);
    wchar_t* w_path = (wchar_t*)malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8_path, -1, w_path, len);

    HINSTANCE result = ShellExecuteW(
        NULL,          // 父窗口句柄
        L"open",       // 操作类型：打开
        w_path,        // 文件路径
        NULL,          // 命令行参数
        NULL,          // 工作目录
        SW_SHOWNORMAL  // 显示方式
    );

    // 3. 释放内存
    free(w_path);

    // 检查是否成功
    if ((INT_PTR)result <= 32) {
      LOG_ERROR("Failed to open path: %s, error code: %jd", utf8_path, (intmax_t)(INT_PTR)result);
    }
}
