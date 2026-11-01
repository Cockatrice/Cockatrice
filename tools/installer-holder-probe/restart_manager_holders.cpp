#include "restart_manager_holders.h"

// clang-format off
// restartmanager.h uses the types from windows.h without including it, so the order
// here is a requirement; Regroup would sort them the other way round.
#include <windows.h>
#include <restartmanager.h>
#include <tlhelp32.h>
// clang-format on

#include <algorithm>
#include <iterator>
#include <utility>

namespace
{
/// Stop registering after this many files. The install directory carries around 150
/// files in practice; the cap only exists so a directory someone has filled with files
/// cannot turn the abort path into a long Restart Manager call.
constexpr std::size_t MAX_PROBED_FILES = 2000;

/// How deep the Plugins walk goes. Its real shape is Plugins\<category>\<plugin>.dll.
constexpr int MAX_PLUGIN_DEPTH = 4;

/// How many parents askingProcessIds() walks up before it stops on its own. A real chain
/// is the probe, the installer or uninstaller, and the shell that started it; the bound
/// only keeps a snapshot with a cycle in it from walking forever.
constexpr int MAX_ASKER_LEVELS = 8;

void collectFilesOf(const std::wstring &directory, int depth, int maxDepth, std::vector<std::wstring> &files)
{
    if (files.size() >= MAX_PROBED_FILES) {
        return;
    }
    WIN32_FIND_DATAW entry{};
    const HANDLE find = FindFirstFileW((directory + L"\\*").c_str(), &entry);
    if (find == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (files.size() >= MAX_PROBED_FILES) {
            break;
        }
        const std::wstring name = entry.cFileName;
        if (name == L"." || name == L".." || (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
            continue;
        }
        const std::wstring path = directory + L"\\" + name;
        if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            if (depth < maxDepth) {
                collectFilesOf(path, depth + 1, maxDepth, files);
            }
        } else {
            files.push_back(path);
        }
    } while (FindNextFileW(find, &entry) != 0);
    FindClose(find);
}

std::wstring imagePathOf(unsigned long processId)
{
    // PROCESS_QUERY_LIMITED_INFORMATION is the right to ask, not to debug: it reads the
    // path of a process this one does not own, which is what keeps a SYSTEM-owned holder
    // from turning the whole list into an error. One that refuses even this is still
    // named, by whatever Restart Manager called it.
    const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (process == nullptr) {
        return {};
    }
    wchar_t path[MAX_PATH * 2] = {};
    DWORD length = static_cast<DWORD>(std::size(path));
    std::wstring imagePath;
    if (QueryFullProcessImageNameW(process, 0, path, &length) != 0) {
        imagePath.assign(path, length);
    }
    CloseHandle(process);
    return imagePath;
}

/// @p directory with exactly one trailing separator, so that two spellings of the same
/// directory compare equal and a sibling whose name merely begins with the same letters
/// ("Cockatrice-old" next to "Cockatrice") cannot pass for it.
std::wstring directoryPrefixOf(const std::wstring &directory)
{
    std::wstring prefix = directory;
    while (!prefix.empty() && (prefix.back() == L'\\' || prefix.back() == L'/')) {
        prefix.pop_back();
    }
    prefix += L'\\';
    return prefix;
}

/// Whether @p left and @p right name the same directory, compared the way the file system
/// does: a difference in case is not a difference in directory, and a trailing separator
/// changes nothing.
bool isSameDirectory(const std::wstring &left, const std::wstring &right)
{
    if (left.empty() || right.empty()) {
        return false;
    }
    const std::wstring leftPrefix = directoryPrefixOf(left);
    const std::wstring rightPrefix = directoryPrefixOf(right);
    return CompareStringOrdinal(leftPrefix.c_str(), -1, rightPrefix.c_str(), -1, TRUE) == CSTR_EQUAL;
}

/// Copies a fixed-size, NUL-terminated WCHAR array the API filled in.
template <std::size_t N> std::wstring fixedString(const wchar_t (&text)[N])
{
    return std::wstring(text, std::find(text, text + N, L'\0'));
}

Holder holderOf(const RM_PROCESS_INFO &info)
{
    Holder holder;
    holder.processId = info.Process.dwProcessId;
    holder.isService = info.ApplicationType == RmService;
    holder.appName = fixedString(info.strAppName);
    if (holder.isService) {
        holder.serviceShortName = fixedString(info.strServiceShortName);
    }
    holder.imageName = fileNameOf(imagePathOf(holder.processId));
    return holder;
}
} // namespace

std::vector<std::wstring> collectPayloadFiles(const std::wstring &installDir)
{
    std::vector<std::wstring> files;
    // The programs the payload installs, by name, plus the configuration file next to
    // them: what the purge deletes by name, and therefore what an instance can have
    // mapped.
    std::vector<std::wstring> replaced(std::begin(PAYLOAD_PROGRAMS), std::end(PAYLOAD_PROGRAMS));
    replaced.emplace_back(L"qt.conf");
    for (const std::wstring &name : replaced) {
        const std::wstring path = installDir + L"\\" + name;
        const DWORD attributes = GetFileAttributesW(path.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            files.push_back(path);
        }
    }

    // Every top-level DLL: the set the purge deletes and therefore the set a running
    // instance can have mapped. Matched by mask rather than by a list of expected names,
    // so a transitive dependency nobody thought of is covered as well.
    WIN32_FIND_DATAW entry{};
    const HANDLE dlls = FindFirstFileW((installDir + L"\\*.dll").c_str(), &entry);
    if (dlls != INVALID_HANDLE_VALUE) {
        do {
            if (files.size() >= MAX_PROBED_FILES) {
                break;
            }
            if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
                (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
                continue;
            }
            files.push_back(installDir + L"\\" + entry.cFileName);
        } while (FindNextFileW(dlls, &entry) != 0);
        FindClose(dlls);
    }

    // Plugins is the other tree the purge removes. Its data directory is not: it holds
    // downloaded pictures, which the purge does not touch and which would turn this into
    // a walk over thousands of files.
    if (files.size() < MAX_PROBED_FILES) {
        collectFilesOf(installDir + L"\\Plugins", 0, MAX_PLUGIN_DEPTH, files);
    }
    return files;
}

