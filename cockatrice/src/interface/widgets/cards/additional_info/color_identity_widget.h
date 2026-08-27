/**
 * @file color_identity_widget.h
 * @ingroup CardExtraInfoWidgets
 */
//! \todo Document this file.

#ifndef COLOR_IDENTITY_WIDGET_H
#define COLOR_IDENTITY_WIDGET_H

#include <QHBoxLayout>
#include <QWidget>
#include <libcockatrice/card/card_info.h>

class ColorIdentityWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorIdentityWidget(QWidget *parent, const QString &_colorIdentity = "");
    explicit ColorIdentityWidget(QWidget *parent, const CardInfoPtr &card);

    void populateManaSymbolWidgets();

    static QStringList parseColorIdentity(const QString &manaString);

public slots:
    void setColorIdentity(const QString &_colorIdentity);
    void resizeEvent(QResizeEvent *event) override;
    void toggleUnusedVisibility();

private:
    QString colorIdentity;
    QHBoxLayout *layout;
    int lastIconSize = -1; ///< The symbol size last applied, to skip redundant resize passes.
    int lastWidth = -1;    ///< The width last processed, to skip redundant resize passes.
};

#endif // COLOR_IDENTITY_WIDGET_H
