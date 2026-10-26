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
 * Two things happen per instance. Its top-level window is restored and brought to the
 * foreground, because Cockatrice may answer the close with a confirmation of its own that
 * would otherwise open behind the installer's prompt, and then WM_CLOSE is posted to the
 * main window, the same request as clicking its close button. Stopping the process is
 * never attempted.
 *
 * Once the requests are out, this waits up to @c waitMilliseconds for the processes to
 * exit, so the caller can measure the purge immediately afterwards instead of sleeping
 * for a fixed time or re-asking a question this call already answered.
 *
 * @param holders the Restart Manager answer, one entry per application or service
 * @param waitMilliseconds how long to wait for the asked instances to exit, 0 to not wait
 */
CloseHoldersResult closeCockatriceHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds);

#endif
