#ifndef RESTART_MANAGER_HOLDERS_H
#define RESTART_MANAGER_HOLDERS_H

#include "holder_line.h"

#include <string>
#include <vector>

/**
 * @brief Lists the files a failed purge can be holding up, as full paths.
 *
 * Everything directly in @p installDir plus everything below its Plugins directory:
 * those are the two things the purge removes, so they are the two sets of files a
 * holder can be holding. Restart Manager refuses a directory with ERROR_ACCESS_DENIED,
 * which is why the probe registers files and this has to name them first.
 *
 * Missing or unreadable directories are skipped rather than reported, and the list is
 * capped: this runs on the abort path and must not walk a tree of downloaded pictures.
 */
std::vector<std::wstring> collectProbeFiles(const std::wstring &installDir);

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
 * @brief Closes the applications and services that have one of @p files open or mapped.
 *
 * Asks them to shut down gracefully through Restart Manager first and forces only what
 * refuses. Only called after the user agreed in the installer's dialog to close them.
 *
 * @return 0 when Restart Manager reported every holder shut down; otherwise the Win32
 *         code of the call that failed. The caller re-lists holders to see what is left.
 */
unsigned long closeProcessesUsingFiles(const std::vector<std::wstring> &files);

#endif
