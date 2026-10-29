#ifndef CLOSE_HOLDERS_H
#define CLOSE_HOLDERS_H

#include "holder_line.h"

#include <string>
#include <vector>

/**
 * @brief What asking the holders to close did, in the form the installer logs.
 *
 * @c report is one line naming every holder, including the ones this deliberately
 * leaves alone, because the installer repeats it in its log.
 * @c withoutWindow counts the holders that had no window of their own to ask: they were
 * named and left alone, and the launcher mode reads that count as "nothing to ask" rather
 * than as a window the user still has to answer.
 */
struct CloseHoldersResult
{
    unsigned int askedToClose = 0;  ///< holders a WM_CLOSE was posted to
    unsigned int withoutWindow = 0; ///< holders with no window of their own to ask
    std::wstring report;            ///< one line, narrowed to ASCII when written
};

/**
 * @brief Whether @p imageName is the client the update belongs to, matched without case.
 *
 * The close mode asks only this image to close, and the launcher mode is only allowed to
 * report it as safe to end, so both read the answer from here.
 *
 * @param imageName the file name of a process image, empty when it could not be read
 */
bool isCockatriceImage(const std::wstring &imageName);

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
 * One thing happens per instance: its own window is asked to close with WM_CLOSE, the same
 * request as clicking that window's close button - whether it is on screen or only in the
 * notification area, since the request does not need the window to be visible and the
 * confirmation Cockatrice answers it with is a window Qt shows itself, painted, unlike one
 * restored from outside. A window that is on screen is brought forward first, because that
 * confirmation would otherwise open behind the installer's prompt; nothing is shown from
 * here. Qt's own bookkeeping windows are never the target - posting a close to the
 * notification-area one terminates the instance without its confirmation - so a process
 * without a window of its own is named and left to the guard, which offers to end it.
 * Stopping the process is never attempted here.
 *
 * Once the requests are out, this waits up to @c waitMilliseconds for the processes to
 * exit, so the caller can measure the purge immediately afterwards instead of sleeping
 * for a fixed time or re-asking a question this call already answered. A process that is
 * still running is described further: whether its window is gone, still open, still
 * hidden or hidden again, and whether it answers at all, so the log can tell an instance
 * that refused the close from one that never received it.
 *
 * @param holders the Restart Manager answer, one entry per application or service
 * @param waitMilliseconds how long to wait for the asked instances to exit, 0 to not wait
 * @param askHiddenWindows whether a window that is not on screen is asked to close too.
 *        The guard wants that: an instance in the notification area is the one this update
 *        has no other way to reach. The launcher mode does not - a launcher without a
 *        visible window has already answered its own close and is stuck in shutdown, and
 *        the installer ends it instead of putting a question to a user who is not there.
 */
CloseHoldersResult
closeCockatriceHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds, bool askHiddenWindows);

/**
 * @brief What ending the holders did, in the form the installer logs.
 *
 * @c report is one line naming every holder, including the ones this deliberately leaves
 * alone, because the installer repeats it in its log.
 */
struct EndHoldersResult
{
    unsigned int ended = 0; ///< holders TerminateProcess was accepted for
    std::wstring report;    ///< one line, narrowed to ASCII when written
};

/**
 * @brief Ends every running Cockatrice, Oracle and Servatrice the probe named.
 *
 * This is the last resort behind the installer's "end it now" answer, never a first move: a
 * client that is still running may have an unsaved deck, an open game or a settings write in
 * flight, Oracle may be part-way through writing the card database, and Servatrice may be
 * serving a game, so the caller only gets here after the user picked it from a prompt that
 * says so.
 *
 * Every program the payload installs is ended, services included. A service is terminated
 * like a process rather than stopped through the Service Control Manager: the installer has
 * asked the user first, and the SCM notices the process exit and marks the service stopped
 * afterwards, so it can be started again normally. Anything else the Restart Manager named -
 * a scanner holding a DLL, a program the user keeps in the install directory - is left to
 * the user, because ending it would destroy something this installer does not own.
 *
 * Waits up to @c waitMilliseconds for what it ended to be gone, so the caller can measure the
 * purge immediately afterwards instead of racing a process that is still exiting.
 *
 * @param holders what the probe named, one entry per application or service
 * @param waitMilliseconds how long to wait for the ended instances to be gone, 0 to not wait
 */
EndHoldersResult endHolders(const std::vector<Holder> &holders, unsigned long waitMilliseconds);

#endif
