#include "settings_popup_widget.h"

#include <QApplication>
#include <QFocusEvent>
#include <QPainter>

SettingsPopupWidget::SettingsPopupWidget(QWidget *parent) : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
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

void SettingsPopupWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::gray);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    QWidget::paintEvent(event);
}