#include "tutorial_overlay.h"

#include "tutorial_bubble_widget.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>

TutorialOverlay::TutorialOverlay(QWidget *parent) : QWidget(parent)
{
    // Make it a child widget that covers the entire parent
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    if (parent) {
        parent->installEventFilter(this);
        // Initial sizing to cover parent
        setGeometry(parent->rect());
    }

    // ---- Control bar -------------------------------------------------
    controlBar = new QFrame(this);
    controlBar->setStyleSheet(
        "QFrame { background: rgba(30,30,30,200); border-radius: 6px; }"
        "QPushButton { padding: 6px 10px; border: 1px solid #aaa; border-radius: 4px; background:#f5f5f5; }"
        "QPushButton:hover { background:#eaeaea; }");

    QHBoxLayout *barLayout = new QHBoxLayout(controlBar);
    barLayout->setContentsMargins(8, 4, 8, 4);

    titleLabel = new QLabel("Tutorial", controlBar);
    titleLabel->setStyleSheet("color:white; font-weight:bold;");
    barLayout->addWidget(titleLabel);
    barLayout->addStretch();

    auto mkBtn = [&](const QString &t, const QString &tip) {
        QPushButton *b = new QPushButton(t, controlBar);
        b->setToolTip(tip);
        return b;
    };

    QPushButton *prevSeq = mkBtn("⏮", "Previous chapter");
    QPushButton *prev = mkBtn("◀", "Previous step");
    QPushButton *next = mkBtn("▶", "Next step");
    QPushButton *nextSeq = mkBtn("⏭", "Next chapter");
    QPushButton *close = mkBtn("✕", "Exit tutorial");

    barLayout->addWidget(prevSeq);
    barLayout->addWidget(prev);
    barLayout->addWidget(next);
    barLayout->addWidget(nextSeq);
    barLayout->addWidget(close);

    connect(prev, &QPushButton::clicked, this, &TutorialOverlay::prevStep);
    connect(next, &QPushButton::clicked, this, &TutorialOverlay::nextStep);
    connect(prevSeq, &QPushButton::clicked, this, &TutorialOverlay::prevSequence);
    connect(nextSeq, &QPushButton::clicked, this, &TutorialOverlay::nextSequence);
    connect(close, &QPushButton::clicked, this, &TutorialOverlay::skipTutorial);

    // ---- Bubble ------------------------------------------------------
    bubble = new BubbleWidget(this);
    bubble->hide();

    controlBar->hide();
}

void TutorialOverlay::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Ensure we cover the parent and are on top
    raise();
    parentResized();
}

void TutorialOverlay::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

void TutorialOverlay::setBlocking(bool block)
{
    blockInput = block;
    setAttribute(Qt::WA_TransparentForMouseEvents, !blockInput);
}

void TutorialOverlay::setTargetWidget(QWidget *w)
{
    if (targetWidget)
        targetWidget->removeEventFilter(this);

    targetWidget = w;

    if (targetWidget)
        targetWidget->installEventFilter(this);

    recomputeLayout();
}

void TutorialOverlay::setText(const QString &t)
{
    tutorialText = t;
    bubble->setText(t);
    bubble->adjustSize();
    recomputeLayout();
}

QRect TutorialOverlay::currentHoleRect() const
{
    if (!targetWidget || !targetWidget->isVisible())
        return QRect();

    // Map target widget's position to the overlay's coordinate system
    // Since overlay is a child of the main window, we map through the parent
    QPoint targetGlobal = targetWidget->mapToGlobal(QPoint(0, 0));
    QPoint targetInOverlay = mapFromGlobal(targetGlobal);

    return QRect(targetInOverlay, targetWidget->size()).adjusted(-6, -6, 6, 6);
}

void TutorialOverlay::resizeEvent(QResizeEvent *)
{
    recomputeLayout();
}

bool TutorialOverlay::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parentWidget() && (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        parentResized();
    }

    if (obj == targetWidget) {
        if (event->type() == QEvent::Show) {
            // Defer layout recalculation to give Qt time to finalize geometry
            QMetaObject::invokeMethod(this, [this]() { recomputeLayout(); }, Qt::QueuedConnection);
        } else if (event->type() == QEvent::Hide || event->type() == QEvent::Move || event->type() == QEvent::Resize) {
            recomputeLayout();
        }
    }

    return QWidget::eventFilter(obj, event);
}

void TutorialOverlay::parentResized()
{
    if (!parentWidget())
        return;

    // Simply match parent's geometry as a child widget
    setGeometry(parentWidget()->rect());
    recomputeLayout();
}

void TutorialOverlay::recomputeLayout()
{
    QRect hole = currentHoleRect();

    if (hole.isEmpty()) {
        if (bubble) {
            bubble->hide();
        }
        if (controlBar) {
            controlBar->hide();
        }
        hide();
        return;
    }

    show();
    raise();

    // ---- Bubble ----
    QSize bsize = bubble->sizeHint().expandedTo(QSize(160, 60));
    highlightBubbleRect = computeBubbleRect(hole, bsize);
    bubble->setGeometry(highlightBubbleRect);
    bubble->show();
    bubble->raise();

    // ---- Control bar ----
    controlBar->adjustSize();
    controlBar->show();

    const int margin = 8;
    QRect r = rect();

    QList<QPoint> positions = {{r.right() - controlBar->width() - margin, r.bottom() - controlBar->height() - margin},
                               {r.right() - controlBar->width() - margin, margin},
                               {margin, r.bottom() - controlBar->height() - margin},
                               {margin, margin}};

    for (const QPoint &pos : positions) {
        QRect proposed(pos, controlBar->size());
        if (!proposed.intersects(hole)) {
            controlBar->move(pos);
            break;
        }
    }

    controlBar->raise();
    update();
}

QRect TutorialOverlay::computeBubbleRect(const QRect &hole, const QSize &bubbleSize) const
{
    const int margin = 16;
    QRect r = rect();
    QRect bubble;

    if (hole.isEmpty()) {
        bubble = QRect(r.center() - QPoint(bubbleSize.width() / 2, bubbleSize.height() / 2), bubbleSize);
    } else {
        bubble = QRect(hole.right() + margin, hole.top(), bubbleSize.width(), bubbleSize.height());

        if (!r.contains(bubble))
            bubble.moveLeft(hole.left() - margin - bubbleSize.width());

        if (!r.contains(bubble)) {
            bubble.moveLeft(hole.center().x() - bubbleSize.width() / 2);
            bubble.moveTop(hole.top() - margin - bubbleSize.height());
        }

        if (!r.contains(bubble))
            bubble.moveTop(hole.bottom() + margin);
    }

    // Final clamp to overlay bounds - ensure min <= max for qBound
    int maxLeft = qMax(r.left(), r.right() - bubble.width());
    int maxTop = qMax(r.top(), r.bottom() - bubble.height());

    bubble.moveLeft(qBound(r.left(), bubble.left(), maxLeft));
    bubble.moveTop(qBound(r.top(), bubble.top(), maxTop));

    return bubble;
}

void TutorialOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRect hole = currentHoleRect();

    if (hole.isEmpty()) {
        // No hole, just fill everything
        p.fillRect(rect(), QColor(0, 0, 0, 160));
    } else {
        // Create a path for the entire overlay
        QPainterPath fullPath;
        fullPath.addRect(rect());

        // Subtract the hole from it
        QPainterPath holePath;
        holePath.addRoundedRect(hole, 8, 8);

        QPainterPath overlayPath = fullPath.subtracted(holePath);

        // Fill the overlay (everything except the hole)
        p.fillPath(overlayPath, QColor(0, 0, 0, 160));
    }
}