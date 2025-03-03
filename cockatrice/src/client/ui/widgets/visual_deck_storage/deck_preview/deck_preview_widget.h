#ifndef DECK_PREVIEW_WIDGET_H
#define DECK_PREVIEW_WIDGET_H

#include "../../../../../deck/deck_loader.h"
#include "../../cards/additional_info/color_identity_widget.h"
#include "../../cards/deck_preview_card_picture_widget.h"
#include "../visual_deck_storage_widget.h"
#include "deck_preview_deck_tags_display_widget.h"

#include <QComboBox>
#include <QEvent>
#include <QVBoxLayout>
#include <QWidget>

class QMenu;
class VisualDeckStorageWidget;
class DeckPreviewDeckTagsDisplayWidget;

class DeckPreviewWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit DeckPreviewWidget(QWidget *_parent,
                               VisualDeckStorageWidget *_visualDeckStorageWidget,
                               const QString &_filePath);
    void retranslateUi();
    QString getColorIdentity();

    VisualDeckStorageWidget *visualDeckStorageWidget;
    QVBoxLayout *layout;
    QString filePath;
    DeckLoader *deckLoader;
    DeckPreviewCardPictureWidget *bannerCardDisplayWidget = nullptr;
    ColorIdentityWidget *colorIdentityWidget = nullptr;
    DeckPreviewDeckTagsDisplayWidget *deckTagsDisplayWidget = nullptr;
    QLabel *bannerCardLabel = nullptr;
    QComboBox *bannerCardComboBox = nullptr;
    bool filteredBySearch = false;
    bool filteredByColor = false;
    bool filteredByTags = false;
    bool checkVisibility() const;

signals:
    void deckLoadRequested(const QString &filePath);
    void openDeckEditor(const DeckLoader *deck);
    void visibilityUpdated();

public slots:
    void setFilePath(const QString &filePath);
    void refreshBannerCardText();
    void updateBannerCardComboBox();
    void setBannerCard(int);
    void imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void initializeUi(bool deckLoadSuccess);
    void updateVisibility();
    void updateBannerCardComboBoxVisibility(bool visible);
    void updateTagsVisibility(bool visible);
    void resizeEvent(QResizeEvent *event) override;

private:
    QMenu *createRightClickMenu();

private slots:
    void actRenameDeck();
    void actRenameFile();
    void actDeleteFile();
};

class NoScrollFilter : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Wheel) {
            return true; // Blocks the event
        }
        return QObject::eventFilter(obj, event);
    }
};

#endif // DECK_PREVIEW_WIDGET_H
