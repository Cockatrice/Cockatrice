#ifndef MODERATION_HEADER_STYLE_H
#define MODERATION_HEADER_STYLE_H

#include <QProxyStyle>
#include <qtmetamacros.h>

/**
 * Flat header sections for the moderation tables (report queue, user alts).
 * Replaces the native gradient header with the palette Window background, a
 * 1px Mid bottom rule, and WindowText labels, so headers follow the active
 * theme. Sort indicators are still drawn by the base style. Headers with
 * icons fall back to the base style untouched.
 */
class ModerationHeaderStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit ModerationHeaderStyle(QStyle *base = nullptr);
    void drawControl(ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;
};

#endif // MODERATION_HEADER_STYLE_H
