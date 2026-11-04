#include "token_preview_widget.h"

#include "../board/abstract_card_item.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QResizeEvent>
#include <libcockatrice/card/card_info.h>

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
        const qreal scale = translatedSize.width() / boundingRect().width();
        painter->save();
        transformPainter(painter, translatedSize, 0);
        painter->setPen(Qt::white);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        const QRectF textRect(4 * scale, 4 * scale, translatedSize.width() - 8 * scale,
                              translatedSize.height() - 8 * scale);
        painter->drawText(textRect, Qt::AlignCenter | Qt::TextWrapAnywhere, state->getAnnotation());
        if (state->getPT() != getCardInfo().getPowTough()) {
            painter->setPen(QColor(255, 150, 0));
        }
        painter->drawText(textRect, Qt::AlignRight | Qt::AlignBottom, state->getPT());
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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(150, 200);
    setAccessibleName(tr("Token preview"));
}

void TokenPreviewWidget::setToken(const TokenInfo &token)
{
    card->setCardRef({token.name, token.providerId});
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
