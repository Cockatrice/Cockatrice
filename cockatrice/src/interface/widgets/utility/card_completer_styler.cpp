#include "card_completer_styler.h"

#include "../cards/card_info_picture_enlarged_widget.h"
#include "card_completer_delegate.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "reversed_completer_model.h"

#include <QAbstractAnimation>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QByteArray>
#include <QCompleter>
#include <QEvent>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QList>
#include <QMouseEvent>
#include <QPoint>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QScrollBar>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <QtGlobal>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card/card_search_model.h>
#include <libcockatrice/utility/card_ref.h>
#include <qnamespace.h>

template <class T> class QSharedPointer;

namespace
{
const QSize PreviewSize(300, 419);
const int PreviewMargin = 16;
const int FadeDuration = 120;
} // namespace

// ---------------------------------------------------------------------------

void CardCompleterStyler::apply(QCompleter *completer)
{
    if (!completer) {
        return;
    }

    // The styler lives as long as the completer
    new CardCompleterStyler(completer, completer);
}

// ---------------------------------------------------------------------------

CardCompleterStyler::CardCompleterStyler(QCompleter *completer, QObject *parent)
    : QObject(parent), completer(completer), reversedModel(nullptr), preview(nullptr), above(false)
{
    auto *popup = completer->popup();

    // Wrap the completer's model so its row order can be reversed when the
    // popup is shown above the text edit
    QAbstractItemModel *sourceModel = completer->model();
    reversedModel = new ReversedCompleterModel(completer);
    reversedModel->setSourceModel(sourceModel);
    completer->setModel(reversedModel);

    popup->setItemDelegate(new CardCompleterDelegate(popup));

    popup->viewport()->setMouseTracking(true);

    popup->installEventFilter(this);
    popup->viewport()->installEventFilter(this);

    connect(popup->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &CardCompleterStyler::updatePreview);
    connect(completer, qOverload<const QString &>(&QCompleter::activated), this, &CardCompleterStyler::hidePreview);
    connect(completer->completionModel(), &QAbstractItemModel::modelReset, this,
            &CardCompleterStyler::onCompletionReset);
}

CardCompleterStyler::~CardCompleterStyler()
{
    if (preview) {
        preview->deleteLater();
    }
}

// ---------------------------------------------------------------------------

