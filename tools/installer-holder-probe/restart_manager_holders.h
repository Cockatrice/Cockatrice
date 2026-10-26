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

#endif
