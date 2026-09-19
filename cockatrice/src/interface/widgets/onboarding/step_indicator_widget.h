#ifndef STEP_INDICATOR_WIDGET_H
#define STEP_INDICATOR_WIDGET_H

#include <QWidget>

/** @brief Row of dots showing progress through a fixed-length sequence of steps,
 *         in the style of a mobile/OS setup flow. Purely presentational. */
class StepIndicatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StepIndicatorWidget(QWidget *parent = nullptr);

    void setStepCount(int count);
    void setCurrentStep(int index);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int stepCount = 0;
    int currentStep = 0;

    static constexpr int kDotDiameter = 8;
    static constexpr int kActiveDotWidth = 22;
    static constexpr int kDotSpacing = 10;
    static constexpr int kVerticalMargin = 6;
};

#endif // STEP_INDICATOR_WIDGET_H
