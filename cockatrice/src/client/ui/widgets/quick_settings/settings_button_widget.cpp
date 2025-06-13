#include "settings_button_widget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScreen>

SettingsButtonWidget::SettingsButtonWidget(QWidget *parent)
    : QWidget(parent), button(new QToolButton(this)), popup(new SettingsPopupWidget(nullptr))
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

void SettingsButtonWidget::removeSettingsWidget(QWidget *toRemove) const
{
    popup->removeSettingsWidget(toRemove);
}

void SettingsButtonWidget::setButtonIcon(QPixmap iconMap)
{
    button->setIcon(iconMap);
}

void SettingsButtonWidget::togglePopup()
{
    if (popup->isVisible()) {
        popup->close();
    } else {
        popup->adjustSizeToFitScreen(); // Ensure proper size

        QSize popupSize = popup->size();
        QPoint buttonGlobalPos = button->mapToGlobal(QPoint(0, button->height()));

        QScreen *screen = QApplication::screenAt(buttonGlobalPos);
        QRect screenGeom = screen ? screen->availableGeometry() : QApplication::primaryScreen()->availableGeometry();

        int x = buttonGlobalPos.x();
        int y = buttonGlobalPos.y();

        // Adjust position to stay within screen bounds
        if (x + popupSize.width() > screenGeom.right()) {
            x = buttonGlobalPos.x() - popupSize.width() + button->width();
        }

        if (y + popupSize.height() > screenGeom.bottom()) {
            y = buttonGlobalPos.y() - popupSize.height() - button->height();
        }

        popup->move(x, y);
        popup->show();
        popup->setFocus();
        button->setChecked(true);
    }
}

void SettingsButtonWidget::onPopupClosed() const
{
    button->setChecked(false); // Ensure button unchecks when popup closes
}

void SettingsButtonWidget::mousePressEvent(QMouseEvent *event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    if (popup->isVisible() && !popup->geometry().contains(event->globalPosition().toPoint())) {
#else
    if (popup->isVisible() && !popup->geometry().contains(event->globalPos())) {
#endif
        popup->close();
    }
    QWidget::mousePressEvent(event);
}
