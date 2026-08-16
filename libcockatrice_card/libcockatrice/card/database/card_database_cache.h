#ifndef CARDDATABASE_CACHE_H
#define CARDDATABASE_CACHE_H

#include "card_database_data.h"

#include <QByteArray>
#include <libcockatrice/interfaces/interface_card_set_priority_controller.h>

namespace CardDatabaseCache
{
/**
 * @brief Writes a fully-built database snapshot to a binary cache file.
 * @param cachePath Path of the cache file to write.
 * @param data The snapshot to serialize.
 * @param sourceHash Hash of the source XML used to validate the cache later.
 * @return True if the cache was written successfully.
 */
bool write(const QString &cachePath, const CardDatabaseData &data, const QByteArray &sourceHash);

/**
 * @brief Reads a database snapshot from a binary cache file.
 * @param cachePath Path of the cache file to read.
 * @param data Snapshot to populate.
 * @param sourceHash Expected hash of the source XML; read fails if it differs.
 * @param priorityController Controller used to reconstruct CardSet state.
 * @return True if a valid, up-to-date cache was read.
 */
bool read(const QString &cachePath,
          CardDatabaseData &data,
          const QByteArray &sourceHash,
          ICardSetPriorityController *priorityController);
} // namespace CardDatabaseCache

#endif // CARDDATABASE_CACHE_H
