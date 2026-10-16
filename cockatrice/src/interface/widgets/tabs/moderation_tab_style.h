#ifndef MODERATION_TAB_STYLE_H
#define MODERATION_TAB_STYLE_H

#include <QProxyStyle>
#include <qtmetamacros.h>

/**
 * Flat tab shapes for the moderation detail tabs (report queue detailTabs).
 * Replaces the native tab frame with the palette Window background plus a
 * 2px Highlight underline on the selected tab and a faint Highlight wash on
 * hover. Labels (text, elide, focus) are left to the base style, so keyboard
 * focus stays distinguishable.
 */
class ModerationTabStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit ModerationTabStyle(QStyle *base = nullptr);
    void drawControl(ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;
};

#endif // MODERATION_TAB_STYLE_H
