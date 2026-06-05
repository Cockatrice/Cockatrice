#ifndef COCKATRICE_CARD_DATABASE_VIEW_H
#define COCKATRICE_CARD_DATABASE_VIEW_H

#include "../../key_signals.h"

#include <QTreeView>
#include <libcockatrice/card/card_info.h>

class CardDatabaseModel;
class CardDatabaseDisplayModel;

/**
 * @brief The card database table.
 */
class CardDatabaseView : public QTreeView
{
    Q_OBJECT

    KeySignals searchKeySignals;
    CardDatabaseDisplayModel *databaseDisplayModel;

public:
    explicit CardDatabaseView(QWidget *parent, CardDatabaseDisplayModel *model);

    QString currentCardName() const;

    /**
     * @brief Get the KeySignals that are connected to this view.
     * You can install the KeySignals as an eventFilter to capture keyboard shortcuts for adding and decrementing cards.
     */
    KeySignals *getKeySignals()
    {
        return &searchKeySignals;
    }

signals:
    void cardChanged(const QString &cardName);

    void cardAdded(const QString &cardName, const QString &zoneName);
    void cardDecremented(const QString &cardName, const QString &zoneName);

    void edhrecClicked(const CardInfoPtr &cardInfo, bool isCommander);
    void selectPrintingClicked();
    void relatedCardClicked(const QString &relatedCard);

private slots:
    void actDoubleClick();

    void addCard(const QString &zoneName);
    void decrementCard(const QString &zoneName);
    void updateCard(const QModelIndex &current, const QModelIndex &);

    void resetSelectionIfEmpty();
    void copyDatabaseCellContents() const;
    void saveDbHeaderState();
    void openCustomMenu(QPoint point);
};

#endif // COCKATRICE_CARD_DATABASE_VIEW_H
