#ifndef DECK_EDIT_EVENT_BUS_H
#define DECK_EDIT_EVENT_BUS_H

#include <QObject>

/**
 * A singleton object that can be used to pass signals between objects that are far apart in the object tree.
 * Contains signals that are related to deck editor and deck storage.
 */
class DeckEditEventBus : public QObject
{
    Q_OBJECT

private:
    // hide constructor
    explicit DeckEditEventBus();

    // Delete copy constructor and assignment operator to enforce singleton
    DeckEditEventBus(const DeckEditEventBus &) = delete;
    DeckEditEventBus &operator=(const DeckEditEventBus &) = delete;

public:
    static DeckEditEventBus *instance();

signals:
    /**
     * Should be emitted when a change has been written to a deck file.
     * @param filePath Absolute path to the deck file
     */
    void deckFileModified(const QString &filePath);
};

#endif // DECK_EDIT_EVENT_BUS_H
