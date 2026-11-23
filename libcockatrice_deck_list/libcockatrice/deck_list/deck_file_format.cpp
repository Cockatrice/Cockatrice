#include "deck_file_format.h"

#include <qnamespace.h>

DeckFileFormat::Format DeckFileFormat::getFormatFromName(const QString &fileName)
{
    if (fileName.endsWith(".cod", Qt::CaseInsensitive)) {
        return Cockatrice;
    }
    return PlainText;
}