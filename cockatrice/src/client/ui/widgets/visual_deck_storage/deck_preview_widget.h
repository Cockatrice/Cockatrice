#ifndef DECK_PREVIEW_WIDGET_H
#define DECK_PREVIEW_WIDGET_H

#include "../../../../deck/deck_loader.h"
#include "../cards/deck_preview_card_picture_widget.h"
#include "deck_preview_color_identity_widget.h"

#include <QVBoxLayout>
#include <QWidget>

class DeckPreviewWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit DeckPreviewWidget(QWidget *parent, const QString &_filePath);
    QString getColorIdentity();

    QVBoxLayout *layout;
    QString filePath;
    DeckLoader *deckLoader;
    DeckPreviewCardPictureWidget *bannerCardDisplayWidget;
    DeckPreviewColorIdentityWidget *colorIdentityWidget;

signals:
    void deckPreviewClicked(QMouseEvent *event, DeckPreviewWidget *instance);
    void deckPreviewDoubleClicked(QMouseEvent *event, DeckPreviewWidget *instance);

public slots:
    void setFilePath(const QString &filePath);
    void imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
};

#endif // DECK_PREVIEW_WIDGET_H