bool CardCompleterStyler::eventFilter(QObject *obj, QEvent *ev)
{
    auto *popup = completer->popup();

    if (obj == popup->viewport()) {
        if (ev->type() == QEvent::MouseMove) {
            updatePreviewFromHover(static_cast<QMouseEvent *>(ev)->pos());
        }
    } else if (obj == popup) {
        switch (ev->type()) {
            case QEvent::Show:
            case QEvent::Move:
            case QEvent::Resize:
                applyPopupWidth();
                updateOrientation();
                reposition();
                break;
            case QEvent::Hide:
                hidePreview();
                break;
            case QEvent::KeyPress:
                if (handlePopupKeyPress(static_cast<QKeyEvent *>(ev))) {
                    return true;
                }
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(obj, ev);
}

// ---------------------------------------------------------------------------

bool CardCompleterStyler::handlePopupKeyPress(QKeyEvent *event)
{
    const int key = event->key();
    if (key != Qt::Key_Up && key != Qt::Key_Down) {
        return false;
    }

    auto *popup = completer->popup();
    const int rowCount = completer->completionModel()->rowCount();
    if (rowCount == 0) {
        return true;
    }

    // Handle navigation ourselves so arrow keys move the highlight without
    // rewriting the line edit. QCompleter's default handling selects with a
    // selection change that QLineEdit turns into setText(), which fights the
    // user while typing (e.g. typing "A" becomes "Axe" and cannot be edited).
    // With the popup above the text edit the rows are shown in reverse order and
    // the closest match is in the bottom-most row. The up arrow then advances
    // through the list (towards its end at the top of the popup).
    const QModelIndex current = popup->currentIndex();
    int newRow;
    if (!current.isValid()) {
        newRow = (key == Qt::Key_Down) ? 0 : rowCount - 1;
    } else {
        const int step = (key == Qt::Key_Up) ? -1 : 1;
        newRow = qBound(0, current.row() + step, rowCount - 1);
        if (newRow == current.row()) {
            return true;
        }
    }

    selectWithoutRewrite(completer->completionModel()->index(newRow, completer->completionColumn()));

    return true;
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::applyPopupWidth()
{
    auto *popup = completer->popup();

    // QCompleter sizes its popup to the line edit, so a narrow search field
    // would crush the rows. Ask for the width the rows are designed for (plus
    // the frame and scrollbar) and let QWidget's geometry clamping keep the
    // popup that wide, clamped to the screen it is shown on.
    int contentWidth = CardCompleterDelegate::PopupRowWidth + popup->frameWidth() * 2;
    if (auto *vbar = popup->verticalScrollBar(); vbar != nullptr) {
        contentWidth += vbar->sizeHint().width();
    }

    const QScreen *screen = popup->screen();
    const QRect available = screen ? screen->availableGeometry() : QRect();

    const int minWidth = available.isEmpty() ? contentWidth : qMin(contentWidth, available.width());
    popup->setMinimumWidth(minWidth);

    // setMinimumWidth() already widened the popup if it was narrower, but
    // QCompleter positioned it for the line-edit width, so the widened popup
    // can now extend past the right screen edge. The popup is a top-level
    // Qt::Popup window, so geometry()/move() are already in screen
    // coordinates: clamp the x from geometry() and move directly.
    if (!available.isEmpty()) {
        const QRect geom = popup->geometry();
        const int clampedX =
            qBound(available.left(), geom.x(), qMax(available.left(), available.right() - geom.width() + 1));
        if (clampedX != geom.x()) {
            popup->move(QPoint(clampedX, popup->y()));
        }
    }
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::updateOrientation()
{
    above = isPopupAboveWidget();
    reversedModel->setEnabled(above);
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::ensureClosestSelected()
{
    auto *popup = completer->popup();
    auto *completionModel = completer->completionModel();
    const int rowCount = completionModel->rowCount();
    if (rowCount == 0) {
        return;
    }

    const int currentRow = popup->currentIndex().row();
    const int closestRow = rowCount - 1;

    if (currentRow != closestRow) {
        selectWithoutRewrite(completionModel->index(closestRow, completer->completionColumn()));
    }
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::selectWithoutRewrite(const QModelIndex &index)
{
    auto *popup = completer->popup();
    auto *selectionModel = popup->selectionModel();

    // Changing the popup selection emits selectionChanged, which QCompleter
    // turns into highlighted() and QLineEdit turns into setText(). That
    // rewrites what the user typed (e.g. "A" becomes "Axe") and fights further
    // typing. Block the signals so the highlight and preview move without
    // touching the line edit; Enter/click still activates via activated().
    const bool wasBlocked = selectionModel->blockSignals(true);
    if (!index.isValid()) {
        selectionModel->clear();
    } else {
        selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    selectionModel->blockSignals(wasBlocked);
    popup->viewport()->update();

    // currentRowChanged was suppressed above, so sync the preview manually.
    updatePreview(index);
    if (index.isValid()) {
        popup->scrollTo(index, QAbstractItemView::EnsureVisible);
    } else {
        popup->scrollToTop();
    }
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::onCompletionReset()
{
    if (above) {
        ensureClosestSelected();
    }
}

// ---------------------------------------------------------------------------

bool CardCompleterStyler::isPopupAboveWidget() const
{
    auto *popup = completer->popup();
    QWidget *widget = completer->widget();

    if (!widget || !popup->isVisible()) {
        return false;
    }

    const QPoint popupBottom = popup->mapToGlobal(QPoint(0, popup->height()));
    const QPoint fieldTop = widget->mapToGlobal(QPoint(0, 0));

    return popupBottom.y() <= fieldTop.y();
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::updatePreviewFromHover(const QPoint &pos)
{
    const QModelIndex index = completer->popup()->indexAt(pos);

    // Hovering updates the preview but must not change the current completion
    if (index.isValid() && index != previewedIndex) {
        updatePreview(index);
    }
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::updatePreview(const QModelIndex &index)
{
    previewedIndex = index;

    if (!index.isValid()) {
        hidePreview();
        return;
    }

    auto card = index.data(CardSearchModel::CardInfoRole).value<QSharedPointer<CardInfo>>();

    if (!card) {
        hidePreview();
        return;
    }

    if (!preview) {
        QWidget *contextWindow = completer->widget() ? completer->widget()->window() : nullptr;

        preview = new CardInfoPictureEnlargedWidget(contextWindow);
        preview->hide();
        preview->setWindowOpacity(0.0);
    }

    const ExactCard exact = CardDatabaseManager::query()->getCard({card->getName()});

    if (!exact) {
        hidePreview();
        return;
    }

    preview->setCardPixmap(exact, PreviewSize);

    reposition();
    showPreview();
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::showPreview()
{
    if (!preview) {
        return;
    }

    stopPreviewFade();

    preview->show();
    preview->raise();

    auto *fade = new QPropertyAnimation(preview, "windowOpacity", preview);

    fade->setDuration(FadeDuration);
    fade->setStartValue(preview->windowOpacity());
    fade->setEndValue(1.0);
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::hidePreview()
{
    if (!preview) {
        return;
    }

    // Stop any in-flight fade first so a stale one cannot keep the preview visible
    stopPreviewFade();
    preview->hide();
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::stopPreviewFade()
{
    if (!preview) {
        return;
    }

    const auto animations = preview->findChildren<QPropertyAnimation *>();
    for (auto *animation : animations) {
        animation->stop();
    }
}

// ---------------------------------------------------------------------------

void CardCompleterStyler::reposition()
{
    if (!preview) {
        return;
    }

    auto *popup = completer->popup();

    const QRect popupGlobalRect(popup->mapToGlobal(popup->rect().topLeft()), popup->rect().size());

    int anchorY;

    if (previewedIndex.isValid()) {
        const QRect itemRect = popup->visualRect(previewedIndex);
        const QPoint itemCenter = popup->viewport()->mapToGlobal(itemRect.center());
        anchorY = itemCenter.y();
    } else {
        anchorY = popupGlobalRect.center().y();
    }

    const QScreen *screen = popup->screen();
    const QRect screenGeom = screen ? screen->availableGeometry() : QRect();

    const int rightX = popupGlobalRect.right() + PreviewMargin;
    const int leftX = popupGlobalRect.left() - PreviewMargin - preview->width();

    int x;
    int y = anchorY - preview->height() / 2;

    if (screenGeom.isEmpty()) {
        x = rightX;
    } else if (rightX + preview->width() <= screenGeom.right()) {
        x = rightX;
    } else if (leftX >= screenGeom.left()) {
        x = leftX;
    } else {
        x = rightX;
    }

    if (!screenGeom.isEmpty()) {
        x = qBound(screenGeom.left(), x, qMax(screenGeom.left(), screenGeom.right() - preview->width()));
        y = qBound(screenGeom.top(), y, qMax(screenGeom.top(), screenGeom.bottom() - preview->height()));
    }

    preview->move(x, y);
}
