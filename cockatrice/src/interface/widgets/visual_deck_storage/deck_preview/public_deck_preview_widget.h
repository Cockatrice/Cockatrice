/**
 * @file public_deck_preview_widget.h
 * @ingroup VisualDeckPreviewWidgets
 */

#ifndef PUBLIC_DECK_PREVIEW_WIDGET_H
#define PUBLIC_DECK_PREVIEW_WIDGET_H

#include "../remote_public_decks_model.h"

#include <QList>
#include <QString>
#include <QWidget>

class ColorIdentityWidget;
class DeckPreviewCardPictureWidget;
class FlowWidget;
class QKeyEvent;
class QLabel;
class QMouseEvent;
class QResizeEvent;

/**
 * @brief A preview tile for a public deck published by another user.
 *
 * Renders the banner card picture (looked up by name/provider in the card
 * database) with the deck name overlaid, the color identity, the deck's tags
 * (read-only) and its upload time, all from the metadata the server stores for
 * the deck, so no deck list is downloaded until the user actually opens the
 * deck. Double-clicking the banner requests opening it.
 */
class PublicDeckPreviewWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit PublicDeckPreviewWidget(QWidget *parent, const RemotePublicDecksModel::DeckEntry &entry);

    void setEntry(const RemotePublicDecksModel::DeckEntry &entry);

    /** @brief Sets the accessible name announced to assistive technologies. */
    void setBaseAccessibleName(const QString &name);

    /** @brief Scales the banner card picture, mirroring the Visual Deck Storage. */
    void setScaleFactor(int scale);

signals:
    void openDeckRequested(int deckId);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void updateColorIdentityVisibility();
    void updateTagsVisibility();
    void updateUploadTimeVisibility();

private:
    int deckId = 0;
    QString baseAccessibleName;
    bool hasColorIdentity = false;
    bool hasTags = false;
    bool hasUploadTime = false;
    int lastKnownBannerWidth = 0;
    QList<QWidget *> fixedWidthChildren;
    DeckPreviewCardPictureWidget *bannerCardDisplayWidget;
    ColorIdentityWidget *colorIdentityWidget;
    FlowWidget *tagsFlowWidget;
    QLabel *uploadTimeLabel;
};

#endif // PUBLIC_DECK_PREVIEW_WIDGET_H
