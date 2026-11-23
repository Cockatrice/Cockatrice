#include "close_holders.h"

#include "installer_close_request.h"
#include "payload_paths.h"
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
    HWND main = nullptr;    ///< first answerable window without an owner: the application's own
    HWND toRaise = nullptr; ///< topmost visible window: a dialog when one is up
    bool hidden = false;    ///< the window being closed is not on screen (notification area)
};

/// Only the windows the user could be asked to answer are candidates - see isAnswerableWindow.
///
/// A hidden window is a candidate like a visible one: WM_CLOSE does not need a window on
/// screen, so an instance that is only in the notification area answers it the way it answers
/// its own close button, and the confirmation it raises then is a window Qt shows itself -
/// painted, unlike one restored from outside. Nothing is shown from here. Qt's own bookkeeping
/// windows stay excluded, because a close posted to the notification-area one terminates the
/// instance without the confirmation and that is worse than no close at all.
///
/// @param windows every top-level window the process owns, in Z-order
/// @param askHidden when false, only a window that is on screen is a target: the launcher
///        mode reads "no window" as "past its own prompts and stuck", which stops being true
///        the moment a hidden window is asked to close
CloseTarget chooseCloseTarget(const std::vector<HWND> &windows, bool askHidden)
{
    CloseTarget target;
    for (const HWND window : windows) {
        if (!isAnswerableWindow(window)) {
            continue;
        }
        const bool visible = IsWindowVisible(window) != FALSE;
        if (visible && target.toRaise == nullptr) {
            target.toRaise = window;
        }
        if (target.main == nullptr && GetWindow(window, GW_OWNER) == nullptr && (visible || askHidden)) {
            target.main = window;
            target.hidden = !visible;
        }
    }
    return target;
}

/// What is left of the window the close was posted to, so a log line tells an instance that
/// answered by hiding from one that never answered at all. A process that hid its window and
/// stayed alive is the one the guard's end answer is for.
///
/// @param window the window the close was posted to, or null when there was none
/// @param wasHidden whether that window was already off screen before the close
std::wstring describeWindowState(HWND window, bool wasHidden)
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
    std::wstring state;
    if (IsWindowVisible(window) != FALSE) {
        state = L", window still open";
    } else {
        state = wasHidden ? L", window still hidden" : L", window hidden again";
    }
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

bool isCockatriceImage(const std::wstring &imageName)
{
    constexpr wchar_t COCKATRICE_IMAGE[] = L"cockatrice.exe";
    return CompareStringOrdinal(imageName.c_str(), -1, COCKATRICE_IMAGE, -1, TRUE) == CSTR_EQUAL;
}

CloseHoldersResult
closeCockatriceHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds, bool askHiddenWindows)
{
    CloseHoldersResult result;
    std::vector<std::wstring> lines;
    std::vector<HANDLE> asked;
    std::vector<std::size_t> askedLines;
    std::vector<HWND> askedWindows;
    std::vector<bool> askedWasHidden;

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
        CloseTarget target = chooseCloseTarget(search.windows, askHiddenWindows);
        if (target.main == nullptr && !askHiddenWindows) {
            // A launcher with no window on screen has already answered its own close and is
            // stuck in shutdown (see the header): asking it again cannot reach a user who is
            // not there, so it is named and the installer's wait ends it.
            ++result.withoutWindow;
            lines.push_back(processLabel(holder) + L" has no window of its own to close");
            continue;
        }

        std::wstring line = processLabel(holder);
        if (target.main == nullptr) {
            // No window of the application's own at all: every window left is one of Qt's
            // bookkeeping windows, which must never be asked to close. The installer socket
            // below is the only thing that can reach it.
        } else if (target.hidden) {
            line += L" is hidden in the notification area";
        } else {
            if (target.toRaise == nullptr) {
                target.toRaise = target.main;
            }
            line += bringForward(target.toRaise) ? L" brought forward" : L" could not be brought forward";
        }

        // Kept open across the wait so a pid reused after the process exits cannot be
        // mistaken for the instance that was asked to close.
        const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, holder.processId);
        const DWORD openError = process == nullptr ? GetLastError() : ERROR_SUCCESS;

        // Ask over the installer socket first. It is the same request as the window's close
        // button - the application still answers its own confirmation - but it reaches an
        // instance whose window is hidden in the notification area or is sitting on that
        // confirmation, the two states a posted close cannot get through. A client from
        // before the socket answers with a failed connect, so the posted close below stays
        // the fallback and older versions keep behaving exactly as they did.
        if (requestInstallerClose(holder.processId)) {
            // "and" only when the line already says something about the instance's window.
            line += target.main == nullptr ? L" asked to close over its installer socket"
                                           : L" and asked to close over its installer socket";
        } else if (target.main == nullptr) {
            // Nothing of the application's own to post a close to, and no socket to ask: the
            // instance is named and the guard's end answer is what can free its files.
            if (process != nullptr) {
                CloseHandle(process);
            }
            ++result.withoutWindow;
            lines.push_back(line + L" has no window of its own to close");
            continue;
        } else if (PostMessageW(target.main, WM_CLOSE, 0, 0) == FALSE) {
            // Posted to the instance's own window even when a question of its own is up. The
            // close is what that window's close button does, and the application is the one
            // that answers it: a current client refuses the close while its confirmation is
            // pending, leaving the question in front, and only a client from before that guard
            // accepts it and hides the window with the question. Withholding the request
            // therefore protected nobody - it left the installer asking the user to close a
            // program it had never asked - while the guard's end answer is what frees the files
            // when the close cannot.
            //
            // Read the error before CloseHandle, which is free to reset it.
            const DWORD postError = GetLastError();
            if (process != nullptr) {
                CloseHandle(process);
            }
            lines.push_back(line + L", and the close request could not be sent (win32 " + std::to_wstring(postError) +
                            L")");
            continue;
        } else {
            line += L" and asked to close";
        }
        ++result.askedToClose;
        if (process == nullptr) {
            line += L", but its exit could not be watched (win32 " + std::to_wstring(openError) + L")";
        } else {
            askedLines.push_back(lines.size());
            asked.push_back(process);
            askedWindows.push_back(target.main);
            askedWasHidden.push_back(target.hidden);
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
            } else if (askedWindows[index] == nullptr) {
                // Asked over the socket with no window of its own: there is no window state
                // to describe, only that it is still there.
                lines[askedLines[index]] += L", still running after " + std::to_wstring(elapsed) + L" ms";
            } else {
                lines[askedLines[index]] += L", still running after " + std::to_wstring(elapsed) + L" ms" +
                                            describeWindowState(askedWindows[index], askedWasHidden[index]);
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
