#ifndef CLOSE_HOLDERS_H
#define CLOSE_HOLDERS_H

#include "holder_line.h"

#include <string>
#include <vector>

/**
 * @brief What asking the holders to close did, in the form the installer logs.
 *
 * @c report is one line naming every holder, including the ones this deliberately
 * leaves alone, because the installer repeats it in its log and in the second prompt.
 */
struct CloseHoldersResult
{
    unsigned int askedToClose = 0; ///< holders a WM_CLOSE was posted to
    std::wstring report;           ///< one line, narrowed to ASCII when written
};

/**
 * @brief Brings every running Cockatrice holding an installer file forward and asks it
 *        to close.
 *
 * The set is the Restart Manager answer, so it is exactly the processes holding a file of
 * the previous version: nothing here has to guess at an executable path, and an
 * application that merely shares the image name is never touched. Only cockatrice.exe is
 * asked to close. Oracle and Servatrice are left to the user - Oracle writes the card
 * database in place - and so is every service.
 *
 * Two things happen per instance. Every window it left behind is shown again and its main
 * window is brought to the foreground, because Cockatrice may answer the close with a
 * confirmation of its own that would otherwise open behind the installer's prompt - and an
 * instance that already answered an earlier close by hiding instead of quitting has that
 * confirmation hidden with the window that owns it, where nobody can answer it. Then
 * WM_CLOSE is posted to the main window, the same request as clicking its close button -
 * unless the instance is asking a question of its own, which disables that window: posting
 * the close then only hides the question, so it is brought forward and left to the user.
 * Stopping the process is never attempted.
 *
 * Once the requests are out, this waits up to @c waitMilliseconds for the processes to
 * exit, so the caller can measure the purge immediately afterwards instead of sleeping
 * for a fixed time or re-asking a question this call already answered. A process that is
 * still running is described further: whether its window is gone, still open, hidden
 * again, and whether it answers at all, so the log can tell an instance that refused the
 * close from one that never received it.
 *
 * @param holders the Restart Manager answer, one entry per application or service
 * @param waitMilliseconds how long to wait for the asked instances to exit, 0 to not wait
 */
CloseHoldersResult closeCockatriceHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds);

#endif
