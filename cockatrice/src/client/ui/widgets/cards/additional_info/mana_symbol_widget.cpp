#include "mana_symbol_widget.h"

#include <QResizeEvent>

ManaSymbolWidget::ManaSymbolWidget(QWidget *parent, QString _symbol, bool _isActive, bool _mayBeToggled)
    : QLabel(parent), symbol(_symbol), isActive(_isActive), mayBeToggled(_mayBeToggled)
{
    loadManaIcon();
    setPixmap(manaIcon.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setMaximumWidth(50);

    // Initialize opacity effect
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    updateOpacity();
}

void ManaSymbolWidget::setColorActive(bool active)
{
    if (isActive != active) {
        isActive = active;
        updateOpacity();
        emit colorToggled(getSymbolChar(), isActive);
    }
}

void ManaSymbolWidget::updateOpacity()
{
    qreal opacity = isActive ? 1.0 : 0.5;
    opacityEffect->setOpacity(opacity);
}

void ManaSymbolWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (mayBeToggled) {
        isActive = !isActive;
        updateOpacity();
        emit colorToggled(getSymbolChar(), isActive);
    }
}

void ManaSymbolWidget::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    setPixmap(manaIcon.scaled(event->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void ManaSymbolWidget::loadManaIcon()
{
    QString filename = "theme:icons/mana/";

    if (symbol == "W" || symbol == "U" || symbol == "B" || symbol == "R" || symbol == "G") {
        filename += symbol + ".svg";
    }

    manaIcon = QPixmap(filename);
}
