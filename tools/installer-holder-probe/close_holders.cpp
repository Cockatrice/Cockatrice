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

/// Joins the per-holder lines into the one line the installer logs and cuts it to what one
/// NSIS string holds.
std::wstring joinReport(const std::vector<std::wstring> &lines)
{
    std::wstring report;
    for (const std::wstring &line : lines) {
        if (!report.empty()) {
            report += L"; ";
        }
        report += line;
    }
    if (report.size() > REPORT_CHAR_MAX) {
        report.erase(REPORT_CHAR_MAX);
        report += L"...";
    }
    return report;
}

struct WindowSearch
{
    DWORD processId = 0;
    std::vector<HWND> windows; ///< every top-level window of the process, in Z-order
};

BOOL CALLBACK collectProcessWindow(HWND window, LPARAM parameter)
{
    auto &search = *reinterpret_cast<WindowSearch *>(parameter);
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (processId == search.processId) {
        search.windows.push_back(window);
    }
    return TRUE;
}

/// Shows the windows an instance left hidden, and restores the ones it left minimized.
///
/// A window that exists but is not visible is not noise to skip: an instance that answered an
/// earlier close by hiding instead of quitting - which is what a shutdown stuck in a
/// confirmation of its own does, because accepting a second close hides the dialog together
/// with the window that owns it - has no visible window left, and the question it is waiting on
/// is hidden with it. Showing them again is what puts that question back in front of the user,
/// where answering it is the only way the instance can finish quitting.
///
/// Only tool windows are left alone: they are tooltips and popups, never a question.
unsigned int restoreHiddenWindows(const std::vector<HWND> &windows)
{
    unsigned int restored = 0;
    for (const HWND window : windows) {
        if ((GetWindowLongPtr(window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) != 0) {
            continue;
        }
        if (IsWindowVisible(window) == FALSE) {
            ShowWindow(window, SW_SHOW);
            if (IsWindowVisible(window) != FALSE) {
                ++restored;
            }
        } else if (IsIconic(window) != FALSE) {
            ShowWindow(window, SW_RESTORE);
        }
    }
    return restored;
}

/// Picks the window to post the close to and the one the user should end up looking at.
struct CloseTarget
{
    HWND main = nullptr;    ///< first visible window without an owner: the application's own
    HWND toRaise = nullptr; ///< topmost visible window: a dialog when one is up
    bool modalOpen = false; ///< the window a close would reach is disabled, so a dialog of its own is up
};

CloseTarget chooseCloseTarget(const std::vector<HWND> &windows)
{
    CloseTarget target;
    HWND topmost = nullptr;
    for (const HWND window : windows) {
        if (IsWindowVisible(window) == FALSE) {
            continue;
        }
        if (topmost == nullptr) {
            topmost = window;
        }
        if (target.main == nullptr && GetWindow(window, GW_OWNER) == nullptr) {
            target.main = window;
        }
    }
    target.toRaise = topmost != nullptr ? topmost : (target.main != nullptr ? target.main : windows.front());
    // A modal dialog disables the windows behind it, so a disabled main window is the
    // instance asking a question of its own.
    target.modalOpen = IsWindowEnabled(target.main != nullptr ? target.main : target.toRaise) == FALSE;
    return target;
}

/// What is left of the window the close was posted to, so a log line tells an instance that
/// answered by hiding from one that never answered at all. A process that hid its window and
/// stayed alive is the one that needs the next close to bring it back into view.
std::wstring describeWindowState(HWND window)
{
    if (window == nullptr || IsWindow(window) == FALSE) {
        return L", its window is gone";
    }
    // WM_NULL is the cheapest way to ask whether the thread behind the window still dispatches.
    // A hung instance does not answer at all, which is a different problem from one that
    // answered the close and stayed open, and the two want different words in the prompt.
    DWORD_PTR ignored = 0;
    const bool responding =
        SendMessageTimeoutW(window, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 1000, &ignored) != 0;
    std::wstring state = IsWindowVisible(window) != FALSE ? L", window still open" : L", window hidden again";
    if (!responding) {
        state += L" and not responding";
    }
    return state;
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
    std::vector<HWND> askedWindows;

    for (const Holder &holder : holders) {
        if (holder.isService || !isCockatriceImage(holder.imageName)) {
            // Oracle and Servatrice are named by the prompt and close by hand: Oracle
            // writes the card database in place. A service has no window to ask either.
            lines.push_back(processLabel(holder) + L" left to you");
            continue;
        }

        WindowSearch search;
        search.processId = holder.processId;
        EnumWindows(collectProcessWindow, reinterpret_cast<LPARAM>(&search));
        if (search.windows.empty()) {
            lines.push_back(processLabel(holder) + L" has no window to close, left to you");
            continue;
        }

        const unsigned int restored = restoreHiddenWindows(search.windows);
        const CloseTarget target = chooseCloseTarget(search.windows);
        std::wstring line = processLabel(holder);
        line += bringForward(target.toRaise) ? L" brought forward" : L" could not be brought forward";
        if (restored == 1) {
            line += L", restored a hidden window";
        } else if (restored > 1) {
            line += L", restored " + std::to_wstring(restored) + L" hidden windows";
        }

        // Posting the close to an instance that is asking a question of its own is what made
        // an earlier attempt useless: the client accepts it, hides this window and the dialog
        // with it, and stays alive inside the nested loop the dialog opened, where nothing can
        // reach it any more. The question is the request now, so it is brought forward and
        // left alone.
        if (target.modalOpen) {
            lines.push_back(line + L", and is waiting for an answer to a question of its own");
            continue;
        }

        // Kept open across the wait so a pid reused after the process exits cannot be
        // mistaken for the instance that was asked to close.
        const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, holder.processId);
        const DWORD openError = process == nullptr ? GetLastError() : ERROR_SUCCESS;
        if (PostMessageW(target.main, WM_CLOSE, 0, 0) == FALSE) {
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
            askedWindows.push_back(target.main);
        }
        lines.push_back(std::move(line));
    }

    if (!asked.empty() && waitMilliseconds > 0) {
        unsigned long long elapsed = 0;
        const bool allExited = waitForExit(asked, waitMilliseconds, elapsed);
        for (std::size_t index = 0; index < asked.size(); ++index) {
            const bool exited = allExited || WaitForSingleObject(asked[index], 0) == WAIT_OBJECT_0;
            if (exited) {
                lines[askedLines[index]] += L", exited after " + std::to_wstring(elapsed) + L" ms";
            } else {
                lines[askedLines[index]] += L", still running after " + std::to_wstring(elapsed) + L" ms" +
                                            describeWindowState(askedWindows[index]);
            }
        }
    }
    for (const HANDLE process : asked) {
        CloseHandle(process);
    }

    if (lines.empty()) {
        result.report = L"nothing is holding the files";
        return result;
    }
    result.report = joinReport(lines);
    return result;
}

