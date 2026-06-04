#ifndef COCKATRICE_CARD_DATABASE_VIEW_H
#define COCKATRICE_CARD_DATABASE_VIEW_H

#include "../../key_signals.h"
#include "libcockatrice/filters/filter_tree.h"

#include <QTreeView>

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

signals:
    void cardChanged(const QString &cardName);

    void cardAdded(const QString &cardName, const QString &zoneName);
    void cardDecremented(const QString &cardName, const QString &zoneName);

    void edhrecClicked(const CardInfoPtr &cardInfo, bool isCommander);
    void selectPrintingClicked();
    void relatedCardClicked(const QString &relatedCard);

protected:
    bool eventFilter(QObject *, QEvent *event) override;

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
