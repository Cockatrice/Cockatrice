/**
 * @file card_amount_widget.h
 * @ingroup CardExtraInfoWidgets
 * @ingroup PrintingWidgets
 * @brief TODO: Document this.
 */

#ifndef CARD_AMOUNT_WIDGET_H
#define CARD_AMOUNT_WIDGET_H

#include "../../../interface/widgets/tabs/abstract_tab_deck_editor.h"
#include "../general/display/dynamic_font_size_push_button.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/models/deck_list/deck_list_model.h>
#include <libcockatrice/models/deck_list/deck_loader.h>

class CardAmountWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CardAmountWidget(QWidget *parent,
                              AbstractTabDeckEditor *deckEditor,
                              DeckListModel *deckModel,
                              QTreeView *deckView,
                              QSlider *cardSizeSlider,
                              const ExactCard &rootCard,
                              const QString &zoneName);
    int countCardsInZone(const QString &deckZone);

public slots:
    void updateCardCount();
    void addPrinting(const QString &zone);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    AbstractTabDeckEditor *deckEditor;
    DeckListModel *deckModel;
    QTreeView *deckView;
    QSlider *cardSizeSlider;
    ExactCard rootCard;
    QString zoneName;
    QHBoxLayout *layout;
    DynamicFontSizePushButton *incrementButton;
    DynamicFontSizePushButton *decrementButton;
    QLabel *cardCountInZone;

    bool hovered;

    void offsetCountAtIndex(const QModelIndex &idx, int offset);
    void decrementCardHelper(const QString &zoneName);
    void recursiveExpand(const QModelIndex &index);

private slots:
    void addPrintingMainboard();
    void addPrintingSideboard();
    void removePrintingMainboard();
    void removePrintingSideboard();
    void adjustFontSize(int scalePercentage);
};

#endif // CARD_AMOUNT_WIDGET_H
