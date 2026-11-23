#ifndef PAYLOAD_PATHS_H
#define PAYLOAD_PATHS_H

#include <string>

/**
 * @brief The file names of the programs the payload installs, in the order the guard names
 *        them.
 *
 * The single list behind isPayloadProgram(), so the guard acts on exactly the programs it
 * names, and so a payload program added later is closed, ended and tested for a lock
 * without a second list to update. A payload executable that refuses to be written is
 * running, which is what makes this list the evidence the two process lookups cannot
 * provide - see payloadExecutablesInUse().
 */
inline constexpr const wchar_t *PAYLOAD_PROGRAMS[] = {L"cockatrice.exe", L"oracle.exe", L"servatrice.exe"};

/**
 * @brief The last name in @p path: what follows its last separator, or @p path itself when
 *        it has none.
 *
 * Empty for an empty path and for one that ends in a separator, which has no name of its
 * own - a caller that hands in a directory there gets no name rather than the directory's
 * own last component.
 */
std::wstring fileNameOf(const std::wstring &path);

/**
 * @brief The directory @p path lies in, without a trailing separator.
 *
 * Empty when @p path has no directory part. Used to tell where a process was started from:
 * an image running directly in the install directory has that directory as its parent,
 * while one started from a folder below it has that folder, and the two are not the same
 * answer to "does this process belong to the installation being replaced?".
 */
std::wstring parentDirectoryOf(const std::wstring &path);

/**
 * @brief Whether @p imageName is one of the programs the payload installs.
 *
 * The single place that answers which images are Cockatrice's own, so the guard acts on the
 * same set it names, and so a program a user happens to keep in the install directory is
 * never treated as an instance of the application. Matched on the image name alone, without
 * case: the names are the payload's own, and the comparison is deliberately free of Windows
 * API so it can be exercised off Windows.
 *
 * @param imageName the file name of a process image, empty when it could not be read
 */
bool isPayloadProgram(const std::wstring &imageName);

#endif
