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
    setAttribute(Qt::WA_StaticContents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
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
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }

    raise();
    parentResized();
}

void TutorialOverlay::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ParentChange) {
        // Remove filter from old parent (Qt has already changed parentWidget() by now,
        // so iterate sender list isn't easy — reinstall unconditionally on the new parent)
        QWidget *p = parentWidget();
        if (p) {
            p->installEventFilter(this); // safe to call multiple times
            setGeometry(p->rect());
            raise();
        }
    }
    QWidget::changeEvent(event);
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
        nextButton->setToolTip(interactive ? "Complete the highlighted action to continue" : "Next step");
    }

    if (nextSeqButton) {
        nextSeqButton->setEnabled(!interactive);
        nextSeqButton->setToolTip(interactive ? "Complete the highlighted action to continue" : "Next chapter");
    }

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

    if (targetWidget) {
        targetWidget->removeEventFilter(this);
    }

    targetWidget = w;

    if (targetWidget) {
        targetWidget->installEventFilter(this);
    }

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
    if (!targetWidget || !targetWidget->isVisible()) {
        return QRect();
    }

    QPoint p = targetWidget->mapToGlobal(QPoint(0, 0));
    QPoint local = mapFromGlobal(p);

    QRect r(local, targetWidget->size());
    r = r.adjusted(-6, -6, 6, 6);

    return r;
}

void TutorialOverlay::mousePressEvent(QMouseEvent *event)
{
    QRect hole = currentHoleRect();

    if (hole.contains(event->pos())) {

        if (!allowClickThrough && isInteractive && !qobject_cast<QLineEdit *>(targetWidget) &&
            !qobject_cast<QTextEdit *>(targetWidget) && !qobject_cast<QPlainTextEdit *>(targetWidget) &&
            !qobject_cast<QComboBox *>(targetWidget)) {
            QTimer::singleShot(100, this, [this]() { emit targetClicked(); });
        }
        return;
    }

    event->accept();
}

void TutorialOverlay::updateMask()
{
    clearMask();

    if (!isVisible() || !parentWidget()) {
        return;
    }
    if (!allowClickThrough) {
        return;
    }

    QRect hole = currentHoleRect();

    if (!hole.isEmpty()) {
        QRegion full(rect());
        QRegion cut(hole);
        setMask(full.subtracted(cut));
    }
}

bool TutorialOverlay::event(QEvent *event)
{
    return QWidget::event(event);
}

void TutorialOverlay::resizeEvent(QResizeEvent *)
{
    recomputeLayout();
}

bool TutorialOverlay::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parentWidget()) {
        switch (event->type()) {
            case QEvent::Resize:
            case QEvent::Move: // window dragged to another monitor / position
                parentResized();
                break;
            default:
                break;
        }
    }

    if (obj == targetWidget) {
        switch (event->type()) {
            case QEvent::Show:
                QMetaObject::invokeMethod(this, [this] { recomputeLayout(); }, Qt::QueuedConnection);
                break;
            case QEvent::Hide:
            case QEvent::Move:
            case QEvent::Resize:
                recomputeLayout();
                break;
            default:
                break;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void TutorialOverlay::parentResized()
{
    if (!parentWidget()) {
        return;
    }

    const QSize s = parentWidget()->size();

    setGeometry(0, 0, s.width(), s.height());

    clearMask();
    recomputeLayout();

    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_NoSystemBackground, true);

    update();
}

void TutorialOverlay::recomputeLayout()
{
    if (!parentWidget()) {
        return;
    }

    resize(parentWidget()->window()->geometry().size());

    bubble->adjustSize();

    QRect hole = currentHoleRect();

    if (hole.isEmpty()) {
        bubble->hide();
        controlBar->hide();

        hide();

        update();
        return;
    }

    show();
    raise();

    QSize bsize = bubble->sizeHint().expandedTo(QSize(160, 60));
    highlightBubbleRect = computeBubbleRect(hole, bsize);

    bubble->setGeometry(highlightBubbleRect);
    bubble->show();

    controlBar->adjustSize();
    controlBar->show();

    const int margin = 8;
    QRect r = rect();

    QList<QPoint> positions = {{r.right() - controlBar->width() - margin, r.bottom() - controlBar->height() - margin},
                               {r.right() - controlBar->width() - margin, margin},
                               {margin, r.bottom() - controlBar->height() - margin},
                               {margin, margin}};

    QRect placedRect;

    for (const QPoint &pos : positions) {
        QRect proposed(pos, controlBar->size());

        if (!proposed.intersects(hole) && !proposed.intersects(highlightBubbleRect)) {
            placedRect = proposed;
            break;
        }
    }

    if (!placedRect.isValid()) {
        placedRect = QRect(QPoint(margin, margin), controlBar->size());
    }

    controlBar->move(placedRect.topLeft());
    controlBar->show();

    updateMask();
    update();
}

QRect TutorialOverlay::computeBubbleRect(const QRect &hole, const QSize &bubbleSize) const
{
    QRect r = rect();

    QRect bubble;

    if (hole.isEmpty()) {
        bubble = QRect(r.center() - QPoint(bubbleSize.width() / 2, bubbleSize.height() / 2), bubbleSize);
    } else {
        const int margin = 16;

        bubble = QRect(hole.right() + margin, hole.top(), bubbleSize.width(), bubbleSize.height());

        if (!r.contains(bubble)) {
            bubble.moveLeft(hole.left() - margin - bubbleSize.width());
        }

        if (!r.contains(bubble)) {
            bubble.moveLeft(hole.center().x() - bubbleSize.width() / 2);
            bubble.moveTop(hole.top() - margin - bubbleSize.height());
        }

        if (!r.contains(bubble)) {
            bubble.moveTop(hole.bottom() + margin);
        }
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