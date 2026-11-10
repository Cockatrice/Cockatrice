#include "settings_popup_widget.h"

#include <QApplication>
#include <QFrame>
#include <QPainter>
#include <QRect>
#include <QScreen>
#include <QScrollArea>
#include <QSize>
#include <QVBoxLayout>
#include <libcockatrice/utility/qt_utils.h>
#include <qnamespace.h>

SettingsPopupWidget::SettingsPopupWidget(QWidget *parent) : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    // Main layout for the popup itself
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);

    // Container for the content (with or without scroll area)
    containerWidget = new QWidget();
    containerLayout = new QVBoxLayout(containerWidget); // Store a separate layout
    containerWidget->setLayout(containerLayout);        // Make sure containerWidget has its layout

    // Add the container widget directly to the layout initially
    layout->addWidget(containerWidget); // Initially, we add the containerWidget without a scroll area

    setLayout(layout);

    scrollArea = nullptr; // Initialize scrollArea pointer to null
}

void SettingsPopupWidget::addSettingsWidget(QWidget *toAdd) const
{
    containerLayout->addWidget(toAdd); // Add to containerWidget's layout
}

void SettingsPopupWidget::removeSettingsWidget(QWidget *toRemove) const
{
    containerLayout->removeWidget(toRemove);
    toRemove->deleteLater();
}

void SettingsPopupWidget::adjustSizeToFitScreen()
{
    QScreen *screen = QApplication::screenAt(this->pos());
    QRect screenGeom = screen ? screen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();
    int maxHeight = screenGeom.height() / 2; // Limit height to 50% of screen

    // Adjust the container widget's size hint to get the actual size of content
    containerWidget->adjustSize();
    int contentHeight = containerWidget->sizeHint().height();

    // If content height exceeds maxHeight, we need to scroll
    if (contentHeight > maxHeight) {
        // Create a scroll area and add the container widget to it if not already created
        if (!scrollArea) {
            scrollArea = QtUtils::makeScrollArea(this, Qt::ScrollBarAlwaysOn);
            scrollArea->setFrameShape(QFrame::NoFrame);
        }

        // Set maximum height for the scroll area and show vertical scrollbar
        scrollArea->setMaximumHeight(maxHeight);

        // Resize the popup widget without squishing the content
        resize(sizeHint().width(), maxHeight); // Ensure content width is kept intact

        // Add scrollArea to layout if not already added
        if (layout->indexOf(scrollArea) == -1) {
            layout->addWidget(scrollArea);
        }

        // Set the scroll area widget; it takes ownership of containerWidget until handed back above
        if (scrollArea->widget() != containerWidget) {
            layout->removeWidget(containerWidget);
            scrollArea->setWidget(containerWidget);
        }
    } else {
        // If the scroll area exists, remove it. QScrollArea owns the widget set on it and would
        // delete containerWidget (and every settings widget inside it) along with itself, so take
        // the container back before deleting the scroll area.
        if (scrollArea) {
            if (scrollArea->widget() == containerWidget) {
                scrollArea->takeWidget();
                containerWidget->setParent(this);
            }
            layout->removeWidget(scrollArea);
            delete scrollArea;
            scrollArea = nullptr; // Reset the pointer
        }

        // Set the containerWidget directly without scrollArea and adjust its height
        resize(sizeHint().width(), contentHeight); // Resize the widget based on content height
        if (layout->indexOf(containerWidget) == -1) {
            layout->addWidget(containerWidget); // Re-add the containerWidget without scroll area
        }
        containerWidget->show();
    }

    // Ensure layout updates after changes
    layout->update(); // Update layout for proper size fitting
    updateGeometry(); // Update widget geometry to reflect changes
}

void SettingsPopupWidget::resizeEvent(QResizeEvent *event)
{
    // Make sure the scroll area and popup adjust to new size constraints
    adjustSizeToFitScreen();
    QWidget::resizeEvent(event);
}

void SettingsPopupWidget::focusOutEvent(QFocusEvent *event)
{
    if (!this->isAncestorOf(QApplication::focusWidget())) {
        close();
    }
    QWidget::focusOutEvent(event);
}

void SettingsPopupWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
    QWidget::closeEvent(event);
}

void SettingsPopupWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::gray);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    QWidget::paintEvent(event);
}
