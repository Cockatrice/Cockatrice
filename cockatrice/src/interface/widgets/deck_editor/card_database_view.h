#ifndef COCKATRICE_CARD_DATABASE_VIEW_H
#define COCKATRICE_CARD_DATABASE_VIEW_H

#include "../../key_signals.h"

#include <QTreeView>
#include <functional>
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

    /// Provides the custom zones available in the current deck, grouped by board zone.
    /// The list contains (board zone name, custom zone names) pairs for every board.
    std::function<QList<QPair<QString, QStringList>>()> zoneMenuProvider;
    /// Handler invoked when the user picks "New zone..." from the add-to-zone menu.
    /// Returns the name of the created zone, or an empty string if creation was cancelled.
    std::function<QString()> newZoneHandler;

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

    /**
     * @brief Sets the provider used to populate the "Add to zone" submenu of the context menu.
     * If no provider is set, the submenu is not shown.
     *
     * @param provider Returns the custom zones of the current deck, grouped by board zone
     * @param newZoneHandler Creates a new custom zone and returns its name, or an empty string
     *                       if creation was cancelled. The menu entry is hidden when not provided.
     */
    void setZoneMenuProvider(const std::function<QList<QPair<QString, QStringList>>()> &provider,
                             const std::function<QString()> &newZoneHandler);

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