unsigned long findProcessesUsingFiles(const std::vector<std::wstring> &files, std::vector<Holder> &holders)
{
    holders.clear();
    if (files.empty()) {
        return ERROR_SUCCESS;
    }

    DWORD session = 0;
    wchar_t sessionKey[CCH_RM_SESSION_KEY + 1] = {};
    DWORD error = RmStartSession(&session, 0, sessionKey);
    if (error != ERROR_SUCCESS) {
        return error;
    }

    // One call for the whole set, as the API asks: RmRegisterResources does registry
    // work per file and is documented as too expensive to call once per file.
    std::vector<LPCWSTR> names;
    names.reserve(files.size());
    for (const std::wstring &file : files) {
        names.push_back(file.c_str());
    }
    error = RmRegisterResources(session, static_cast<UINT>(names.size()), names.data(), 0, nullptr, 0, nullptr);

    if (error == ERROR_SUCCESS) {
        UINT needed = 0;
        UINT count = 0;
        DWORD rebootReasons = 0;
        // The first call asks how large the answer is; Restart Manager reports
        // ERROR_MORE_DATA rather than truncating a list that does not fit.
        error = RmGetList(session, &needed, &count, nullptr, &rebootReasons);
        // Bounded: each retry more than covers the list growing between the two calls,
        // and an abort path cannot chase a process that keeps opening files. Giving up
        // reports the code, which is a truthful answer instead of a truncated list.
        for (int attempt = 0; attempt < 3 && error == ERROR_MORE_DATA; ++attempt) {
            count = needed;
            std::vector<RM_PROCESS_INFO> infos(count);
            error = RmGetList(session, &needed, &count, infos.data(), &rebootReasons);
            if (error == ERROR_SUCCESS) {
                holders.reserve(count);
                for (UINT index = 0; index < count; ++index) {
                    holders.push_back(holderOf(infos[index]));
                }
            }
        }
    }

    RmEndSession(session);
    return error;
}

std::vector<unsigned long> askingProcessIds()
{
    std::vector<unsigned long> asking;
    asking.push_back(GetCurrentProcessId());

    // One snapshot for the whole walk: the parent ids come from it, so a level costs no
    // second lookup.
    std::vector<PROCESSENTRY32W> processes;
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return asking;
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry) != FALSE) {
        do {
            processes.push_back(entry);
        } while (Process32NextW(snapshot, &entry) != FALSE);
    }
    CloseHandle(snapshot);

    unsigned long current = asking.back();
    for (int level = 0; level < MAX_ASKER_LEVELS; ++level) {
        const auto named = std::find_if(processes.begin(), processes.end(), [current](const PROCESSENTRY32W &process) {
            return process.th32ProcessID == current;
        });
        if (named == processes.end() || named->th32ParentProcessID == 0) {
            break;
        }
        const unsigned long parent = named->th32ParentProcessID;
        if (isPayloadProgram(fileNameOf(imagePathOf(parent)))) {
            break;
        }
        asking.push_back(parent);
        current = parent;
    }
    return asking;
}

std::vector<Holder> findProcessesRunningFrom(const std::wstring &installDir)
{
    std::vector<Holder> holders;
    if (installDir.empty()) {
        return holders;
    }

    // Never the probe, and never the installer or uninstaller that started it: either can
    // run from the directory being replaced, and naming one asks the user to close the
    // program that is asking (see askingProcessIds).
    const std::vector<unsigned long> asking = askingProcessIds();

    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return holders;
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry) != FALSE) {
        do {
            if (std::find(asking.begin(), asking.end(), entry.th32ProcessID) != asking.end()) {
                continue;
            }
            // One process at a time, because the path is per process; a process that
            // refuses to be read is not named here and is still named by Restart
            // Manager when it holds a handle. The idle process has no image at all.
            const std::wstring imagePath = imagePathOf(entry.th32ProcessID);
            const std::wstring imageName = fileNameOf(imagePath);
            // Only the programs the payload installs, and only those started from the
            // directory itself: a folder a user installs into can hold other applications,
            // each in its own subfolder, and one of those is not an instance of this
            // payload. Naming it would ask for a close the probe cannot send and an end it
            // would refuse to perform, so the guard could only ever abort. A program below
            // the directory that really does hold a payload file is named by
            // findProcessesUsingFiles, which is the lookup for that question.
            if (!isPayloadProgram(imageName) || !isSameDirectory(parentDirectoryOf(imagePath), installDir)) {
                continue;
            }
            Holder holder;
            holder.processId = entry.th32ProcessID;
            holder.imageName = imageName;
            holders.push_back(std::move(holder));
        } while (Process32NextW(snapshot, &entry) != FALSE);
    }
    CloseHandle(snapshot);
    return holders;
}
