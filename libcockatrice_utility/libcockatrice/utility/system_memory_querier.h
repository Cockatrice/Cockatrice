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
        const qulonglong GiB = 1024ull * 1024ull * 1024ull;
        return totalMemoryBytes() >= (qulonglong)gib * GiB;
    }
};

#endif // COCKATRICE_SYSTEM_MEMORY_QUERIER_H
