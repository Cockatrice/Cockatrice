/**
 * @file card_info_text_widget.h
 * @ingroup CardWidgets
 * @brief TODO: Document this.
 */

#ifndef CARDINFOTEXT_H
#define CARDINFOTEXT_H

#include "libcockatrice/card/printing/exact_card.h"

#include <QFrame>
#include <libcockatrice/card/card_info.h>
class QLabel;
class QScrollArea;
class QTextEdit;

class CardInfoTextWidget : public QFrame
{
    Q_OBJECT

private:
    QLabel *propsLabel;
    QScrollArea *propsScroll;
    QTextEdit *textLabel;
    CardInfoPtr info;
    void setTexts(const QString &propsText, const QString &textText);

public:
    explicit CardInfoTextWidget(QWidget *parent = nullptr);
    void retranslateUi();
    void setInvalidCardName(const QString &cardName);

signals:
    void linkActivated(const QString &link);
public slots:
    void setCard(const ExactCard &card);
};

#endif
