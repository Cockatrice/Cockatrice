#include "close_holders.h"

#include "restart_manager_holders.h"

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
/// How long one poll of the asked processes takes. One WaitForSingleObject per process
/// and no enumeration, so polling costs nothing next to the seconds this is called on.
constexpr unsigned long WAIT_POLL_MILLISECONDS = 200;

/// How long the report line may get before it is cut. NSIS holds one string in 1024
/// characters, and this one is written there verbatim; every holder is named, so normally
/// the line stays far below it.
constexpr std::size_t REPORT_CHAR_MAX = 900;

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

/// Whether a window is one the user could be asked to answer.
///
/// Only a framed, non-empty window qualifies. Qt keeps hidden top-level windows of its own
/// next to the application's: the tray icon creates one to receive the TaskbarCreated
/// message, styled WS_OVERLAPPED - which is no frame at all - and titled
/// QTrayIconMessageWindow. Showing one of those puts an empty white rectangle on the
/// taskbar that nothing ever closes again: it has no window procedure of its own to hide it,
/// and the process is busy in the loop this is meant to break, so it stays until the process
/// dies. Posting a close to one is worse than useless too: Qt's tray window answers WM_CLOSE
/// by asking the application to terminate, without going through the confirmation the close
/// is supposed to reach.
bool isAnswerableWindow(HWND window)
{
    if ((GetWindowLongPtr(window, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) != 0) {
        return false;
    }
    if ((GetWindowLongPtr(window, GWL_STYLE) & WS_CAPTION) == 0) {
        return false;
    }
    RECT bounds{};
    if (GetWindowRect(window, &bounds) == FALSE) {
        return false;
    }
    return bounds.right > bounds.left && bounds.bottom > bounds.top;
}

/// Picks the window to post the close to and the one the user should end up looking at.
struct CloseTarget
{
    HWND main = nullptr;    ///< first visible window without an owner: the application's own
    HWND toRaise = nullptr; ///< topmost visible window: a dialog when one is up
    bool modalOpen = false; ///< the only window a user could answer is the instance's own question
};

/// Only the windows the user could answer are candidates - see isAnswerableWindow. An
/// instance left with nothing but Qt's own bookkeeping windows visible has no window to ask:
/// the caller reports that instead of posting a close the application would answer with a
/// terminate that skips its own confirmation.
CloseTarget chooseCloseTarget(const std::vector<HWND> &windows)
{
    CloseTarget target;
    HWND topmost = nullptr;
    for (const HWND window : windows) {
        if (IsWindowVisible(window) == FALSE || !isAnswerableWindow(window)) {
            continue;
        }
        if (topmost == nullptr) {
            topmost = window;
        }
        if (target.main == nullptr && GetWindow(window, GW_OWNER) == nullptr) {
            target.main = window;
        }
    }
    target.toRaise = topmost != nullptr ? topmost : target.main;
    // A modal dialog disables the windows behind it, so a disabled main window is the
    // instance asking a question of its own. A visible window with no unowned main window
    // next to it is one too: what is on screen is then the question itself - the window
    // behind it is hidden, not gone - and there is no window a close could be posted to
    // without hiding the question with it.
    target.modalOpen = target.toRaise != nullptr && (target.main == nullptr || IsWindowEnabled(target.main) == FALSE);
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

unsigned int installerQuitRequestMessage()
{
    // Must match the name the client registers in cockatrice/src/main.cpp.
    return RegisterWindowMessageW(L"Cockatrice.InstallerQuitRequest");
}

bool isCockatriceImage(const std::wstring &imageName)
{
    constexpr wchar_t COCKATRICE_IMAGE[] = L"cockatrice.exe";
    return CompareStringOrdinal(imageName.c_str(), -1, COCKATRICE_IMAGE, -1, TRUE) == CSTR_EQUAL;
}

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
            ++result.withoutWindow;
            lines.push_back(processLabel(holder) + L" has no window to close, left to you");
            continue;
        }

        const CloseTarget target = chooseCloseTarget(search.windows);
        if (target.toRaise == nullptr) {
            // Nothing visible that a user could answer: the instance is hidden - in the
            // notification area, say - or has nothing but Qt's own bookkeeping windows on
            // screen. Showing one of those from here does not help, and WM_CLOSE to Qt's tray
            // window terminates without the confirmation. Instead the instance is sent the
            // installer quit request: a registered message Cockatrice answers by showing its own
            // main window and closing it the normal way, confirmations included. Any of its
            // windows reaches the same GUI-thread filter, so the first one that accepts the post
            // is enough.
            const UINT quitRequest = installerQuitRequestMessage();
            const HANDLE process = quitRequest != 0 ? OpenProcess(SYNCHRONIZE, FALSE, holder.processId) : nullptr;
            const DWORD openError = process == nullptr ? GetLastError() : ERROR_SUCCESS;
            bool posted = false;
            DWORD postError = ERROR_SUCCESS;
            if (quitRequest != 0) {
                for (const HWND window : search.windows) {
                    if (PostMessageW(window, quitRequest, 0, 0) != FALSE) {
                        posted = true;
                        break;
                    }
                    postError = GetLastError();
                }
            }
            if (!posted) {
                if (process != nullptr) {
                    CloseHandle(process);
                }
                ++result.withoutWindow;
                std::wstring line = processLabel(holder);
                line += L" has no window to ask, and the quit request could not be sent (win32 ";
                line += std::to_wstring(postError);
                line += L"), left to you";
                lines.push_back(std::move(line));
                continue;
            }
            ++result.askedToClose;
            std::wstring line = processLabel(holder) + L" was hidden and sent the installer quit request";
            if (process == nullptr) {
                line += L", but its exit could not be watched (win32 " + std::to_wstring(openError) + L")";
            } else {
                askedLines.push_back(lines.size());
                asked.push_back(process);
                // No window to describe afterwards: describeWindowState reports a null window as gone.
                askedWindows.push_back(nullptr);
            }
            lines.push_back(std::move(line));
            continue;
        }
        std::wstring line = processLabel(holder);
        line += bringForward(target.toRaise) ? L" brought forward" : L" could not be brought forward";

        // Posting the close to an instance that is asking a question of its own is what made
        // an earlier attempt useless: the client accepts it, hides this window and the dialog
        // with it, and stays alive inside the nested loop the dialog opened, where nothing can
        // reach it any more. The question is the request now, so it is brought forward and
        // left alone - including a question whose owner window is hidden, which is the only
        // thing left on screen and has no unowned window a close could be posted to.
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

EndHoldersResult endHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds)
{
    EndHoldersResult result;
    std::vector<std::wstring> lines;
    std::vector<HANDLE> ended;
    std::vector<std::size_t> endedLines;

    for (const Holder &holder : holders) {
        if (!isPayloadProgram(holder.imageName)) {
            // Not one of the programs the payload installs: a scanner that has a DLL open, or
            // a program the user keeps in the install directory. It is named and left to the
            // user, because ending it would destroy something this installer does not own.
            lines.push_back(processLabel(holder) + L" left to you");
            continue;
        }

        // Services included: the installer asked before getting here, and the Service
        // Control Manager reports the terminated process as a stopped service afterwards.
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
