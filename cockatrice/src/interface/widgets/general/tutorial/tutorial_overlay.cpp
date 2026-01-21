#include "tutorial_overlay.h"

#include "tutorial_bubble_widget.h"

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QTextEdit>
#include <QTimer>

TutorialOverlay::TutorialOverlay(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setAutoFillBackground(false);

    if (parent) {
        parent->installEventFilter(this);
        setGeometry(parent->rect());
        raise();
    }

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
    nextButton = mkBtn("▶", "Next step");
    nextSeqButton = mkBtn("⏭", "Next chapter");
    QPushButton *close = mkBtn("✕", "Exit tutorial");

    barLayout->addWidget(prevSeq);
    barLayout->addWidget(prev);
    barLayout->addWidget(nextButton);
    barLayout->addWidget(nextSeqButton);
    barLayout->addWidget(close);

    connect(prev, &QPushButton::clicked, this, &TutorialOverlay::prevStep);
    connect(nextButton, &QPushButton::clicked, this, &TutorialOverlay::nextStep);
    connect(prevSeq, &QPushButton::clicked, this, &TutorialOverlay::prevSequence);
    connect(nextSeqButton, &QPushButton::clicked, this, &TutorialOverlay::nextSequence);
    connect(close, &QPushButton::clicked, this, &TutorialOverlay::skipTutorial);

    bubble = new BubbleWidget(this);
    bubble->hide();

    controlBar->hide();
}

void TutorialOverlay::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (parentWidget()) {
        QWidget *parent = parentWidget();
        setGeometry(0, 0, parent->width(), parent->height());
    }

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

void TutorialOverlay::setInteractive(bool interactive, bool clickThrough)
{
    isInteractive = interactive;
    allowClickThrough = clickThrough;

    if (nextButton) {
        nextButton->setEnabled(!interactive);
        if (interactive) {
            nextButton->setToolTip("Complete the highlighted action to continue");
        } else {
            nextButton->setToolTip("Next step");
        }
    }

    if (nextSeqButton) {
        nextSeqButton->setEnabled(!interactive);
        if (interactive) {
            nextSeqButton->setToolTip("Complete the highlighted action to continue");
        } else {
            nextSeqButton->setToolTip("Next chapter");
        }
    }

    // Update mask when clickThrough changes
    updateMask();
}

void TutorialOverlay::setInteractionHint(const QString &hint)
{
    bubble->setInteractionHint(hint);
}

void TutorialOverlay::showValidationHint(const QString &hint)
{
    if (!hint.isEmpty()) {
        bubble->setValidationHint(hint);
    }
}

void TutorialOverlay::setProgress(int stepNum,
                                  int totalSteps,
                                  int overallStep,
                                  int overallTotal,
                                  const QString &sequenceTitle)
{
    bubble->setProgress(stepNum, totalSteps, overallStep, overallTotal);

    if (!sequenceTitle.isEmpty()) {
        titleLabel->setText(sequenceTitle);
    }
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

    QPoint targetGlobal = targetWidget->mapToGlobal(QPoint(0, 0));
    QPoint targetInOverlay = mapFromGlobal(targetGlobal);

    return QRect(targetInOverlay, targetWidget->size()).adjusted(-6, -6, 6, 6);
}

void TutorialOverlay::mousePressEvent(QMouseEvent *event)
{
    QRect hole = currentHoleRect();

    // Check if click is in the highlighted area
    if (hole.contains(event->pos())) {
        // For non-clickthrough steps, emit targetClicked for advancement
        if (!allowClickThrough && isInteractive && !qobject_cast<QLineEdit *>(targetWidget) &&
            !qobject_cast<QTextEdit *>(targetWidget) && !qobject_cast<QPlainTextEdit *>(targetWidget) &&
            !qobject_cast<QComboBox *>(targetWidget)) {
            QTimer::singleShot(100, this, [this]() { emit targetClicked(); });
        }
        // If allowClickThrough, the mask ensures events pass through
        return;
    }

    // Click outside highlighted area - block it
    event->accept();
}

void TutorialOverlay::updateMask()
{
    if (allowClickThrough) {
        QRect hole = currentHoleRect();
        if (!hole.isEmpty()) {
            // Create a mask that excludes the hole area
            QRegion fullRegion(rect());
            QRegion holeRegion(hole);
            QRegion maskRegion = fullRegion.subtracted(holeRegion);
            setMask(maskRegion);
        } else {
            clearMask();
        }
    } else {
        clearMask();
    }
}

bool TutorialOverlay::event(QEvent *event)
{
    // Update mask on any event that might change geometry
    if (event->type() == QEvent::Move || event->type() == QEvent::Resize) {
        updateMask();
    }
    return QWidget::event(event);
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
            QMetaObject::invokeMethod(
                this, [this]() { recomputeLayout(); }, Qt::QueuedConnection);
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

    setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
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

    QSize bsize = bubble->sizeHint().expandedTo(QSize(160, 60));
    highlightBubbleRect = computeBubbleRect(hole, bsize);
    bubble->setGeometry(highlightBubbleRect);
    bubble->show();
    bubble->raise();

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
    updateMask(); // Update mask after layout changes
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
        p.fillRect(rect(), QColor(0, 0, 0, 160));
    } else {
        QPainterPath fullPath;
        fullPath.addRect(rect());

        QPainterPath holePath;
        holePath.addRoundedRect(hole, 8, 8);

        QPainterPath overlayPath = fullPath.subtracted(holePath);
        p.fillPath(overlayPath, QColor(0, 0, 0, 160));

        if (isInteractive) {
            QPen pen(QColor(100, 200, 255, 180), 2);
            pen.setStyle(Qt::DashLine);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(hole, 8, 8);
        }
    }
}