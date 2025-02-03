#include "deck_preview_color_identity_widget.h"

#include "../../../../../settings/cache_settings.h"

#include <QPainter>
#include <QResizeEvent>

DeckPreviewColorCircleWidget::DeckPreviewColorCircleWidget(QChar color, QWidget *parent)
    : QWidget(parent), colorChar(color), circleDiameter(0), isActive(false)
{
}

void DeckPreviewColorCircleWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Get the parent of the DeckPreviewColorIdentityWidget
    QWidget *identityParent = parentWidget() ? parentWidget()->parentWidget() : nullptr;
    if (identityParent) {
        // Calculate the circle diameter as 15% of the parent's height
        int maxSize = identityParent->width() * 0.15;
        circleDiameter = maxSize;

        // Update the widget size based on the diameter
        updateGeometry(); // Request a resize based on sizeHint()
    }

    update(); // Trigger a repaint
}

QSize DeckPreviewColorCircleWidget::sizeHint() const
{
    // Return the size we calculated based on the parent widget
    return QSize(circleDiameter, circleDiameter);
}

QSize DeckPreviewColorCircleWidget::minimumSizeHint() const
{
    // Return the same value as sizeHint() for minimum size
    return QSize(circleDiameter, circleDiameter);
}

void DeckPreviewColorCircleWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate the circle's bounding rectangle
    int x = (width() - circleDiameter) / 2;
    int y = (height() - circleDiameter) / 2;
    QRect circleRect(x, y, circleDiameter, circleDiameter);

    // Map color characters to their respective colors
    QColor circleColor;
    switch (colorChar.unicode()) {
        case 'W':
            circleColor = Qt::white;
            break;
        case 'U':
            circleColor = QColor(0, 115, 230);
            break; // Blue
        case 'B':
            circleColor = QColor(50, 50, 50);
            break; // Black
        case 'R':
            circleColor = QColor(230, 30, 30);
            break; // Red
        case 'G':
            circleColor = QColor(30, 180, 30);
            break; // Green
        default:
            circleColor = Qt::transparent;
            break; // Fallback
    }

    if (SettingsCache::instance().getVisualDeckStorageDrawUnusedColorIdentities() || isActive) {
        // Make the circle faint if it is not active
        if (!isActive) {
            circleColor.setAlpha(SettingsCache::instance().getVisualDeckStorageUnusedColorIdentitiesOpacity() / 100.0 *
                                 255.0);
        }

        // Draw the circle
        painter.setBrush(circleColor);
        painter.setPen(Qt::black);
        painter.drawEllipse(circleRect);
    }

    // Draw the color character only if the circle is active
    if (isActive) {
        QFont font = painter.font();
        font.setBold(true);
        font.setPointSize(circleDiameter * 0.4); // Adjust font size relative to circle diameter
        painter.setFont(font);
        if (colorChar.unicode() == 'B') {
            painter.setPen(Qt::white);
        } else {
            painter.setPen(Qt::black);
        }

        // Center the text within the circle
        painter.drawText(circleRect, Qt::AlignCenter, colorChar);
    }
}

void DeckPreviewColorCircleWidget::setColorActive(bool active)
{
    isActive = active;
    update(); // Redraw the circle with the new active state
}

QChar DeckPreviewColorCircleWidget::getColorChar() const
{
    return colorChar;
}

DeckPreviewColorIdentityWidget::DeckPreviewColorIdentityWidget(QWidget *parent, const QString &colorIdentity)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 3, 0, 0);
    setLayout(layout);

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Define the full WUBRG set (White, Blue, Black, Red, Green)
    QString fullColorIdentity = "WUBRG";

    // Create and add a DeckPreviewColorCircleWidget for each color in WUBRG
    for (const QChar &color : fullColorIdentity) {
        auto *circle = new DeckPreviewColorCircleWidget(color, this);
        layout->addWidget(circle);
    }

    // Set any active colors from the input colorIdentity
    for (const QChar &color : colorIdentity) {
        for (DeckPreviewColorCircleWidget *circle : findChildren<DeckPreviewColorCircleWidget *>()) {
            if (circle->getColorChar() == color) {
                circle->setColorActive(true); // Mark the color as active
            }
        }
    }
}

void DeckPreviewColorIdentityWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Notify child widgets to update their sizes based on the new parent size
    for (auto *circle : findChildren<DeckPreviewColorCircleWidget *>()) {
        circle->updateGeometry(); // Request each circle to resize
    }
}
