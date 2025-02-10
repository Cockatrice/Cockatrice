#include "settings_popup_widget.h"

#include <QApplication>
#include <QFocusEvent>

SettingsPopupWidget::SettingsPopupWidget(QWidget *parent) : QWidget(parent, Qt::Popup)
{
    layout = new QVBoxLayout(this);
}

void SettingsPopupWidget::addSettingsWidget(QWidget *toAdd) const
{
    layout->addWidget(toAdd);
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
