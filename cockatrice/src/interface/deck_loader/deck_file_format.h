#ifndef COCKATRICE_DECK_FILE_FORMAT_H
#define COCKATRICE_DECK_FILE_FORMAT_H
#include <QString>

namespace DeckFileFormat
{

/**
 * The deck file formats that Cockatrice supports.
 */
enum Format
{
    /**
     * Plaintext deck files, a format that is intended to be widely supported among different programs.
     * This format does not support Cockatrice specific features such as banner cards or tags.
     */
    PlainText,

    /**
     * This is cockatrice's native deck file format, and supports deck metadata such as banner cards and tags.
     * Stored as .cod files.
     */
    Cockatrice
};

/**
 * Determines what deck file format the given filename corresponds to.
 *
 * @param fileName The filename
 * @return The deck format
 */
Format getFormatFromName(const QString &fileName);

} // namespace DeckFileFormat

#endif // COCKATRICE_DECK_FILE_FORMAT_H
