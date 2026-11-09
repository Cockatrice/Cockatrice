#ifndef COCKATRICE_TOKEN_PREVIEW_WIDGET_H
#define COCKATRICE_TOKEN_PREVIEW_WIDGET_H

#include <QGraphicsView>
#include <libcockatrice/game/player/token_info.h>

class TokenPreviewCardItem;

/** A noninteractive preview of the token that the dialog will create. */
class TokenPreviewWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit TokenPreviewWidget(QWidget *parent = nullptr);
    void setToken(const TokenInfo &token);
    QSize sizeHint() const override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    TokenPreviewCardItem *card;
};

#endif
