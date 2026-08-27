#include "mana_symbol_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../pixel_map_generator.h"

#include <QResizeEvent>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

ManaSymbolWidget::ManaSymbolWidget(QWidget *parent, QString _symbol, bool _isActive, bool _mayBeToggled)
    : QLabel(parent), symbol(std::move(_symbol)), isActive(_isActive), mayBeToggled(_mayBeToggled)
{
    setPixmap(ManaSymbolPixmapGenerator::generatePixmap(symbol, QSize(50, 50)));
    setMaximumWidth(50);

    // Initialize opacity effect
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    updateOpacity();

    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageUnusedColorIdentitiesOpacityChanged, this,
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
        opacity =
            isActive
                ? 1.0
                : SettingsCache::instance().visualDeckStorage().getVisualDeckStorageUnusedColorIdentitiesOpacity() /
                      100.0;
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
    const QSize newSize = event->size();

    // Skip the rescale when the size didn't actually change: layout passes resize these
    // widgets repeatedly with identical sizes.
    if (newSize.isEmpty() || pixmap().size() == newSize) {
        return;
    }

    setPixmap(ManaSymbolPixmapGenerator::generatePixmap(symbol, newSize));
}
