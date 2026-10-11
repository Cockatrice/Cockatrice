/**
 * @file card_amount_widget.h
 * @ingroup CardExtraInfoWidgets
 * @ingroup PrintingWidgets
 */
//! \todo Document this file.

#ifndef CARD_AMOUNT_WIDGET_H
#define CARD_AMOUNT_WIDGET_H

#include "libcockatrice/card/printing/exact_card.h"

#include <QString>
#include <QWidget>
#include <qtmetamacros.h>

class DeckStateManager;
class DynamicFontSizePushButton;
class QHBoxLayout;
class QLabel;
class QSlider;

class CardAmountWidget : public QWidget
{
    Q_OBJECT

signals:
    void deckModified(const QString &modificationReason);

public:
    explicit CardAmountWidget(QWidget *parent,
                              DeckStateManager *deckStateManager,
                              QSlider *cardSizeSlider,
                              const ExactCard &rootCard,
                              const QString &zoneName);
    int getAmount();

public slots:
    void setAmount(int _amount);
    void updateCardCount();
    void addPrinting(const QString &zone);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    DeckStateManager *deckStateManager;
    QSlider *cardSizeSlider;
    ExactCard rootCard;
    QString zoneName;
    QHBoxLayout *layout;
    DynamicFontSizePushButton *incrementButton;
    DynamicFontSizePushButton *decrementButton;
    QLabel *cardCountInZone;

    bool hovered;
    int amount = 0;

    void decrementCardHelper(const QString &zoneName);

private slots:
    void addPrintingMainboard();
    void addPrintingSideboard();
    void addPrintingTokensboard();
    void removePrintingMainboard();
    void removePrintingSideboard();
    void removePrintingTokensboard();
    void adjustFontSize(int scalePercentage);
};

#endif // CARD_AMOUNT_WIDGET_H
