#include "deck_preview_deck_box_widget.h"

#include "../../../../client/settings/cache_settings.h"
#include "../../../pixel_map_generator.h"
#include "../../../theme_manager.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/card_info_picture_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "libcockatrice/settings/visual_deck_storage_settings.h"
#include "libcockatrice/utility/color.h"
#include "libcockatrice/utility/qt_utils.h"
#include "libcockatrice/utility/raise_on_hover.h"

#include <QEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QStringList>
#include <algorithm>

namespace
{
/// How far the box lifts while hovered, and how long that takes.
constexpr int RAISE_OFFSET_PX = 10;
constexpr int RAISE_DURATION_MS = 200;

/// Chrome proportions, as fractions of the box width, so the box keeps its shape
/// at every card size. The lids are clamped so a tiny tile keeps a readable band
/// and a huge one keeps a slim one.
constexpr qreal LID_HEIGHT_RATIO = 0.13;
constexpr qreal SIDE_INSET_RATIO = 0.035;
constexpr qreal LID_GAP_RATIO = 0.03;
constexpr qreal BOTTOM_INSET_RATIO = 0.04;
constexpr qreal CORNER_RADIUS_RATIO = 0.04;
constexpr qreal NAME_FONT_RATIO = 0.048;

/// The mana band under the name: one continuous strip split into equal colour
/// segments. Its thickness is the tile's chip measurement, so it reads as a line
/// rather than a row of discs.
constexpr qreal MANA_BAND_HEIGHT_RATIO = 0.09;
/// Share of the band's thickness the icon is inset by, so the ring around the rune
/// keeps its proportion to the band at every card size. A fraction of the tile width
/// would grow the ring while the band stays thin, and swallow the rune at 250 %.
constexpr qreal MANA_ICON_INSET_RATIO = 0.11;
/// How far a colour boundary leans over the band's full thickness. A share of the band
/// keeps the angle constant at every card size, so the band never flattens into columns.
constexpr qreal MANA_SEGMENT_SLANT_RATIO = 0.55;

/// Each segment is the mana colour pulled down a step, so the icon's own light disc
/// reads against it instead of merging with the lid.
constexpr int MANA_BAND_DARKEN = 125;
} // namespace

DeckPreviewDeckBoxWidget::DeckPreviewDeckBoxWidget(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // The box is the tile's control surface, so it has to be reachable with the
    // keyboard: see keyPressEvent for the Space / Enter mapping.
    setFocusPolicy(Qt::StrongFocus);

    // The picture is the window contents: no overlay text (the lid carries the
    // name) and no raise of its own (the box lifts as one surface). Its base class
    // reserves a minimum height for a full card, which would refuse the smaller
    // card sizes, so the window geometry wins.
    picture = new DeckPreviewCardPictureWidget(this, false, false);
    picture->setMinimumSize(0, 0);

    raiseOnHover = new RaiseOnHover(this, RAISE_OFFSET_PX, RAISE_DURATION_MS);

    // The name is painted with the theme's interface font, so a font change is a repaint.
    connect(themeManager, &ThemeManager::fontsChanged, this, [this] { update(); });

    // The band answers two settings live: how many segments it has and how far it dims
    // the unused ones. Both are paint-time decisions, but the segment count also moves
    // the lid's bottom edge, so the artwork window is laid out again.
    const auto relayout = [this] {
        layoutPicture();
        update();
    };
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageDrawUnusedColorIdentitiesChanged, this, relayout);
    connect(&SettingsCache::instance().visualDeckStorage(),
            &VisualDeckStorageSettings::visualDeckStorageUnusedColorIdentitiesOpacityChanged, this, relayout);

    setScaleFactor(scaleFactor);
}

void DeckPreviewDeckBoxWidget::setDeckName(const QString &name)
{
    if (deckName == name) {
        return;
    }
    deckName = name;
    setAccessibleName(name);
    update();
}

void DeckPreviewDeckBoxWidget::setCardCount(int count)
{
    if (cardCount == count) {
        return;
    }
    cardCount = count;
    updateAccessibleDescription();
    update();
}

