#ifndef RESTART_MANAGER_HOLDERS_H
#define RESTART_MANAGER_HOLDERS_H

#include "holder_line.h"

#include <string>
#include <vector>

/**
 * @brief Lists the payload files, as full paths.
 *
 * The set is what the installer replaces and the purge removes: the three executables,
 * qt.conf, every top-level DLL and everything below Plugins. A document, a log or a
 * screenshot the user happens to have open in the install directory is not part of it,
 * so it cannot be mistaken for a running instance and block an upgrade.
 *
 * Restart Manager refuses a directory with ERROR_ACCESS_DENIED, which is why the probe
 * registers files and this has to name them first. Missing files and unreadable
 * directories are skipped rather than reported, and the list is capped: this runs before
 * the first page and on the abort path, and must not walk a tree of downloaded pictures.
 */
std::vector<std::wstring> collectPayloadFiles(const std::wstring &installDir);

/**
 * @brief Names the applications and services that have one of @p files open or mapped.
 *
 * Restart Manager reports both a process that loaded one of the files as a module and
 * one that merely holds a handle to it, and it reports services, which is the whole
 * reason it is used instead of walking every process's module list.
 *
 * @param holders replaced with one entry per holder, in the order Windows reported them.
 * @return 0 when the lookup ran, even if nothing is holding the files; otherwise the
 *         Win32 code of the Restart Manager call that failed.
 */
unsigned long findProcessesUsingFiles(const std::vector<std::wstring> &files, std::vector<Holder> &holders);

/**
 * @brief Lists every process whose image lives in @p installDir, whether or not it has a
 *        payload file open.
 *
 * Restart Manager answers the question "who has this file open", and a running instance
 * does not necessarily have one open: its own image and the modules it loaded are mapped
 * into it, not opened by it, and RmGetList does not reliably report a mapping. That is
 * the state the failed updates this probe exists for were in - every DLL still on disk,
 * an instance running from the directory, and "nothing is holding them any more" as the
 * answer - so an answer built on file handles alone cannot be the guard's authority.
 *
 * Reading the image path needs PROCESS_QUERY_LIMITED_INFORMATION and
 * QueryFullProcessImageNameW, neither of which depends on the bitness of either process,
 * so a 64-bit instance is named from this 32-bit stub. The comparison is on the
 * directory, not on an image name, so a process that merely shares a name (an Oracle
 * database server next to a Cockatrice install) is never named.
 *
 * @param installDir the directory being replaced, as a full path.
 * @return one entry per process running from it, in snapshot order, without the probe
 *         itself; empty when the directory is empty or no process runs from it.
 */
std::vector<Holder> findProcessesRunningFrom(const std::wstring &installDir);

#endif
