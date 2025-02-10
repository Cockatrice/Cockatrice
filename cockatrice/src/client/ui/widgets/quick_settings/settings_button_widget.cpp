#include "settings_button_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScreen>

SettingsButtonWidget::SettingsButtonWidget(QWidget *parent)
    : QWidget(parent), button(new QToolButton(this)), popup(new SettingsPopupWidget(this))
{

    button->setIcon(QPixmap("theme:icons/cogwheel"));
    button->setCheckable(true);
    button->setFixedSize(32, 32);
    connect(button, &QToolButton::clicked, this, &SettingsButtonWidget::togglePopup);
    connect(popup, &SettingsPopupWidget::aboutToClose, this, &SettingsButtonWidget::onPopupClosed);

    layout = new QHBoxLayout(this);
    layout->addWidget(button);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void SettingsButtonWidget::addSettingsWidget(QWidget *toAdd) const
{
    popup->addSettingsWidget(toAdd);
}

void SettingsButtonWidget::togglePopup()
{
    if (popup->isVisible()) {
        popup->close();
    } else {
        // Ensure popup size is known before positioning
        popup->adjustSize();
        QSize popupSize = popup->size();

        // Get button position
        QPoint buttonGlobalPos = button->mapToGlobal(QPoint(0, button->height()));

        // Get screen geometry
        QScreen *screen = QApplication::screenAt(buttonGlobalPos);
        QRect screenGeom = screen ? screen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();

        int x = buttonGlobalPos.x();
        int y = buttonGlobalPos.y();

        // Adjust X position if popup overflows the right side of the screen
        if (x + popupSize.width() > screenGeom.right()) {
            x = buttonGlobalPos.x() - popupSize.width() + button->width(); // Move left
        }

        // Adjust Y position if popup overflows the bottom of the screen
        if (y + popupSize.height() > screenGeom.bottom()) {
            y = buttonGlobalPos.y() - popupSize.height() - button->height(); // Move up
        }

        popup->move(x, y);
        popup->show();
        popup->setFocus();
        button->setChecked(true); // Ensure button is checked when popup is visible
    }
}

void SettingsButtonWidget::onPopupClosed() const
{
    button->setChecked(false); // Ensure button unchecks when popup closes
}

void SettingsButtonWidget::mousePressEvent(QMouseEvent *event)
{
    if (popup->isVisible() && !popup->geometry().contains(event->globalPosition().toPoint())) {
        popup->close();
    }
    QWidget::mousePressEvent(event);
}
