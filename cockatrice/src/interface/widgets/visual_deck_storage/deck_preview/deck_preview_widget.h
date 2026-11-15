/**
 * @file deck_preview_widget.h
 * @ingroup VisualDeckPreviewWidgets
 */

#ifndef DECK_PREVIEW_WIDGET_H
#define DECK_PREVIEW_WIDGET_H

#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <libcockatrice/utility/qt_utils.h>
#include <qtmetamacros.h>

class QLabel;
class QMenu;
class QMouseEvent;
class DeckPreviewCardPictureWidget;
class DeckPreviewDeckBoxWidget;
class DeckPreviewDeckTagsDisplayWidget;
class VisualDeckStorageModel;
class VisualDeckStorageWidget;
class QVBoxLayout;
struct LoadedDeck;

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
     * @brief The deck box; the parent widget wires its size to the card size setting.
     */
    DeckPreviewDeckBoxWidget *deckBox;

    /** @brief The path of the deck file backing this preview. */
    QString filePath;

    /**
     * @brief Forwards the VDS animation setting to the box's hover lift.
     *
     * Pushed down by the storage widget's folder tree instead of every preview
     * subscribing to the setting, so a thousand decks cost one connection.
     */
    void setSelectionAnimationEnabled(bool enabled);

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
    DeckPreviewDeckTagsDisplayWidget *deckTagsDisplayWidget;
    QLabel *bannerCardLabel;
    QComboBox *bannerCardComboBox;
    QList<QWidget *> fixedWidthChildren; ///< Children clamped to the tile width on resize.
    int lastKnownTileWidth = -1;         ///< The tile width last applied to the children.
    bool shareSelectable = false;
    bool shareSelected = false;

    void updateSelectionStyle();
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
                    if (auto *scroll = QtUtils::findParentOfType<QAbstractScrollArea>(combo)) {
                        QApplication::sendEvent(scroll->viewport(), event);
                        return true; // Mark event as handled
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
