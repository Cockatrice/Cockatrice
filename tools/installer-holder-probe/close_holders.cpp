#include "close_holders.h"

// clang-format off
// windows.h has to come before anything that pulls in a Windows type, as everywhere in
// this tool.
#include <windows.h>
// clang-format on

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr wchar_t COCKATRICE_IMAGE[] = L"cockatrice.exe";

/// How long one poll of the asked processes takes. One WaitForSingleObject per process
/// and no enumeration, so polling costs nothing next to the seconds this is called on.
constexpr unsigned long WAIT_POLL_MILLISECONDS = 200;

/// How long the report line may get before it is cut. NSIS holds one string in 1024
/// characters, and this one is written there verbatim; every holder is named, so normally
/// the line stays far below it.
constexpr std::size_t REPORT_CHAR_MAX = 900;

bool isCockatriceImage(const std::wstring &imageName)
{
    return CompareStringOrdinal(imageName.c_str(), -1, COCKATRICE_IMAGE, -1, TRUE) == CSTR_EQUAL;
}

/// The holder as the log has to name it to be actionable. Deliberately not
/// formatHolder(): that one is worded for the dialog, where ", running" says something
/// the line here continues after.
std::wstring processLabel(const Holder &holder)
{
    std::wstring label = holder.imageName.empty() ? holder.appName : holder.imageName;
    if (label.empty()) {
        label = L"an unnamed process";
    }
    label += L" (pid ";
    label += std::to_wstring(holder.processId);
    label += L")";
    return label;
}

struct WindowSearch
{
    DWORD processId = 0;
    HWND topmost = nullptr; ///< first visible window in Z-order: a dialog when one is up
    HWND main = nullptr;    ///< first visible window without an owner: the one to close
    HWND any = nullptr;     ///< first window at all, used when nothing is visible
};

BOOL CALLBACK findProcessWindow(HWND window, LPARAM parameter)
{
    auto &search = *reinterpret_cast<WindowSearch *>(parameter);
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (processId != search.processId) {
        return TRUE;
    }
    if (search.any == nullptr) {
        search.any = window;
    }
    // A tray icon leaves the main window hidden, so a window that is not visible is still
    // kept as a fallback. EnumWindows walks the Z-order top down, so the first visible
    // window is the one the user would see in front.
    if (IsWindowVisible(window) != FALSE) {
        if (search.topmost == nullptr) {
            search.topmost = window;
        }
        if (search.main == nullptr && GetWindow(window, GW_OWNER) == nullptr) {
            search.main = window;
        }
    }
    return TRUE;
}

/// Restores a minimized window and takes the foreground, so a confirmation Cockatrice
/// answers the close with is in front of the installer instead of behind it.
///
/// A process in the background may not take the foreground on its own. Attaching to the
/// thread that owns the foreground window lifts that restriction for the duration of the
/// call; detaching afterwards leaves the two input queues as they were.
bool bringForward(HWND window)
{
    if (IsIconic(window) != FALSE) {
        ShowWindow(window, SW_RESTORE);
    } else {
        ShowWindow(window, SW_SHOW);
    }

    const HWND foreground = GetForegroundWindow();
    const DWORD foregroundThread = foreground != nullptr ? GetWindowThreadProcessId(foreground, nullptr) : 0;
    const DWORD currentThread = GetCurrentThreadId();
    const bool attached = foregroundThread != 0 && foregroundThread != currentThread &&
                          AttachThreadInput(currentThread, foregroundThread, TRUE) != FALSE;
    BringWindowToTop(window);
    SetForegroundWindow(window);
    if (attached) {
        AttachThreadInput(currentThread, foregroundThread, FALSE);
    }
    return GetForegroundWindow() == window;
}

/// Waits for @p processes to signal, up to @p waitMilliseconds. Returns whether they all
/// exited and how long the wait took.
bool waitForExit(const std::vector<HANDLE> &processes, unsigned long waitMilliseconds, unsigned long long &elapsed)
{
    const ULONGLONG start = GetTickCount64();
    const ULONGLONG deadline = start + waitMilliseconds;
    for (;;) {
        bool allExited = true;
        for (const HANDLE process : processes) {
            if (WaitForSingleObject(process, 0) != WAIT_OBJECT_0) {
                allExited = false;
                break;
            }
        }
        elapsed = GetTickCount64() - start;
        if (allExited || GetTickCount64() >= deadline) {
            return allExited;
        }
        Sleep(WAIT_POLL_MILLISECONDS);
    }
}
} // namespace

CloseHoldersResult closeCockatriceHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds)
{
    CloseHoldersResult result;
    std::vector<std::wstring> lines;
    std::vector<HANDLE> asked;
    std::vector<std::size_t> askedLines;

    for (const Holder &holder : holders) {
        if (holder.isService || !isCockatriceImage(holder.imageName)) {
            // Oracle and Servatrice are named by the prompt and close by hand: Oracle
            // writes the card database in place. A service has no window to ask either.
            lines.push_back(processLabel(holder) + L" left to you");
            continue;
        }

        WindowSearch search;
        search.processId = holder.processId;
        EnumWindows(findProcessWindow, reinterpret_cast<LPARAM>(&search));
        if (search.any == nullptr) {
            lines.push_back(processLabel(holder) + L" has no window to close, left to you");
            continue;
        }

        const HWND target =
            search.main != nullptr ? search.main : (search.topmost != nullptr ? search.topmost : search.any);
        const HWND toRaise = search.topmost != nullptr ? search.topmost : target;
        std::wstring line = processLabel(holder);
        line += bringForward(toRaise) ? L" brought forward" : L" could not be brought forward";

        // Kept open across the wait so a pid reused after the process exits cannot be
        // mistaken for the instance that was asked to close.
        const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, holder.processId);
        const DWORD openError = process == nullptr ? GetLastError() : ERROR_SUCCESS;
        if (PostMessageW(target, WM_CLOSE, 0, 0) == FALSE) {
            // Read the error before CloseHandle, which is free to reset it.
            const DWORD postError = GetLastError();
            if (process != nullptr) {
                CloseHandle(process);
            }
            lines.push_back(line + L", and the close request could not be sent (win32 " + std::to_wstring(postError) +
                            L")");
            continue;
        }
        line += L" and asked to close";
        ++result.askedToClose;
        if (process == nullptr) {
            line += L", but its exit could not be watched (win32 " + std::to_wstring(openError) + L")";
        } else {
            askedLines.push_back(lines.size());
            asked.push_back(process);
        }
        lines.push_back(std::move(line));
    }

    if (!asked.empty() && waitMilliseconds > 0) {
        unsigned long long elapsed = 0;
        const bool allExited = waitForExit(asked, waitMilliseconds, elapsed);
        for (std::size_t index = 0; index < asked.size(); ++index) {
            const bool exited = allExited || WaitForSingleObject(asked[index], 0) == WAIT_OBJECT_0;
            lines[askedLines[index]] += exited ? L", exited after " + std::to_wstring(elapsed) + L" ms"
                                               : L", still running after " + std::to_wstring(elapsed) + L" ms";
        }
    }
    for (const HANDLE process : asked) {
        CloseHandle(process);
    }

    if (lines.empty()) {
        result.report = L"nothing is holding the files";
        return result;
    }
    for (const std::wstring &line : lines) {
        if (!result.report.empty()) {
            result.report += L"; ";
        }
        result.report += line;
    }
    if (result.report.size() > REPORT_CHAR_MAX) {
        result.report.erase(REPORT_CHAR_MAX);
        result.report += L"...";
    }
    return result;
}
