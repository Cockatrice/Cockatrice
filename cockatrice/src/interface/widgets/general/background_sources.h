/**
 * @file background_sources.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_BACKGROUND_SOURCES_H
#define COCKATRICE_BACKGROUND_SOURCES_H

#include <QList>
#include <QObject>
#include <QString>

class BackgroundSources
{
    Q_GADGET
public:
    enum Type
    {
        Theme,
        RandomCardArt,
        DeckFileArt
    };
    Q_ENUM(Type)

    struct Entry
    {
        Type type;
        const char *id;    // stable ID for settings
        const char *trKey; // key for translation
    };

    static QList<Entry> all()
    {
        return {{Theme, "theme", QT_TR_NOOP("Theme")},
                {RandomCardArt, "random_card_art", QT_TR_NOOP("Art crop of random card")},
                {DeckFileArt, "deck_file_art", QT_TR_NOOP("Art crop of background.cod deck file")}};
    }

    static QString toId(Type type)
    {
        for (const auto &e : all()) {
            if (e.type == type)
                return e.id;
        }
        return {};
    }

    static Type fromId(const QString &id)
    {
        for (const auto &e : all()) {
            if (id == e.id)
                return e.type;
        }
        return Theme; // default
    }

    static QString toDisplay(Type type)
    {
        for (const auto &e : all()) {
            if (e.type == type)
                return QObject::tr(e.trKey);
        }
        return {};
    }
};

#endif // COCKATRICE_BACKGROUND_SOURCES_H
