/**
 * @file mana_cost_widget.h
 * @ingroup CardExtraInfoWidgets
 * @brief TODO: Document this.
 */

#ifndef MANA_COST_WIDGET_H
#define MANA_COST_WIDGET_H

#include "card/card_info.h"

#include <QHBoxLayout>
#include <QWidget>

class ManaCostWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ManaCostWidget(QWidget *parent, CardInfoPtr card);

    QStringList parseManaCost(const QString &manaString);
public slots:
    void resizeEvent(QResizeEvent *event) override;

private:
    CardInfoPtr card;
    QHBoxLayout *layout;
};

#endif // MANA_COST_WIDGET_H
