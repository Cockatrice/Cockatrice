#include "token_preview_widget.h"

#include "../../interface/theme_manager.h"
#include "../board/abstract_card_item.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QPainter>
#include <QPointF>
#include <QResizeEvent>
#include <QString>
#include <algorithm>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/utility/card_ref.h>

namespace
{
// P/T pill, matching the #7700 hover panel. Its sizes are given for a 270 px picture
// (CardHoverWidget::CARD_IMAGE_WIDTH) and scale with the preview.
constexpr qreal PILL_REFERENCE_WIDTH = 270.0;
constexpr qreal PILL_MIN_WIDTH = 48.0;
constexpr qreal PILL_MIN_HEIGHT = 22.0;
constexpr qreal PILL_TEXT_PADDING = 4.0;
constexpr qreal PILL_CORNER_RADIUS = 4.0;
constexpr qreal PILL_FONT_PIXELS = 12.0;
constexpr qreal PILL_ANCHOR_X = 0.92;
constexpr qreal PILL_ANCHOR_Y = 0.94;
constexpr int PILL_SCRIM_ALPHA = 180;
} // namespace

class TokenPreviewCardItem : public AbstractCardItem
{
public:
    TokenPreviewCardItem()
    {
        setFlag(ItemIsSelectable, false);
        setAcceptedMouseButtons(Qt::NoButton);
        setAcceptHoverEvents(false);
        setCursor(Qt::ArrowCursor);
        connect(state, &CardState::stateChanged, this, [this] { update(); });
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
    {
        // Show what the user is creating even when it will enter face down.
        // Reuse the board's artwork, custom-token background and name rendering.
        AbstractCardItem::paint(painter, option, widget);

        const QSizeF translatedSize = getTranslatedSize(painter);
        if (translatedSize.isEmpty()) {
            return;
        }
        const qreal scale = translatedSize.width() / boundingRect().width();

        painter->save();
        transformPainter(painter, translatedSize, 0);

        if (!state->getAnnotation().isEmpty()) {
            painter->setPen(Qt::white);
            painter->setBackground(Qt::black);
            painter->setBackgroundMode(Qt::OpaqueMode);
            const QRectF textRect(4 * scale, 4 * scale, translatedSize.width() - 8 * scale,
                                  translatedSize.height() - 8 * scale);
            painter->drawText(textRect, Qt::AlignCenter | Qt::TextWrapAnywhere, state->getAnnotation());
        }

        // The card picture already shows the printed P/T, so only a different value
        // needs the pill. A custom token with no picture also has no printed value.
        const QString pt = state->getPT();
        if (!pt.isEmpty() && pt != getCardInfo().getPowTough()) {
            paintPtPill(painter, translatedSize, pt);
        }

        painter->restore();
    }

protected:
    // The artwork carries the name; the dialog's own fields already show it.
    [[nodiscard]] bool shouldPaintNameOverPicture() const override
    {
        return false;
    }

private:
    void paintPtPill(QPainter *painter, const QSizeF &translatedSize, const QString &pt) const
    {
        const qreal ratio = translatedSize.width() / PILL_REFERENCE_WIDTH;

        QFont font = themeManager->appFont(AppFont::CardText);
        font.setBold(true);
        font.setPixelSize(std::max(1, qRound(PILL_FONT_PIXELS * ratio)));

        const QFontMetricsF metrics(font);
        const QSizeF pillSize(
            qMax(PILL_MIN_WIDTH * ratio, metrics.horizontalAdvance(pt) + 2 * PILL_TEXT_PADDING * ratio),
            qMax(PILL_MIN_HEIGHT * ratio, metrics.height()));
        const QPointF anchor(translatedSize.width() * PILL_ANCHOR_X, translatedSize.height() * PILL_ANCHOR_Y);
        // Centred on the card's printed P/T box, then kept inside the card.
        const QRectF pill(qBound(0.0, anchor.x() - pillSize.width() / 2, translatedSize.width() - pillSize.width()),
                          qBound(0.0, anchor.y() - pillSize.height() / 2, translatedSize.height() - pillSize.height()),
                          pillSize.width(), pillSize.height());

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setBackgroundMode(Qt::TransparentMode);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 0, PILL_SCRIM_ALPHA));
        const qreal radius = PILL_CORNER_RADIUS * ratio;
        painter->drawRoundedRect(pill, radius, radius);
        painter->setFont(font);
        painter->setPen(Qt::white);
        painter->drawText(pill, Qt::AlignCenter, pt);
        painter->restore();
    }
};

TokenPreviewWidget::TokenPreviewWidget(QWidget *parent) : QGraphicsView(parent), card(new TokenPreviewCardItem)
{
    auto *previewScene = new QGraphicsScene(this);
    previewScene->addItem(card);
    previewScene->setSceneRect(card->boundingRect());
    setScene(previewScene);
    setInteractive(false);
    setFocusPolicy(Qt::NoFocus);
    setFrameShape(QFrame::NoFrame);
    viewport()->setBackgroundRole(QPalette::Window);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(150, 200);
    setAccessibleName(tr("Token preview"));
}

void TokenPreviewWidget::setToken(const TokenInfo &token)
{
    card->setCardRef(CardRef{.name = token.name, .providerId = token.providerId});
    card->setColor(token.color);
    card->setFaceDown(false);
    card->getState()->setPT(token.pt);
    card->getState()->setAnnotation(token.annotation);
    setToolTip(token.annotation);
    QString description = token.name + " " + token.pt + " " + token.annotation;
    if (token.faceDown) {
        description += " " + tr("Will be created face down.");
    }
    setAccessibleDescription(description);
}

QSize TokenPreviewWidget::sizeHint() const
{
    return {200, 280};
}

void TokenPreviewWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}
