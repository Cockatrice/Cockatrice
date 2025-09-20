#include "settings_popup_widget.h"

#include <QApplication>
#include <QFocusEvent>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>

SettingsPopupWidget::SettingsPopupWidget(QWidget *parent) : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    // Main layout for the popup itself
    layout = new QVBoxLayout(this);

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
            scrollArea = new QScrollArea(this);
            scrollArea->setWidgetResizable(true);
            scrollArea->setFrameShape(QFrame::NoFrame);
            scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // No horizontal scrollbar
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    // Enable vertical scrollbar when needed
        }

        // Set maximum height for the scroll area and show vertical scrollbar
        scrollArea->setMaximumHeight(maxHeight);

        // Resize the popup widget without squishing the content
        resize(sizeHint().width(), maxHeight); // Ensure content width is kept intact

        // Add scrollArea to layout if not already added
        if (layout->count() == 1) { // We only have one widget (containerWidget) at the start
            layout->addWidget(scrollArea);
        }

        // Set the scroll area widget
        scrollArea->setWidget(containerWidget);
    } else {
        // If the scroll area exists, remove it
        if (scrollArea) {
            layout->removeWidget(scrollArea);
            delete scrollArea;
            scrollArea = nullptr; // Reset the pointer
        }

        // Set the containerWidget directly without scrollArea and adjust its height
        resize(sizeHint().width(), contentHeight); // Resize the widget based on content height
        layout->addWidget(containerWidget);        // Re-add the containerWidget without scroll area
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
