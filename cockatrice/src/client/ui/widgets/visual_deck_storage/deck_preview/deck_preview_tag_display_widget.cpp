#include "deck_preview_tag_display_widget.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>

DeckPreviewTagDisplayWidget::DeckPreviewTagDisplayWidget(QWidget *parent, const QString &_tagName)
    : QWidget(parent), tagName(_tagName), state(TagState::NotSelected)
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
    switch (event->button()) {
        case Qt::LeftButton:
            setState(state != TagState::Selected ? TagState::Selected : TagState::NotSelected);
            break;
        case Qt::RightButton:
            setState(state != TagState::Excluded ? TagState::Excluded : TagState::NotSelected);
            break;
        case Qt::MiddleButton:
            setState(TagState::NotSelected);
            break;
        default:
            break;
    }

    emit tagClicked();
    QWidget::mousePressEvent(event);
}

void DeckPreviewTagDisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QColor backgroundColor;
    QColor borderColor;
    int borderWidth;

    switch (state) {
        case TagState::Selected:
            backgroundColor = QColor(173, 216, 230); // Light blue
            borderColor = Qt::blue;
            borderWidth = 2;
            break;
        case TagState::Excluded:
            backgroundColor = QColor(255, 182, 193); // Light red/pink
            borderColor = Qt::red;
            borderWidth = 2;
            break;
        case TagState::NotSelected:
        default:
            backgroundColor = Qt::white;
            borderColor = Qt::gray;
            borderWidth = 1;
            break;
    }

    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    QPen borderPen(borderColor, borderWidth);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

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
