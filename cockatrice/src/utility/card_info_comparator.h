#ifndef CARD_INFO_COMPARATOR_H
#define CARD_INFO_COMPARATOR_H

#include "../game/cards/card_database.h"

#include <QStringList>
#include <QVariant>
#include <Qt>

class CardInfoComparator
{
public:
    explicit CardInfoComparator(const QStringList &properties, Qt::SortOrder order = Qt::AscendingOrder);
    bool operator()(const CardInfoPtr &a, const CardInfoPtr &b) const;

private:
    QStringList m_properties; // List of properties to sort by
    Qt::SortOrder m_order;

    QVariant getProperty(const CardInfoPtr &card, const QString &property) const;
    bool compareVariants(const QVariant &a, const QVariant &b) const;
};

#endif // CARD_INFO_COMPARATOR_H