void DeckPreviewDeckBoxWidget::setColorIdentity(const QString &identity)
{
    if (colorIdentity == identity) {
        return;
    }
    colorIdentity = identity;
    updateAccessibleDescription();
    // With the unused colours shown the band keeps all five segments whatever the
    // identity is, so only a band that follows the identity can move the lid's bottom
    // edge and the artwork window with it.
    layoutPicture();
    update();
}

void DeckPreviewDeckBoxWidget::setColorIdentityVisible(bool visible)
{
    if (colorIdentityVisible == visible) {
        return;
    }
    colorIdentityVisible = visible;
    updateAccessibleDescription();
    // The lid grew or shrank, so the artwork window has to follow it before the repaint.
    layoutPicture();
    update();
}

void DeckPreviewDeckBoxWidget::setSelected(bool _selected)
{
    if (selected == _selected) {
        return;
    }
    selected = _selected;
    updateAccessibleDescription();
    update();
}

void DeckPreviewDeckBoxWidget::setRaiseOnEnterEnabled(bool enabled)
{
    raiseOnHover->setEnabled(enabled);
}

void DeckPreviewDeckBoxWidget::setScaleFactor(int scale)
{
    scaleFactor = scale;
    setFixedSize(CardInfoPictureWidget::sizeForScaleFactor(scale));
}

QSize DeckPreviewDeckBoxWidget::sizeHint() const
{
    return CardInfoPictureWidget::sizeForScaleFactor(scaleFactor);
}

int DeckPreviewDeckBoxWidget::nameRowHeight() const
{
    return std::clamp(qRound(width() * LID_HEIGHT_RATIO), 14, 34);
}

int DeckPreviewDeckBoxWidget::lidPadding() const
{
    return std::max(4, qRound(width() * SIDE_INSET_RATIO));
}

int DeckPreviewDeckBoxWidget::manaBandHeight() const
{
    return std::clamp(qRound(width() * MANA_BAND_HEIGHT_RATIO), 10, 20);
}

QStringList DeckPreviewDeckBoxWidget::manaSymbols() const
{
    // The five colours are the band's whole vocabulary: the identity only says which of
    // them the deck has, so any leftover without a mana icon is simply not shown.
    static const QStringList wubrg = {QStringLiteral("W"), QStringLiteral("U"), QStringLiteral("B"),
                                      QStringLiteral("R"), QStringLiteral("G")};

    if (!colorIdentityVisible) {
        return {};
    }

    if (SettingsCache::instance().visualDeckStorage().getVisualDeckStorageDrawUnusedColorIdentities()) {
        return wubrg;
    }

    // With the unused colours hidden the band keeps its full width and shares it
    // between the colours the deck has, so it stays one continuous line either way.
    const QStringList identity = ColorIdentityWidget::parseColorIdentity(colorIdentity);
    QStringList used;
    for (const QString &symbol : wubrg) {
        if (identity.contains(symbol)) {
            used.append(symbol);
        }
    }
    return used;
}

int DeckPreviewDeckBoxWidget::lidHeight() const
{
    const int nameRow = nameRowHeight();
    // The band ends the lid: no padding under it, so the colour line itself is the
    // boundary. A deck with nothing to draw keeps the single-row lid.
    return nameRow + (manaSymbols().isEmpty() ? 0 : manaBandHeight());
}

int DeckPreviewDeckBoxWidget::sideInset() const
{
    return std::max(3, qRound(width() * SIDE_INSET_RATIO));
}

int DeckPreviewDeckBoxWidget::lidGap() const
{
    return std::max(2, qRound(width() * LID_GAP_RATIO));
}

int DeckPreviewDeckBoxWidget::bottomInset() const
{
    return std::max(3, qRound(width() * BOTTOM_INSET_RATIO));
}

qreal DeckPreviewDeckBoxWidget::cornerRadius() const
{
    return std::max<qreal>(2.0, width() * CORNER_RADIUS_RATIO);
}

QRect DeckPreviewDeckBoxWidget::windowArea() const
{
    return rect().adjusted(sideInset(), lidHeight() + lidGap(), -sideInset(), -bottomInset());
}

