#include "mana_symbol_widget.h"

#include "../../../../client/settings/cache_settings.h"

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

    connect(&SettingsCache::instance(), &SettingsCache::visualDeckStorageUnusedColorIdentitiesOpacityChanged, this,
            &ManaSymbolWidget::updateOpacity);
}

void ManaSymbolWidget::toggleSymbol()
{
    setColorActive(!isActive);
    emit colorToggled(getSymbolChar(), isActive);
}

void ManaSymbolWidget::setColorActive(bool active)
{
    if (isActive != active) {
        isActive = active;
        updateOpacity();
    }
}

void ManaSymbolWidget::updateOpacity()
{
    qreal opacity;
    if (mayBeToggled) {
        // UI elements that users can click on shouldn't be transparent.
        opacity = isActive ? 1.0 : 0.5;
    } else {
        // It's just for display, they can do whatever they want.
        opacity = isActive ? 1.0 : SettingsCache::instance().getVisualDeckStorageUnusedColorIdentitiesOpacity() / 100.0;
    }
    opacityEffect->setOpacity(opacity);
}

void ManaSymbolWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (mayBeToggled) {
        toggleSymbol();
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
        filename += symbol;
    }

    manaIcon = QPixmap(filename);
}
