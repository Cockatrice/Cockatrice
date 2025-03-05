#ifndef COLOR_IDENTITY_WIDGET_H
#define COLOR_IDENTITY_WIDGET_H

#include "../../../../../game/cards/card_database.h"

#include <QHBoxLayout>
#include <QWidget>

class ColorIdentityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorIdentityWidget(QWidget *parent, CardInfoPtr card);
    explicit ColorIdentityWidget(QWidget *parent, QString manaCost);

    QStringList parseColorIdentity(const QString &manaString);
public slots:
    void resizeEvent(QResizeEvent *event) override;

private:
    CardInfoPtr card;
    QHBoxLayout *layout;
};

#endif // COLOR_IDENTITY_WIDGET_H