QRect DeckPreviewDeckBoxWidget::artRect() const
{
    const QRect area = windowArea();
    if (area.width() <= 0 || area.height() <= 0) {
        return {};
    }

    int artHeight = area.height();
    int artWidth = qRound(artHeight / CardInfoPictureWidget::ASPECT_RATIO);
    if (artWidth > area.width()) {
        artWidth = area.width();
        artHeight = qRound(artWidth * CardInfoPictureWidget::ASPECT_RATIO);
    }

    return {area.x() + (area.width() - artWidth) / 2, area.y() + (area.height() - artHeight) / 2, artWidth, artHeight};
}

void DeckPreviewDeckBoxWidget::layoutPicture()
{
    picture->setGeometry(artRect());
}

void DeckPreviewDeckBoxWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutPicture();
}

QFont DeckPreviewDeckBoxWidget::nameFont() const
{
    QFont font = QtUtils::boldFont(themeManager->appFont(AppFont::InterfaceDefault));
    // Pixel sizes keep the lid typography tied to the tile size rather than to the
    // system point size, which is what makes the band readable at 50 % and sleek at 250 %.
    font.setPixelSize(std::clamp(qRound(width() * NAME_FONT_RATIO), 8, 20));
    return font;
}

void DeckPreviewDeckBoxWidget::updateAccessibleDescription()
{
    QStringList parts;
    if (selected) {
        parts << tr("Selected");
    }
    if (cardCount >= 0) {
        parts << tr("%n card(s)", "", cardCount);
    }
    // The identity used to be its own widget row and is now a painted band, so the tile
    // is the only thing that still carries it. A hidden band carries nothing to read.
    if (colorIdentityVisible) {
        const QStringList identity = ColorIdentityWidget::parseColorIdentity(colorIdentity);
        if (!identity.isEmpty()) {
            parts << tr("Color identity: %1").arg(identity.join(QStringLiteral(" ")));
        }
    }
    setAccessibleDescription(parts.join(QStringLiteral(", ")));
}

QColor DeckPreviewDeckBoxWidget::countColor() const
{
    const QColor roleColor = themeManager->appColor(AppColor::Icon);
    return roleColor.isValid() ? roleColor : palette().color(QPalette::PlaceholderText);
}

void DeckPreviewDeckBoxWidget::paintManaBand(QPainter &painter) const
{
    const QStringList symbols = manaSymbols();
    if (symbols.isEmpty()) {
        return;
    }

    const VisualDeckStorageSettings &settings = SettingsCache::instance().visualDeckStorage();
    const qreal unusedOpacity = settings.getVisualDeckStorageUnusedColorIdentitiesOpacity() / 100.0;
    const QStringList usedSymbols = ColorIdentityWidget::parseColorIdentity(colorIdentity);

    const int bandHeight = manaBandHeight();
    const int inset = std::max(1, qRound(bandHeight * MANA_ICON_INSET_RATIO));
    const int iconSide = std::max(1, bandHeight - 2 * inset);
    // The boundaries lean, so each colour is a parallelogram rather than a column. The
    // lean is a share of the band's thickness, which keeps the angle constant at every
    // card size instead of flattening as the band gets thinner.
    const qreal slant = std::max<qreal>(2.0, bandHeight * MANA_SEGMENT_SLANT_RATIO);
    // The lid starts half a pixel in (the body outline is stroked on the half-pixel), so
    // the band's own edges snap to the device grid to stay a crisp line.
    const int top = qRound(0.5 + nameRowHeight());
    const qreal bottom = top + bandHeight;
    const qreal dpr = devicePixelRatioF();

    painter.save();
    painter.setPen(Qt::NoPen);
    // The outer ends of the band are square against the box outline: the lean belongs to
    // the boundaries between colours, not to the band's ends.
    painter.setClipRect(rect());

    for (int i = 0; i < symbols.size(); ++i) {
        const QString &symbol = symbols.at(i);
        const qreal x0 = i * qreal(width()) / symbols.size();
        const qreal x1 = (i + 1) * qreal(width()) / symbols.size();
        // Interior boundaries grow a hair to the right so the antialiased edges of two
        // neighbours overlap instead of leaving a sliver of lid between them.
        const qreal grow = i + 1 == symbols.size() ? 0.0 : 1.0;
        // Only the boundaries between colours lean; the band's two ends stay square against
        // the box outline, or the first and last colour would taper away into the lid.
        const qreal leftTop = i == 0 ? 0.0 : x0 + slant;
        const qreal leftBottom = i == 0 ? 0.0 : x0 - slant;
        const qreal rightTop = i + 1 == symbols.size() ? width() : x1 + slant + grow;
        const qreal rightBottom = i + 1 == symbols.size() ? width() : x1 - slant + grow;

        QPolygonF segment;
        segment << QPointF(leftTop, top) << QPointF(rightTop, top) << QPointF(rightBottom, bottom)
                << QPointF(leftBottom, bottom);

        // An unused colour fades as a whole, background and rune together, which is what
        // the standalone colour identity row does.
        painter.setOpacity(usedSymbols.contains(symbol) ? 1.0 : unusedOpacity);
        painter.setBrush(GameSpecificColors::MTG::colorHelper(symbol).darker(MANA_BAND_DARKEN));
        painter.drawPolygon(segment);

        // Rasterize at the screen's pixel ratio: at a band's thickness a DPR-1 pixmap
        // would be upscaled by the backing store and the rune inside the disc would smear.
        QPixmap icon =
            ManaSymbolPixmapGenerator::generatePixmap(symbol, QSize(qRound(iconSide * dpr), qRound(iconSide * dpr)));
        if (!icon.isNull()) {
            icon.setDevicePixelRatio(dpr);
            painter.drawPixmap(QPointF((x0 + x1) / 2.0 - iconSide / 2.0, top + inset), icon);
        }
    }

    // The black segment is only a few steps above the lid, so without an edge it reads as
    // a gap with a skull floating in it. Hairlines in the page colour at the band's top and
    // bottom keep it part of the line at every theme.
    painter.setClipping(false);
    painter.setOpacity(1.0);
    const QColor seam = palette().color(QPalette::Window);
    painter.fillRect(QRectF(0, top, width(), 1), seam);
    painter.fillRect(QRectF(0, bottom - 1, width(), 1), seam);
    painter.restore();
}

void DeckPreviewDeckBoxWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF body = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = cornerRadius();

    QPainterPath bodyPath;
    bodyPath.addRoundedRect(body, radius, radius);

    QLinearGradient bodyGradient(body.topLeft(), body.bottomLeft());
    bodyGradient.setColorAt(0.0, palette().color(QPalette::Button));
    bodyGradient.setColorAt(1.0, palette().color(QPalette::Base));
    painter.fillPath(bodyPath, bodyGradient);
    painter.setPen(QPen(palette().color(QPalette::Shadow), 1));
    painter.drawPath(bodyPath);

    // The lid is a top band that follows the body's rounded corners, with a hairline
    // seam where a real lid would part from the box.
    const qreal lidBottom = body.top() + lidHeight();
    QPainterPath lidPath;
    lidPath.moveTo(body.left(), lidBottom);
    lidPath.lineTo(body.left(), body.top() + radius);
    lidPath.quadTo(body.topLeft(), QPointF(body.left() + radius, body.top()));
    lidPath.lineTo(body.right() - radius, body.top());
    lidPath.quadTo(body.topRight(), QPointF(body.right(), body.top() + radius));
    lidPath.lineTo(body.right(), lidBottom);
    lidPath.closeSubpath();

    QLinearGradient lidGradient(body.topLeft(), QPointF(body.left(), lidBottom));
    lidGradient.setColorAt(0.0, palette().color(QPalette::Midlight));
    lidGradient.setColorAt(1.0, palette().color(QPalette::Button));
    painter.fillPath(lidPath, lidGradient);

    // The seam is drawn in the page colour rather than Shadow: the lid and the body
    // meet at the same Button tone, so the only thing that can separate them is the
    // gap between the box and the view behind it. Shadow is darker than the page on
    // every dark theme, which is exactly when the lid needs the line most.
    painter.setPen(QPen(palette().color(QPalette::Window), 1));
    painter.drawLine(QPointF(body.left() + 1, lidBottom), QPointF(body.right() - 1, lidBottom));

    QColor lidHighlight = palette().color(QPalette::Light);
    lidHighlight.setAlpha(70);
    painter.setPen(QPen(lidHighlight, 1));
    painter.drawLine(QPointF(body.left() + radius, body.top() + 1.5), QPointF(body.right() - radius, body.top() + 1.5));

    // A groove just outside the artwork reads as the opening the card sits in; the
    // picture is a child widget and paints over its own rect, so the ring has to
    // stay outside it.
    const QRect art = artRect();
    if (!art.isEmpty()) {
        const qreal grooveRadius = std::max<qreal>(2.0, art.width() * 0.05);
        painter.setBrush(Qt::NoBrush);
        // Page colour again: on a dark theme a Shadow ring around the art is invisible
        // against the body, and the groove is the only thing that makes the art read as
        // inset rather than pasted on top.
        painter.setPen(QPen(palette().color(QPalette::Window), 1));
        painter.drawRoundedRect(QRectF(art).adjusted(-1, -1, 1, 1), grooveRadius, grooveRadius);
    }

    // The band belongs to the lid, so it goes down before the cues that have to stay
    // readable across it. The selection rim and the focus ring both run down the box's
    // sides, and a band painted last would cut both of them where it meets the edge.
    paintManaBand(painter);

    if (selected) {
        QColor glow = palette().color(QPalette::Highlight);
        glow.setAlpha(70);
        painter.setPen(QPen(glow, 5));
        painter.drawPath(bodyPath);
        painter.setPen(QPen(palette().color(QPalette::Highlight), 2));
        painter.drawPath(bodyPath);
    }

    if (hasFocus()) {
        // Which tile the keyboard is on is independent of which tiles are selected for
        // sharing, so focus gets its own dashed cue. It has to be inset: on the body
        // edge a 1 px stroke would hide completely under the 2 px selection rim.
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(palette().color(QPalette::Highlight), 1, Qt::DashLine));
        const QRectF focusRect = body.adjusted(3, 3, -3, -3);
        const qreal focusRadius = std::max<qreal>(1.0, radius - 3);
        painter.drawRoundedRect(focusRect, focusRadius, focusRadius);
    }

    const int padding = lidPadding();
    const QFont font = nameFont();
    const QFontMetrics metrics(font);

    int countWidth = 0;
    const QString countText = cardCount >= 0 ? QString::number(cardCount) : QString();
    if (!countText.isEmpty()) {
        countWidth = metrics.horizontalAdvance(countText) + padding;
    }

    // The name and the count are centred in the name row, not in the whole lid: the row
    // below them belongs to the mana strip.
    const QRectF lidRect(body.left() + padding, body.top(), body.width() - 2 * padding - countWidth, nameRowHeight());
    painter.setFont(font);
    painter.setPen(palette().color(QPalette::ButtonText));
    painter.drawText(lidRect, Qt::AlignVCenter | Qt::AlignLeft,
                     metrics.elidedText(deckName, Qt::ElideRight, qRound(lidRect.width())));

    if (!countText.isEmpty()) {
        QFont countFont = font;
        countFont.setBold(false);
        painter.setFont(countFont);
        painter.setPen(countColor());
        painter.drawText(QRectF(body.left(), body.top(), body.width() - padding, nameRowHeight()),
                         Qt::AlignVCenter | Qt::AlignRight, countText);
    }
}

void DeckPreviewDeckBoxWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
        event->accept();
        return;
    }

    if (event->button() == Qt::RightButton) {
        emit contextMenuRequested();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void DeckPreviewDeckBoxWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void DeckPreviewDeckBoxWidget::keyPressEvent(QKeyEvent *event)
{
    // Space mirrors a single click (toggling share selection) and Enter mirrors a
    // double click (opening the deck), so selecting and opening do not need a mouse.
    // The context menu stays mouse-only; see the open item in the plan.
    //
    // A held key must not fire repeatedly: Space would flip the selection at the OS
    // repeat rate and Enter would request the same deck over and over.
    if (event->isAutoRepeat()) {
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Space) {
        event->accept();
        emit clicked();
        return;
    }

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        event->accept();
        emit doubleClicked();
        return;
    }

    QWidget::keyPressEvent(event);
}

void DeckPreviewDeckBoxWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    // The focus cue is painted, so gaining or losing focus is a repaint.
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
        update();
    }
}
