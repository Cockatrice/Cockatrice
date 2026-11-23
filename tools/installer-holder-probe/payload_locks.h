#ifndef PAYLOAD_LOCKS_H
#define PAYLOAD_LOCKS_H

#include <string>
#include <vector>

/**
 * @brief The payload executables in @p installDir that refuse to be written, as full paths:
 *        the programs that are running from it right now.
 *
 * The guard asks two questions and both can come back empty for a running instance: the
 * Restart Manager does not reliably report a program's own image or the modules it mapped,
 * and the image path of a process running under another account cannot be read from here,
 * so the process is not named either. The file itself is the evidence neither question
 * depends on: a program's image is opened without write sharing for as long as it runs, so
 * the write access replacing it needs is refused for exactly that long, while a file that is
 * merely being read still accepts it.
 *
 * An installer that answers "nothing is holding the payload" from two lookups that both
 * missed is what sent updates into an uninstall that could only fail halfway, leaving the
 * previous version half removed. This answer is what keeps that from being said: a payload
 * executable that refuses the write is running, whether or not this process may name it.
 *
 * Only the programs the payload installs are tested, not every file the purge removes. A
 * payload executable refusing the write proves that it is running; a mapped module belongs
 * to a running executable, which is tested. A DLL or qt.conf on its own can be held by the
 * program that is asking - the uninstaller runs in place from the directory it empties - and
 * naming that as a holder is the loop the guard already had.
 *
 * @return one full path per executable that still refused after the re-test, in the order
 *         PAYLOAD_PROGRAMS names them; empty when every one of them can be written, or none
 *         is installed.
 */
std::vector<std::wstring> payloadExecutablesInUse(const std::wstring &installDir);

#endif
