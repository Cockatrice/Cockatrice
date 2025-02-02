#include "banner_widget.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

BannerWidget::BannerWidget(QWidget *parent, const QString &text, Qt::Orientation orientation, int transparency)
    : QWidget(parent), gradientOrientation(orientation), transparency(qBound(0, transparency, 100))
{
    // Create the banner label and set properties
    bannerLabel = new QLabel(text, this);
    bannerLabel->setAlignment(Qt::AlignCenter);
    bannerLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

    // Layout to center the banner label
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(bannerLabel);
    layout->setContentsMargins(0, 20, 0, 20); // Space for the gradient
    setLayout(layout);

    // Set minimum height for the widget
    setMinimumHeight(100);
    connect(this, &BannerWidget::buddyVisibilityChanged, this, &BannerWidget::toggleBuddyVisibility);
}

void BannerWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (clickable) {
        emit buddyVisibilityChanged();
    }
}

void BannerWidget::setText(const QString &text) const
{
    bannerLabel->setText(text);
}

void BannerWidget::toggleBuddyVisibility() const
{
    if (buddy) {
        buddy->setVisible(!buddy->isVisible());
    }
}

void BannerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    // Calculate alpha based on transparency percentage
    int alpha = (255 * transparency) / 100;

    // Determine gradient direction
    QLinearGradient gradient;
    if (gradientOrientation == Qt::Vertical) {
        gradient = QLinearGradient(rect().topLeft(), rect().bottomLeft());
    } else {
        gradient = QLinearGradient(rect().topLeft(), rect().topRight());
    }

    // Set neutral gradient colors with calculated transparency
    gradient.setColorAt(0, QColor(200, 200, 200, alpha));       // Light grey with alpha
    gradient.setColorAt(1, QColor(100, 100, 100, alpha / 1.5)); // Darker grey, slightly more transparent

    // Fill the widget background with the gradient
    painter.fillRect(rect(), gradient);
}
