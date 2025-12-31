#include "resizable_panel.h"

#include "libcockatrice/utility/qt_utils.h"

#include <QColor>
#include <QHBoxLayout>
#include <QPixmap>
#include <QtGlobal>

ResizablePanel::ResizablePanel(const QString &_typeId, AbstractAnalyticsPanelWidget *analyticsPanel, QWidget *parent)
    : QWidget(parent), panel(analyticsPanel), typeId(_typeId)
{
    setAcceptDrops(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Frame for selection highlight
    frame = new QFrame(this);
    frame->setFrameShape(QFrame::Box);
    frame->setLineWidth(2);
    frame->setStyleSheet("border: none;");

    auto *frameLayout = new QVBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);

    // Add the analytics panel
    frameLayout->addWidget(analyticsPanel);

    dropIndicator = new QFrame(frame);
    dropIndicator->setStyleSheet("background-color: #3daee9;");
    dropIndicator->setFixedHeight(3);
    dropIndicator->hide();  // hidden by default
    dropIndicator->raise(); // make sure it's above children

    selectionOverlay = new QFrame(frame);
    selectionOverlay->setStyleSheet("background-color: rgba(61,174,233,50);"); // semi-transparent blue
    selectionOverlay->hide();                                                  // hidden by default
    selectionOverlay->raise();                                                 // make sure it is above children
    selectionOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);

    // Bottom bar with drag button and resize handle
    auto *bottomBar = new QWidget(frame);
    auto *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    // Drag button on the left
    dragButton = new QPushButton("☰", bottomBar);
    dragButton->setFixedSize(40, 8);
    dragButton->setCursor(Qt::OpenHandCursor);
    dragButton->setStyleSheet("QPushButton { "
                              "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4a4a4a, stop:1 #3a3a3a); "
                              "border: none; color: #888; font-size: 10px; }"
                              "QPushButton:hover { background: #5a5a5a; }");
    bottomLayout->addWidget(dragButton);

    // Resize handle fills the rest
    resizeHandle = new QWidget(bottomBar);
    resizeHandle->setFixedHeight(8);
    resizeHandle->setCursor(Qt::SizeVerCursor);
    resizeHandle->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                                "stop:0 #3a3a3a, stop:1 #2a2a2a);");
    bottomLayout->addWidget(resizeHandle, 1);

    frameLayout->addWidget(bottomBar);

    mainLayout->addWidget(frame);

    // Set size policy
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Calculate initial height - use panel's size hint if available
    int panelHint = analyticsPanel->sizeHint().height();
    int panelMin = analyticsPanel->minimumSizeHint().height();

    // Start with the larger of panel's hint and panel's minimum hint
    currentHeight = qMax(panelHint + 8, panelMin + 8);
    updateSizeConstraints();

    // Install event filters
    dragButton->installEventFilter(this);
    resizeHandle->installEventFilter(this);

    // Timer for auto-scroll during drag
    autoScrollTimer = new QTimer(this);
    autoScrollTimer->setInterval(50);
    connect(autoScrollTimer, &QTimer::timeout, this, &ResizablePanel::performAutoScroll);
}

void ResizablePanel::setSelected(bool selected)
{
    if (selected) {
        selectionOverlay->setGeometry(0, 0, width(), height());
        selectionOverlay->show();
    } else {
        selectionOverlay->hide();
    }
}

void ResizablePanel::setHeightFromSaved(int h)
{
    if (h > 0) {
        currentHeight = qMax(h, getMinimumAllowedHeight());
        updateSizeConstraints();
    }
}

int ResizablePanel::getCurrentHeight() const
{
    return currentHeight;
}

QSize ResizablePanel::sizeHint() const
{
    return QSize(width(), currentHeight);
}

QSize ResizablePanel::minimumSizeHint() const
{
    return QSize(0, getMinimumAllowedHeight());
}

// =====================================================================================================================
//                                                      Event Handling
// =====================================================================================================================

bool ResizablePanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == dragButton) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                dragStartPos = mouseEvent->globalPosition().toPoint();
#else
                dragStartPos = mouseEvent->globalPos();
#endif
                isDraggingPanel = false;
                dragButton->setCursor(Qt::ClosedHandCursor);
            }
            return false;
        } else if (event->type() == QEvent::MouseMove) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QPoint currentPos = mouseEvent->globalPosition().toPoint();
#else
                QPoint currentPos = mouseEvent->globalPos();
#endif
                int distance = (currentPos - dragStartPos).manhattanLength();
                if (distance >= 5 && !isDraggingPanel) {
                    isDraggingPanel = true;
                    startDrag();
                    return true;
                }
            }
            return false;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            dragButton->setCursor(Qt::OpenHandCursor);
            isDraggingPanel = false;
            return false;
        }
    }

    if (obj == resizeHandle) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            resizeStartY = mouseEvent->globalPosition().y();
#else
            resizeStartY = mouseEvent->globalPos().y();
