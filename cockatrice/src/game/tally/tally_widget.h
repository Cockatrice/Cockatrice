#ifndef COCKATRICE_TALLY_WIDGET_H
#define COCKATRICE_TALLY_WIDGET_H
#include "tally_manager.h"

#include <QGridLayout>
#include <QScrollArea>
#include <QWidget>

class QLabel;
class CardItem;

class TallyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TallyWidget(QWidget *parent, TallyManager *tallyManager);

private:
    TallyManager *tallyManager;

    struct TallyEntryInternal
    {
        TallyManager::EntryType type;
        QLabel *textLabel;
        QLabel *valueLabel;
    };

    QList<TallyEntryInternal> tallyEntriesInternal;

    QGridLayout *gridLayout;

    void resetGridLayout(QList<TallyManager::EntryType> entryTypes);
    QLabel *createTextLabel(TallyManager::EntryType entryType);
    QLabel *createValueLabel(TallyManager::EntryType entryType);
};

#endif // COCKATRICE_TALLY_WIDGET_H
