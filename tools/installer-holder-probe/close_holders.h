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
 * One thing happens per instance: its main window is brought to the foreground, because
 * Cockatrice may answer the close with a confirmation of its own that would otherwise open
 * behind the installer's prompt. Then WM_CLOSE is posted to the main window, the same
 * request as clicking its close button - unless the instance is asking a question of its
 * own, which disables that window: posting the close then only hides the question, so it is
 * brought forward and left to the user. An instance with no window a user could answer -
 * one hidden in the notification area, say - is named and left to the user as well; showing
 * its windows from here does not help, because the application would still believe they are
 * hidden and never paint them. Stopping the process is never attempted.
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
