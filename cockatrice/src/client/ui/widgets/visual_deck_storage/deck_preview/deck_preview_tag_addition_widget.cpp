#include "deck_preview_tag_addition_widget.h"

#include "deck_preview_tag_dialog.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>

DeckPreviewTagAdditionWidget::DeckPreviewTagAdditionWidget(DeckPreviewDeckTagsDisplayWidget *_parent,
                                                           const QString &_tagName)
    : QWidget(_parent), parent(_parent), tagName_(_tagName)
{
    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    // Adjust widget size
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

QSize DeckPreviewTagAdditionWidget::sizeHint() const
{
    // Calculate the size based on the tag name
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(tagName_);
    int width = textWidth + 50;    // Add extra padding
    int height = fm.height() + 10; // Height based on font size + padding

    return QSize(width, height);
}

void DeckPreviewTagAdditionWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit tagClicked();
    }
    QWidget::mousePressEvent(event);
    QStringList knownTags = parent->parent->parent->gatherAllTagsFromFlowWidget();
    QStringList activeTags = parent->deckLoader->getTags();

    DeckPreviewTagDialog dialog(knownTags, activeTags);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList updatedTags = dialog.getActiveTags();
        parent->deckLoader->setTags(updatedTags);
        parent->deckLoader->saveToFile(parent->parent->filePath, DeckLoader::CockatriceFormat);
    }
}

void DeckPreviewTagAdditionWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Set background color
    QColor backgroundColor = Qt::lightGray;
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);

    // Draw background
    painter.drawRect(rect());

    // Draw border
    QColor borderColor = Qt::gray;
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(0, 0, -1, -1)); // Adjust for pen width

    // Calculate font size based on widget height
    QFont font = painter.font();
    int fontSize = std::max(10, height() / 2); // Ensure a minimum font size of 10
    font.setPointSize(fontSize);
    painter.setFont(font);

    // Calculate text rect with margin
    int margin = 10; // Left and right margins
    QRect textRect(margin, 0, width() - margin * 2, height());

    // Draw the text with a black border for better legibility
    painter.setPen(Qt::black);

    // Draw text border by offsetting
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {
                painter.drawText(textRect.translated(dx, dy), Qt::AlignLeft | Qt::AlignVCenter, tagName_);
            }
        }
    }

    // Draw the actual text
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, tagName_);

    QWidget::paintEvent(event);
}
