#ifndef COCKATRICE_CARD_SET_H
#define COCKATRICE_CARD_SET_H

#include <QDate>
#include <QList>
#include <QSharedPointer>
#include <QString>

class CardInfo;
using CardInfoPtr = QSharedPointer<CardInfo>;

class CardSet;
using CardSetPtr = QSharedPointer<CardSet>;

class CardSet : public QList<CardInfoPtr>
{
public:
    enum Priority
    {
        PriorityFallback = 0,
        PriorityPrimary = 10,
        PrioritySecondary = 20,
        PriorityReprint = 30,
        PriorityOther = 40,
        PriorityLowest = 100,
    };

    static const char *TOKENS_SETNAME;

private:
    QString shortName, longName;
    unsigned int sortKey;
    QDate releaseDate;
    QString setType;
    Priority priority;
    bool enabled, isknown;

public:
    explicit CardSet(const QString &_shortName = QString(),
                     const QString &_longName = QString(),
                     const QString &_setType = QString(),
                     const QDate &_releaseDate = QDate(),
                     const Priority _priority = PriorityFallback);

    static CardSetPtr newInstance(const QString &_shortName = QString(),
                                  const QString &_longName = QString(),
                                  const QString &_setType = QString(),
                                  const QDate &_releaseDate = QDate(),
                                  const Priority _priority = PriorityFallback);

    QString getCorrectedShortName() const;

    QString getShortName() const
    {
        return shortName;
    }
    QString getLongName() const
    {
        return longName;
    }
    QString getSetType() const
    {
        return setType;
    }
    QDate getReleaseDate() const
    {
        return releaseDate;
    }
    Priority getPriority() const
    {
        return priority;
    }

    void setLongName(const QString &_longName)
    {
        longName = _longName;
    }
    void setSetType(const QString &_setType)
    {
        setType = _setType;
    }
    void setReleaseDate(const QDate &_releaseDate)
    {
        releaseDate = _releaseDate;
    }
    void setPriority(const Priority _priority)
    {
        priority = _priority;
    }

    void loadSetOptions();
    int getSortKey() const
    {
        return sortKey;
    }
    void setSortKey(unsigned int _sortKey);

    bool getEnabled() const
    {
        return enabled;
    }
    void setEnabled(bool _enabled);

    bool getIsKnown() const
    {
        return isknown;
    }
    void setIsKnown(bool _isknown);

    bool getIsKnownIgnored() const
    {
        return longName.length() + setType.length() + releaseDate.toString().length() == 0;
    }
};

#endif // COCKATRICE_CARD_SET_H
