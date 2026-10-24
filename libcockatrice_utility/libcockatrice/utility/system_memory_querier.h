#ifndef COCKATRICE_SYSTEM_MEMORY_QUERIER_H
#define COCKATRICE_SYSTEM_MEMORY_QUERIER_H

#include <QtGlobal>

class SystemMemoryQuerier
{
public:
    static qulonglong totalMemoryBytes();
    static qulonglong availableMemoryBytes();

    static bool hasAtLeastGiB(int gib)
    {
        const qulonglong total = totalMemoryBytes();
        // 0 means the amount could not be determined (unsupported platform or failed query).
        // Treat it as unknown and skip the check instead of misreading it as low memory.
        if (total == 0) {
            return true;
        }
        const qulonglong GiB = 1024ull * 1024ull * 1024ull;
        return total >= (qulonglong)gib * GiB;
    }
};

#endif // COCKATRICE_SYSTEM_MEMORY_QUERIER_H
