#ifndef COCKATRICE_DECK_LIST_MEMENTO_H
#define COCKATRICE_DECK_LIST_MEMENTO_H
#include <QString>

class DeckListMemento
{
public:
    explicit DeckListMemento(QString memento, QString reason = QString()) : memento(memento), reason(reason)
    {
    }

    QString getMemento() const
    {
        return memento;
    };

    QString getReason() const
    {
        return reason;
    }

private:
    QString memento;
    QString reason;
};

#endif // COCKATRICE_DECK_LIST_MEMENTO_H
