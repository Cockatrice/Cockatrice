#ifndef INSTALLER_CLOSE_REQUEST_H
#define INSTALLER_CLOSE_REQUEST_H

/**
 * @brief Asks one running payload instance to close over its installer socket.
 *
 * A 3.1.3 client (and later) listens on a local socket named by
 * installerCloseSocketName(). Connecting to it and writing one line is the same request as
 * the window's close button, but it reaches an instance whose window is hidden in the
 * notification area or is sitting on its own confirmation - the two states WM_CLOSE cannot
 * get through. Older clients have no socket, so the caller still falls back to WM_CLOSE.
 *
 * Best effort: a failed connect or write is not an error. The caller has the window close
 * and the guard's force-close answer left.
 *
 * @param processId the process to ask, as the Restart Manager named it
 * @return true when the request was written to a listening instance
 */
bool requestInstallerClose(unsigned long processId);

#endif // INSTALLER_CLOSE_REQUEST_H
