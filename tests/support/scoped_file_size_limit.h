#ifndef COCKATRICE_TEST_SCOPED_FILE_SIZE_LIMIT_H
#define COCKATRICE_TEST_SCOPED_FILE_SIZE_LIMIT_H

#include <QtGlobal>

#ifdef Q_OS_UNIX

#include <algorithm>
#include <csignal>
#include <sys/resource.h>

namespace test_support
{
/**
 * @brief Caps how large a single file may grow, so a writer's short-write path is reachable.
 *
 * Lowering RLIMIT_FSIZE makes the next write past the cap fail with EFBIG instead of
 * filling a filesystem, which is the only way to fail an install step mid-write without
 * depending on free disk space or file ownership. Only the soft limit is lowered, so the
 * destructor can hand the process back exactly as it found it: ctest runs one case per
 * process, but a whole test binary must not carry the cap into its neighbours.
 *
 * Do not use this inside a gtest death test. A forked child under RLIMIT_FSIZE is killed
 * by SIGSEGV on the macOS runners, while the same case passes on Linux, so the fork is
 * what breaks - not the writer. The guard alone isolates the failure just as well.
 */
class ScopedFileSizeLimit
{
public:
    explicit ScopedFileSizeLimit(rlim_t maxBytes)
    {
        if (getrlimit(RLIMIT_FSIZE, &previousLimit) != 0) {
            return;
        }

        struct sigaction ignore = {};
        ignore.sa_handler = SIG_IGN;
        sigemptyset(&ignore.sa_mask);
        if (sigaction(SIGXFSZ, &ignore, &previousAction) != 0) {
            return;
        }
        actionSaved = true;

        struct rlimit capped = previousLimit;
        capped.rlim_cur = std::min(maxBytes, previousLimit.rlim_max);
        if (setrlimit(RLIMIT_FSIZE, &capped) != 0) {
            return;
        }
        active = true;
    }

    ~ScopedFileSizeLimit()
    {
        if (active) {
            setrlimit(RLIMIT_FSIZE, &previousLimit);
        }
        if (actionSaved) {
            sigaction(SIGXFSZ, &previousAction, nullptr);
        }
    }

    ScopedFileSizeLimit(const ScopedFileSizeLimit &) = delete;
    ScopedFileSizeLimit &operator=(const ScopedFileSizeLimit &) = delete;

    /**
     * @brief Whether the cap is in force; a test that needs the failure must assert this.
     */
    [[nodiscard]] bool isValid() const
    {
        return active;
    }

private:
    bool active = false;
    bool actionSaved = false;
    struct rlimit previousLimit = {};
    struct sigaction previousAction = {};
};
} // namespace test_support

#endif // Q_OS_UNIX

#endif // COCKATRICE_TEST_SCOPED_FILE_SIZE_LIMIT_H
