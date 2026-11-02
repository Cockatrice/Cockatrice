#include "mana_symbol_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../pixel_map_generator.h"

#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QSharedPointer>
#include <QSize>
#include <QStyle>
#include <QStyleOptionFocusRect>
#include <QtGlobal>
#include <libcockatrice/settings/visual_deck_storage_settings.h>
#include <utility>

class QWidget;

ManaSymbolWidget::ManaSymbolWidget(QWidget *parent, QString _symbol, bool _isActive, bool _mayBeToggled)
    : QLabel(parent), symbol(std::move(_symbol)), isActive(_isActive), mayBeToggled(_mayBeToggled)
{
    setPixmap(ManaSymbolPixmapGenerator::generatePixmap(symbol, QSize(50, 50)));
    setMaximumWidth(50);
    if (mayBeToggled) {
        setFocusPolicy(Qt::StrongFocus);
        setCursor(Qt::PointingHandCursor);
    }

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
    if (mayBeToggled) {
        setAccessibleDescription(isActive ? tr("Selected") : tr("Not selected"));
    }
}

void ManaSymbolWidget::mousePressEvent(QMouseEvent *event)
{
    if (mayBeToggled && event->button() == Qt::LeftButton) {
        toggleSymbol();
        event->accept();
        return;
    }
    QLabel::mousePressEvent(event);
}

void ManaSymbolWidget::keyPressEvent(QKeyEvent *event)
{
    if (mayBeToggled &&
        (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        if (!event->isAutoRepeat()) {
            toggleSymbol();
        }
        event->accept();
        return;
    }
    QLabel::keyPressEvent(event);
}

void ManaSymbolWidget::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if (mayBeToggled && hasFocus()) {
        QPainter painter(this);
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.rect = rect().adjusted(1, 1, -1, -1);
        option.backgroundColor = palette().color(QPalette::Window);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void ManaSymbolWidget::focusInEvent(QFocusEvent *event)
{
    QLabel::focusInEvent(event);
    update();
}

void ManaSymbolWidget::focusOutEvent(QFocusEvent *event)
{
    QLabel::focusOutEvent(event);
    update();
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
