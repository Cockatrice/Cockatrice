#ifndef MANA_CURVE_WIDGET_H
#define MANA_CURVE_WIDGET_H
#include <QHBoxLayout>
#include <QWidget>
#include <utility>

class ManaCurveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManaCurveWidget(QWidget *parent = nullptr);
    void updateDisplay();

    void setCurve(std::map<int, int> new_curve)
    {
        this->curve = std::move(new_curve);
    }

private:
    std::map<int, int> curve;
    QHBoxLayout *layout;
};

#endif // MANA_CURVE_WIDGET_H
