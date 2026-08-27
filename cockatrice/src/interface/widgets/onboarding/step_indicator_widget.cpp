#include "step_indicator_widget.h"

#include <QPainter>
#include <QPainterPath>

StepIndicatorWidget::StepIndicatorWidget(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(kDotDiameter + 2 * kVerticalMargin);
}

void StepIndicatorWidget::setStepCount(int count)
{
    stepCount = qMax(0, count);
    currentStep = qBound(0, currentStep, qMax(0, stepCount - 1));
    updateGeometry();
    update();
}

void StepIndicatorWidget::setCurrentStep(int index)
{
    if (stepCount == 0) {
        return;
    }
    currentStep = qBound(0, index, stepCount - 1);
    update();
}

QSize StepIndicatorWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize StepIndicatorWidget::minimumSizeHint() const
{
    if (stepCount == 0) {
        return QSize(0, height());
    }
    int width = kActiveDotWidth + (stepCount - 1) * kDotDiameter + (stepCount - 1) * kDotSpacing;
    return QSize(width, height());
}

void StepIndicatorWidget::paintEvent(QPaintEvent * /*event*/)
{
    if (stepCount == 0) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor activeColor = palette().color(QPalette::Highlight);

    // QPalette::Mid alpha-blended against a dark Window background reads as
    // near-invisible (Mid is itself a dark grey in dark palettes -- see
    // PaletteGenerator's satShadeLo/Dark roles). WindowText is guaranteed to
    // contrast against Window in any theme by definition, so alpha-blending
    // *that* instead keeps the dots visibly dim-but-present in both light and
    // dark schemes. Same trick PaletteGenerator uses for placeholder text.
    QColor inactiveColor = palette().color(QPalette::WindowText);
    inactiveColor.setAlpha(100);

    int totalWidth = 0;
    for (int i = 0; i < stepCount; ++i) {
        totalWidth += (i == currentStep) ? kActiveDotWidth : kDotDiameter;
        if (i > 0) {
            totalWidth += kDotSpacing;
        }
    }

    int x = (width() - totalWidth) / 2;
    const int y = height() / 2;

    for (int i = 0; i < stepCount; ++i) {
        const bool active = (i == currentStep);
        const int dotWidth = active ? kActiveDotWidth : kDotDiameter;

        QPainterPath path;
        QRectF rect(x, y - kDotDiameter / 2.0, dotWidth, kDotDiameter);
        path.addRoundedRect(rect, kDotDiameter / 2.0, kDotDiameter / 2.0);
        painter.fillPath(path, active ? activeColor : inactiveColor);

        x += dotWidth + kDotSpacing;
    }
}
