#include "mana_symbol_widget.h"

#include "../../../../client/settings/cache_settings.h"

#include <QHash>
#include <QImageReader>
#include <QResizeEvent>
#include <libcockatrice/settings/visual_deck_storage_settings.h>

namespace
{
/// Longest side mana symbols are rendered at before being scaled to their final size.
constexpr int MASTER_ICON_SIZE = 128;

QHash<QString, QPixmap> &scaledIconCache()
{
    static QHash<QString, QPixmap> cache;
    return cache;
}

/**
 * @brief Renders \a symbol once at a fixed moderate size, so repeated scalings never
 * re-rasterize the source file (SVG sources can be very expensive to rasterize).
 */
const QPixmap &masterIcon(const QString &symbol)
{
    static QHash<QString, QPixmap> cache;
    auto it = cache.constFind(symbol);
    if (it != cache.constEnd()) {
        return it.value();
    }

    QImageReader reader("theme:icons/mana/" + symbol);
    QSize sourceSize = reader.size();
    if (!sourceSize.isEmpty()) {
        sourceSize.scale(QSize(MASTER_ICON_SIZE, MASTER_ICON_SIZE), Qt::KeepAspectRatio);
        reader.setScaledSize(sourceSize);
    }
    const QPixmap rendered = QPixmap::fromImageReader(&reader);

    return cache.insert(symbol, rendered).value();
}

QString cacheKey(const QString &symbol, const QSize &size)
{
    return symbol + QLatin1Char('|') + QString::number(size.width()) + QLatin1Char('x') +
           QString::number(size.height());
}
} // namespace

ManaSymbolWidget::ManaSymbolWidget(QWidget *parent, QString _symbol, bool _isActive, bool _mayBeToggled)
    : QLabel(parent), symbol(std::move(_symbol)), isActive(_isActive), mayBeToggled(_mayBeToggled)
{
    setPixmap(getCachedScaledIcon(symbol, QSize(50, 50)));
    setMaximumWidth(50);

    // Initialize opacity effect
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    updateOpacity();

    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageUnusedColorIdentitiesOpacityChanged, this,
            &ManaSymbolWidget::updateOpacity);
}

QPixmap ManaSymbolWidget::getCachedScaledIcon(const QString &symbol, const QSize &size)
{
    const QString key = cacheKey(symbol, size);
    auto it = scaledIconCache().constFind(key);
    if (it != scaledIconCache().constEnd()) {
        return it.value();
    }

    const QPixmap &icon = masterIcon(symbol);
    if (icon.isNull()) {
        return {};
    }

    QPixmap scaled = icon.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledIconCache().insert(key, scaled);
    return scaled;
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

    setPixmap(getCachedScaledIcon(symbol, newSize));
}
