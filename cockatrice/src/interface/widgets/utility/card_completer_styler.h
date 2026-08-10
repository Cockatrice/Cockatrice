/**
 * @file card_completer_styler.h
 * @ingroup UtilityWidgets
 */
//! \todo Document this file.

#ifndef CARD_COMPLETER_STYLER_H
#define CARD_COMPLETER_STYLER_H

#include <QModelIndex>
#include <QObject>

class CardInfoPictureEnlargedWidget;
class QCompleter;
class QKeyEvent;
class QPoint;
class ReversedCompleterModel;

/**
 * @brief Applies styled row painting and a card-image preview to a card completer.
 *
 * The completer popup rows are painted by CardCompleterDelegate and the image of
 * the currently selected (or hovered) row is shown beside the popup.
 */
class CardCompleterStyler : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Styles @p completer and follows its current selection.
     *
     * The styler is parented to the completer so it lives exactly as long as the
     * completer itself.
     */
    static void apply(QCompleter *completer);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
    void updatePreview(const QModelIndex &index);
    void updatePreviewFromHover(const QPoint &pos);
    void onCompletionReset();

private:
    explicit CardCompleterStyler(QCompleter *completer, QObject *parent = nullptr);
    ~CardCompleterStyler() override;

    void showPreview();
    void hidePreview();
    void stopPreviewFade();
    void reposition();

    void updateOrientation();
    void ensureClosestSelected();

    bool handlePopupKeyPress(QKeyEvent *event);
    bool isPopupAboveWidget() const;

    QCompleter *completer;
    ReversedCompleterModel *reversedModel;
    CardInfoPictureEnlargedWidget *preview;
    QModelIndex previewedIndex;
    bool above;
};

#endif // CARD_COMPLETER_STYLER_H