EndHoldersResult endCockatriceHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds)
{
    EndHoldersResult result;
    std::vector<std::wstring> lines;
    std::vector<HANDLE> ended;
    std::vector<std::size_t> endedLines;

    for (const Holder &holder : holders) {
        if (holder.isService || !isCockatriceImage(holder.imageName)) {
            // The same split as the close: Oracle writes the card database in place and a
            // service has nothing to ask, so neither is ever ended here.
            lines.push_back(processLabel(holder) + L" left to you");
            continue;
        }

        const HANDLE process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, holder.processId);
        if (process == nullptr) {
            lines.push_back(processLabel(holder) + L" could not be ended (win32 " + std::to_wstring(GetLastError()) +
                            L")");
            continue;
        }
        if (TerminateProcess(process, 1) == FALSE) {
            // Read the error before CloseHandle, which is free to reset it.
            const DWORD error = GetLastError();
            CloseHandle(process);
            lines.push_back(processLabel(holder) + L" could not be ended (win32 " + std::to_wstring(error) + L")");
            continue;
        }
        ++result.ended;
        endedLines.push_back(lines.size());
        lines.push_back(processLabel(holder) + L" ended");
        ended.push_back(process);
    }

    if (!ended.empty() && waitMilliseconds > 0) {
        unsigned long long elapsed = 0;
        const bool allExited = waitForExit(ended, waitMilliseconds, elapsed);
        for (std::size_t index = 0; index < ended.size(); ++index) {
            const bool exited = allExited || WaitForSingleObject(ended[index], 0) == WAIT_OBJECT_0;
            lines[endedLines[index]] += exited ? L", exited after " + std::to_wstring(elapsed) + L" ms"
                                               : L", still running after " + std::to_wstring(elapsed) + L" ms";
        }
    }
    for (const HANDLE process : ended) {
        CloseHandle(process);
    }

    if (lines.empty()) {
        result.report = L"nothing is holding the files";
        return result;
    }
    result.report = joinReport(lines);
    return result;
}