#endif
            isResizing = true;
            resizeStartHeight = currentHeight;
            resizeHandle->grabMouse();
            return true;
        } else if (event->type() == QEvent::MouseMove && isResizing) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            int deltaY = mouseEvent->globalPosition().y() - resizeStartY;
#else
            int deltaY = mouseEvent->globalPos().y() - resizeStartY;
#endif
            int newHeight = resizeStartHeight + deltaY;

            int minAllowed = getMinimumAllowedHeight();
            newHeight = qMax(newHeight, minAllowed);

            currentHeight = newHeight;
            updateSizeConstraints();

            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            isResizing = false;
            resizeHandle->releaseMouse();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void ResizablePanel::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-resizablepanel")) {
        event->acceptProposedAction();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        showDropIndicator(event->position().y());
#else
        showDropIndicator(event->pos().y());
#endif
    }
}

void ResizablePanel::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-resizablepanel")) {
        event->acceptProposedAction();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        showDropIndicator(event->position().y());
        lastDragPos = mapToGlobal(event->position().toPoint());
#else
        showDropIndicator(event->pos().y());
        lastDragPos = mapToGlobal(event->pos());
#endif

        if (!autoScrollTimer->isActive()) {
            autoScrollTimer->start();
        }
    }
}

void ResizablePanel::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    hideDropIndicator();
    autoScrollTimer->stop();
}

void ResizablePanel::dropEvent(QDropEvent *event)
{
    hideDropIndicator();
    autoScrollTimer->stop();

    if (event->mimeData()->hasFormat("application/x-resizablepanel")) {
        QByteArray data = event->mimeData()->data("application/x-resizablepanel");
        quintptr ptr = *reinterpret_cast<const quintptr *>(data.constData());
        ResizablePanel *draggedPanel = reinterpret_cast<ResizablePanel *>(ptr);

        if (draggedPanel && draggedPanel != this) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            bool insertBefore = (event->position().y() < height() / 2);
#else
            bool insertBefore = (event->pos().y() < height() / 2);
#endif
            emit dropRequested(draggedPanel, this, insertBefore);
            event->acceptProposedAction();
        }
    }
}

void ResizablePanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (selectionOverlay->isVisible()) {
        selectionOverlay->setGeometry(0, 0, width(), height());
    }

    if (dropIndicator->isVisible()) {
        dropIndicator->setGeometry(0, dropIndicator->y(), width(), dropIndicator->height());
    }
}

// =====================================================================================================================
//                                                      Private Helpers
// =====================================================================================================================

int ResizablePanel::getMinimumAllowedHeight() const
{
    QSize panelMin = panel->minimumSizeHint();
    int panelMinHeight = (panelMin.isValid() && panelMin.height() > 0) ? panelMin.height() : 100;
    return panelMinHeight + 8;
}

void ResizablePanel::updateSizeConstraints()
{
    setMinimumHeight(currentHeight);
    setMaximumHeight(currentHeight);
    updateGeometry();
}

void ResizablePanel::startDrag()
{
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    quintptr ptr = reinterpret_cast<quintptr>(this);
    QByteArray data(reinterpret_cast<const char *>(&ptr), sizeof(ptr));
    mimeData->setData("application/x-resizablepanel", data);

    drag->setMimeData(mimeData);

    QPixmap pixmap(width(), 40);
    pixmap.fill(QColor(58, 58, 58, 200));
    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(width() / 2, 20));

    emit dragStarted(this);

    autoScrollTimer->start();

    Qt::DropAction result = drag->exec(Qt::MoveAction);
    Q_UNUSED(result);

    autoScrollTimer->stop();
    dragButton->setCursor(Qt::OpenHandCursor);
    isDraggingPanel = false;
}

void ResizablePanel::performAutoScroll()
{
    QScrollArea *scrollArea = QtUtils::findParentOfType<QScrollArea>(this);

    if (!scrollArea) {
        return;
    }

    QScrollBar *scrollBar = scrollArea->verticalScrollBar();
    if (!scrollBar) {
        return;
    }

    QRect scrollRect = scrollArea->viewport()->rect();
    QPoint scrollTopLeft = scrollArea->viewport()->mapToGlobal(scrollRect.topLeft());
    QRect globalScrollRect(scrollTopLeft, scrollRect.size());

    const int scrollMargin = 50;
    int scrollSpeed = 0;

    if (lastDragPos.y() < globalScrollRect.top() + scrollMargin) {
        scrollSpeed = -15;
    } else if (lastDragPos.y() > globalScrollRect.bottom() - scrollMargin) {
        scrollSpeed = 15;
    }

    if (scrollSpeed != 0) {
        int newValue = scrollBar->value() + scrollSpeed;
        newValue = qBound(scrollBar->minimum(), newValue, scrollBar->maximum());
        scrollBar->setValue(newValue);
    }
}

void ResizablePanel::showDropIndicator(double y)
{
    bool before = (y < height() / 2);
    dropIndicator->setGeometry(0, before ? 0 : height() - 3, width(), 3);
    dropIndicator->show();
}

void ResizablePanel::hideDropIndicator()
{
    dropIndicator->hide();
}
