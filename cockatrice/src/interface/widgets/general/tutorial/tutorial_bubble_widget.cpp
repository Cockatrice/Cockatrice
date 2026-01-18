#include "tutorial_bubble_widget.h"

BubbleWidget::BubbleWidget(QWidget *parent) : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setStyleSheet("QFrame { background:white; border-radius:8px; }"
                  "QLabel { color:black; }");

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(8);

    counterLabel = new QLabel(this);
    counterLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    textLabel = new QLabel(this);
    textLabel->setWordWrap(true);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    textLabel->setStyleSheet("color:black;"); // guard against global styles

    // Layout
    layout->addWidget(counterLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1);
    layout->addWidget(textLabel, 1, 0, 1, 3);

    // Make column 1 take extra space so text gets room to expand/wrap
    layout->setColumnStretch(1, 1);

    // sensible default maximum width for bubble so text will wrap
    setMaximumWidth(420);
}

void BubbleWidget::setText(const QString &text)
{
    textLabel->setText(text);
    update();
}