/**
 * @file color_identity_widget.h
 * @ingroup CardExtraInfoWidgets
 * @brief TODO: Document this.
 */

#ifndef COLOR_IDENTITY_WIDGET_H
#define COLOR_IDENTITY_WIDGET_H

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/card/card_info.h>

class ColorIdentityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorIdentityWidget(QWidget *parent, CardInfoPtr card);
    explicit ColorIdentityWidget(QWidget *parent, QString manaCost);
    void populateManaSymbolWidgets();

    QStringList parseColorIdentity(const QString &manaString);

public slots:
    void resizeEvent(QResizeEvent *event) override;
    void toggleUnusedVisibility();

private:
    CardInfoPtr card;
    QString manaCost;
    QHBoxLayout *layout;
};

#endif // COLOR_IDENTITY_WIDGET_H
