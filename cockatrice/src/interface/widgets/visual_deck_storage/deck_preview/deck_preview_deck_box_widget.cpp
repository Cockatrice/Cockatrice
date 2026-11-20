#include "deck_preview_deck_box_widget.h"

#include "../../../theme_manager.h"
#include "../../cards/card_info_picture_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
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

int DeckPreviewDeckBoxWidget::lidHeight() const
{
    return std::clamp(qRound(width() * LID_HEIGHT_RATIO), 14, 34);
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
    setAccessibleDescription(parts.join(QStringLiteral(", ")));
}

QColor DeckPreviewDeckBoxWidget::countColor() const
{
    const QColor roleColor = themeManager->appColor(AppColor::Icon);
    return roleColor.isValid() ? roleColor : palette().color(QPalette::PlaceholderText);
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

    const int padding = std::max(4, qRound(width() * SIDE_INSET_RATIO));
    const QFont font = nameFont();
    const QFontMetrics metrics(font);

    int countWidth = 0;
    const QString countText = cardCount >= 0 ? QString::number(cardCount) : QString();
    if (!countText.isEmpty()) {
        countWidth = metrics.horizontalAdvance(countText) + padding;
    }

    const QRectF lidRect(body.left() + padding, body.top(), body.width() - 2 * padding - countWidth, lidHeight());
    painter.setFont(font);
    painter.setPen(palette().color(QPalette::ButtonText));
    painter.drawText(lidRect, Qt::AlignVCenter | Qt::AlignLeft,
                     metrics.elidedText(deckName, Qt::ElideRight, qRound(lidRect.width())));

    if (!countText.isEmpty()) {
        QFont countFont = font;
        countFont.setBold(false);
        painter.setFont(countFont);
        painter.setPen(countColor());
        painter.drawText(QRectF(body.left(), body.top(), body.width() - padding, lidHeight()),
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
