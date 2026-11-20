#include "raise_on_hover.h"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QEvent>
#include <QPropertyAnimation>
#include <QWidget>

RaiseOnHover::RaiseOnHover(QWidget *target, int offsetPx, int durationMs, QObject *parent)
    : QObject(parent != nullptr ? parent : target), target(target), offsetPx(offsetPx)
{
    baseline = target->pos();

    animation = new QPropertyAnimation(target, "pos", this);
    animation->setDuration(durationMs);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setStartValue(baseline);
    animation->setEndValue(baseline - QPoint(0, offsetPx));

    target->installEventFilter(this);
}

bool RaiseOnHover::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != target) {
        return QObject::eventFilter(watched, event);
    }

    switch (event->type()) {
        case QEvent::Enter:
            raiseTarget();
            break;
        case QEvent::Leave:
            lowerTarget();
            break;
        case QEvent::Resize:
            // A new size is a new resting place, even mid-flight.
            baseline = target->pos();
            break;
        case QEvent::Move:
            if (animation->state() != QAbstractAnimation::Running) {
                baseline = target->pos();
            }
            break;
        default:
            break;
    }

    return QObject::eventFilter(watched, event);
}

void RaiseOnHover::raiseTarget()
{
    if (!enabled) {
        return;
    }

    if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    } else {
        baseline = target->pos();
        animation->setStartValue(baseline);
        animation->setEndValue(baseline - QPoint(0, offsetPx));
    }
    animation->setDirection(QAbstractAnimation::Forward);
    animation->start();
}

void RaiseOnHover::lowerTarget()
{
    if (!enabled) {
        return;
    }

    if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }
    animation->setDirection(QAbstractAnimation::Backward);
    animation->start();
}
