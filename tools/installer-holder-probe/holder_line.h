#ifndef HOLDER_LINE_H
#define HOLDER_LINE_H

#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief One application or service that has an installer file open or mapped.
 *
 * Deliberately free of Windows types: the wording is what the abort dialog shows,
 * and it is the only part of the probe that can be exercised off Windows.
 */
struct Holder
{
    std::wstring imageName;        ///< "cockatrice.exe", empty when the image could not be read
    std::wstring appName;          ///< Restart Manager's name for the application or service
    std::wstring serviceShortName; ///< service short name, empty when this is not a service
    unsigned long processId = 0;
    bool isService = false;
};

/// How many holders the single-line summary names before it says how many were left out.
inline constexpr std::size_t HOLDER_LINE_MAX = 5;

/**
 * @brief How many characters the summary may spend on the holders it names.
 *
 * The line is not the whole string: the abort dialog puts it after a fixed paragraph and
 * before the path of the installer log, inside an NSIS string of 1024 characters. Windows
 * caps one application or service name well below this budget, so it bounds the list
 * without ever cutting a single holder short.
 */
inline constexpr std::size_t HOLDER_LINE_CHAR_MAX = 400;

/**
 * @brief Renders one holder the way the user has to recognise it, e.g.
 *        "cockatrice.exe (pid 24340, running)".
 *
 * The image name is preferred over the application name because it is what Task
 * Manager shows, and the application name is what remains for a process whose
 * image cannot be opened (another account, a protected process). A service is
 * named by Restart Manager's long name and identified by its short name, because
 * both are how the user finds it again in services.msc.
 */
std::wstring formatHolder(const Holder &holder);

/**
 * @brief Renders every holder as a single "; "-joined line.
 *
 * One line because it is written to the installer log as one and repeated in a
 * message box, where nsExec hands the text back as one string. Only the first
 * HOLDER_LINE_MAX are named, and only while they fit in HOLDER_LINE_CHAR_MAX
 * characters; the rest are counted, so a machine with a dozen open handles cannot
 * push the real answer off the dialog.
 */
std::wstring formatHolders(const std::vector<Holder> &holders);

#endif
