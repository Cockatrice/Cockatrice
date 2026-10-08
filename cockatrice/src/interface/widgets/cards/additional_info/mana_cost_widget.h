/**
 * @file mana_cost_widget.h
 * @ingroup CardExtraInfoWidgets
 */
//! \todo Document this file.

#ifndef MANA_COST_WIDGET_H
#define MANA_COST_WIDGET_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <libcockatrice/card/card_info.h>
#include <qtmetamacros.h>

class QHBoxLayout;

class ManaCostWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ManaCostWidget(QWidget *parent, CardInfoPtr card);

    static QStringList parseManaCost(const QString &manaString);
public slots:
    void resizeEvent(QResizeEvent *event) override;

private:
    CardInfoPtr card;
    QHBoxLayout *layout;
};

#endif // MANA_COST_WIDGET_H
