/**
 * @file deck_preview_widget.h
 * @ingroup VisualDeckPreviewWidgets
 */

#ifndef DECK_PREVIEW_WIDGET_H
#define DECK_PREVIEW_WIDGET_H

#include "../../cards/deck_preview_card_picture_widget.h"
#include "../visual_deck_storage_model.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QVBoxLayout>
#include <QWidget>

class QEnterEvent;
class QFrame;
class QLabel;
class QMenu;
class QMouseEvent;
class ColorIdentityWidget;
class DeckPreviewCardPictureWidget;
class DeckPreviewDeckTagsDisplayWidget;
class VisualDeckStorageModel;
class VisualDeckStorageWidget;

class DeckPreviewWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit DeckPreviewWidget(QWidget *parent,
                               VisualDeckStorageWidget *_visualDeckStorageWidget,
                               VisualDeckStorageModel *_model,
                               const QString &_filePath);
    void retranslateUi();

    /**
     * @brief The banner card picture; the parent widget wires its size to the card size setting.
     */
    DeckPreviewCardPictureWidget *bannerCardDisplayWidget;

    /** @brief The path of the deck file backing this preview. */
    QString filePath;

    void setShareSelectable(bool selectable);
    void setShareSelected(bool selected);
    [[nodiscard]] bool isShareSelected() const;
    [[nodiscard]] bool isShareSelectable() const;

signals:
    void deckLoadRequested(const QString &filePath);
    void openDeckEditor(const LoadedDeck &deck);
    void shareDeckRequested(const QString &filePath);
    void shareSelectionToggled(bool selected);

public slots:
    /**
     * @brief Re-reads the row's data from the model and syncs every child widget.
     * Connected to the model's dataChanged signal.
     */
    void syncFromModel();

    /**
     * @brief Reloads the deck file if its modification time is newer than the stored one.
     */
    void reloadIfModified();
    void refreshBannerCardToolTip();
    void updateColorIdentityVisibility(bool visible);
    void updateBannerCardComboBoxVisibility(bool visible);
    void updateTagsVisibility(bool visible);
    void setBannerCard(int);
    void setTags(const QStringList &tags);

protected:
    void enterEvent(QEnterEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    [[nodiscard]] int row() const;
    [[nodiscard]] QString getDisplayName() const;
    void refreshBannerCardText();
    void updateBannerCardComboBox(const QString &currentText);
    bool promptFileConversionIfRequired();
    QMenu *createRightClickMenu();
    void addSetBannerCardMenu(QMenu *menu);
    void imageClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);
    void imageSingleClicked();
    void imageDoubleClickedEvent(QMouseEvent *event, DeckPreviewCardPictureWidget *instance);

    void actRenameDeck();
    void actRenameFile();
    void actDeleteFile();

    VisualDeckStorageWidget *visualDeckStorageWidget;
    VisualDeckStorageModel *model;
    QVBoxLayout *layout;
    ColorIdentityWidget *colorIdentityWidget;
    DeckPreviewDeckTagsDisplayWidget *deckTagsDisplayWidget;
    QLabel *bannerCardLabel;
    QComboBox *bannerCardComboBox;
    QList<QWidget *> fixedWidthChildren; ///< Children clamped to the picture width on resize.
    int lastKnownBannerWidth = -1;       ///< The picture width last applied to the children.
    QFrame *selectionFrame = nullptr;
    bool shareSelectable = false;
    bool shareSelected = false;

    void updateSelectionStyle();
    void updateSelectionFrameGeometry();
};

class NoScrollFilter : public QObject
{
    Q_OBJECT
public:
    explicit NoScrollFilter(QObject *parent = nullptr) : QObject(parent)
    {
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() == QEvent::Wheel) {
            if (auto *combo = qobject_cast<QComboBox *>(obj)) {
                // If popup is not open, forward event to parent scroll area
                if (!combo->view()->isVisible()) {
                    // Try to find a scrollable parent and manually send the event
                    QWidget *parent = combo->parentWidget();
                    while (parent) {
                        if (auto *scroll = qobject_cast<QAbstractScrollArea *>(parent)) {
                            QApplication::sendEvent(scroll->viewport(), event);
                            return true; // Mark event as handled
                        }
                        parent = parent->parentWidget();
                    }
                    // If no scrollable parent found, just block
                    return true;
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

#endif // DECK_PREVIEW_WIDGET_H
