#include "compact_push_button.h"

CompactPushButton::CompactPushButton(QWidget *parent) : QPushButton(parent)
{
    setCheckable(true);
    setFixedHeight(32);

    // default sizing
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(this, &QPushButton::clicked, this, [] {
        // your popup logic here
    });
}

void CompactPushButton::setButtonText(const QString &text)
{
    fullText = text;

    if (!compact) {
        setText(fullText);
    }

    updateGeometryState();
}

void CompactPushButton::setButtonIcon(const QIcon &icon)
{
    setIcon(icon);
}

void CompactPushButton::setCompact(bool enabled)
{
    compact = enabled;

    if (compact) {
        setText(QString()); // icon only
    } else {
        setText(fullText);
    }

    updateGeometryState();
}

void CompactPushButton::updateGeometryState()
{
    const int buttonHeight = 32;

    setMinimumHeight(buttonHeight);
    setMaximumHeight(buttonHeight);

    if (compact) {
        setMinimumWidth(buttonHeight);
        setMaximumWidth(buttonHeight);
    } else {
        setMinimumWidth(0);
        setMaximumWidth(QWIDGETSIZE_MAX);
    }

    updateGeometry();
}

int CompactPushButton::expandedWidth() const
{
    QFontMetrics fm(font());

    return fm.horizontalAdvance(fullText) + 48; // icon + padding
}

int CompactPushButton::compactWidth() const
{
    return 32;
}