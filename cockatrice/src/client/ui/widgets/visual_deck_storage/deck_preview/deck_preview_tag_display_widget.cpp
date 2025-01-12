#include "deck_preview_tag_display_widget.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>

DeckPreviewTagDisplayWidget::DeckPreviewTagDisplayWidget(QWidget *parent, const QString &_tagName)
    : QWidget(parent), tagName(_tagName), isSelected(false)
{
    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    // Add a stretch spacer for text and close button separation
    // layout->addStretch(); // Ensures the close button stays at the far-right side

    // Create close button
    // closeButton = new QPushButton("x", this);
    // closeButton->setFixedSize(16, 16); // Small square button
    // closeButton->setFocusPolicy(Qt::NoFocus);

    // Set font for close button to ensure the "x" appears correctly
    // QFont closeButtonFont = closeButton->font();
    // closeButtonFont.setPointSize(10); // Adjust the size to make the "x" clear
    // closeButton->setFont(closeButtonFont);

    // layout->addWidget(closeButton);

    // Adjust widget size
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Connect close button to the remove signal
    // connect(closeButton, &QPushButton::clicked, this, &DeckPreviewTagDisplayWidget::tagClosed);
}

QSize DeckPreviewTagDisplayWidget::sizeHint() const
{
    // Calculate the size based on the tag name and close button
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(tagName);
    int width = textWidth + 50;    // Add extra padding
    int height = fm.height() + 10; // Height based on font size + padding

    return QSize(width, height);
}

void DeckPreviewTagDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setSelected(!isSelected);
        emit tagClicked();
    }
    QWidget::mousePressEvent(event);
}

void DeckPreviewTagDisplayWidget::setSelected(bool selected)
{
    isSelected = selected;
    update(); // Trigger a repaint
}

void DeckPreviewTagDisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Set background color
    QColor backgroundColor = isSelected ? QColor(173, 216, 230) : Qt::white;
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);

    // Draw background
    painter.drawRect(rect());

    // Draw border
    QColor borderColor = isSelected ? Qt::blue : Qt::gray;
    QPen borderPen(borderColor, isSelected ? 2 : 1);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(0, 0, -1, -1)); // Adjust for pen width

    // Calculate font size based on widget height
    QFont font = painter.font();
    int fontSize = std::max(10, height() / 2); // Ensure a minimum font size of 10
    font.setPointSize(fontSize);
    painter.setFont(font);

    // Calculate text rect to avoid overlap with the close button
    // int closeButtonWidth = closeButton->width();
    int margin = 10; // Left and right margins
    QRect textRect(margin, 0, width() - margin * 2, height());

    // Draw the text with a black border for better legibility
    painter.setPen(Qt::black);

    // Draw text border by offsetting
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                painter.drawText(textRect.translated(dx, dy), Qt::AlignLeft | Qt::AlignVCenter, tagName);
            }
        }
    }

    // Draw the actual text
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, tagName);

    QWidget::paintEvent(event);
}
