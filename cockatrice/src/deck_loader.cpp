#include <QStringList>
#include <QFile>
#include <QDebug>
#include "deck_loader.h"
#include "decklist.h"

const QStringList DeckLoader::fileNameFilters = QStringList()
    << QObject::tr("Common deck formats (*.cod *.dec *.txt *.mwDeck)")
    << QObject::tr("All files (*.*)");

DeckLoader::DeckLoader()
    : DeckList(),
      lastFileName(QString()),
      lastFileFormat(CockatriceFormat),
      lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const QString &nativeString)
    : DeckList(nativeString),
      lastFileName(QString()),
      lastFileFormat(CockatriceFormat),
      lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const DeckList &other)
    : DeckList(other),
      lastFileName(QString()),
      lastFileFormat(CockatriceFormat),
      lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const DeckLoader &other)
    : DeckList(other),
      lastFileName(other.lastFileName),
      lastFileFormat(other.lastFileFormat),
      lastRemoteDeckId(other.lastRemoteDeckId)
{
}

bool DeckLoader::loadFromFile(const QString &fileName, FileFormat fmt)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    bool result = false;
    switch (fmt) {
        case PlainTextFormat: result = loadFromFile_Plain(&file); break;
        case CockatriceFormat:
            result = loadFromFile_Native(&file);
            qDebug() << "Loaded from" << fileName << "-" << result;
            if (!result) {
                qDebug() << "Retying as plain format";
                file.seek(0);
                result = loadFromFile_Plain(&file);
                fmt = PlainTextFormat;
            }
            break;
    }
    if (result) {
        lastFileName = fileName;
        lastFileFormat = fmt;

        emit deckLoaded();
    }
    qDebug() << "Deck was loaded -" << result;
    return result;
}

bool DeckLoader::loadFromRemote(const QString &nativeString, int remoteDeckId)
{
    bool result = loadFromString_Native(nativeString);
    if (result) {
        lastFileName = QString();
        lastFileFormat = CockatriceFormat;
        lastRemoteDeckId = remoteDeckId;

        emit deckLoaded();
    }
    return result;
}

bool DeckLoader::saveToFile(const QString &fileName, FileFormat fmt)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    bool result = false;
    switch (fmt) {
        case PlainTextFormat: result = saveToFile_Plain(&file); break;
        case CockatriceFormat: result = saveToFile_Native(&file); break;
    }
    if (result) {
        lastFileName = fileName;
        lastFileFormat = fmt;
    }
    return result;
}

DeckLoader::FileFormat DeckLoader::getFormatFromName(const QString &fileName)
{
    if (fileName.endsWith(".cod", Qt::CaseInsensitive)) {
        return CockatriceFormat;
    }
    return PlainTextFormat;
}
