#include "card_completer_styler.h"

#include "../cards/card_info_picture_enlarged_widget.h"
#include "card_completer_delegate.h"
#include "reversed_completer_model.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QCompleter>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QScrollBar>
#include <QSize>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/models/database/card/card_search_model.h>

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

    if (!above) {
        return false;
    }

    auto *popup = completer->popup();
    const int rowCount = completer->completionModel()->rowCount();
    if (rowCount == 0) {
        return true;
    }

    // With the popup above the text edit the rows are shown in reverse order and
    // the closest match is in the bottom-most row. The up arrow then advances
    // through the list (towards its end at the top of the popup).
    const int currentRow = popup->currentIndex().row();
    const int step = (key == Qt::Key_Up) ? -1 : 1;
    const int newRow = qBound(0, currentRow + step, rowCount - 1);

    if (newRow != currentRow) {
        popup->setCurrentIndex(completer->completionModel()->index(newRow, completer->completionColumn()));
    }

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

    // Widen the popup right away on the first show; QCompleter has already
    // positioned it for the narrow size, so keep it on screen afterwards.
    if (popup->width() < minWidth) {
        popup->resize(minWidth, popup->height());

        if (!available.isEmpty()) {
            QPoint pos = popup->pos();
            pos.setX(qBound(available.left(), pos.x(), qMax(available.left(), available.right() - popup->width() + 1)));
            popup->move(pos);
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
        popup->setCurrentIndex(completionModel->index(closestRow, completer->completionColumn()));
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
