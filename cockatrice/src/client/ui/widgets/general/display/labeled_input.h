//
// Created by ascor on 10/11/24.
//

#ifndef LABELED_INPUT_H
#define LABELED_INPUT_H

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QWidget>

class LabeledInput : public QWidget
{
    Q_OBJECT

public:
    LabeledInput(const QString &labelText, QWidget *parent = nullptr);

    // Add a QSpinBox (for arbitrary numbers)
    QSpinBox* addSpinBox(int minValue, int maxValue, int defaultValue = 0);

    // Add a QComboBox (for arbitrary selections)
    QComboBox* addComboBox(const QStringList &items, const QString &defaultItem = QString());

    // Add a QComboBox specifically for Qt Directions
    QComboBox* addDirectionComboBox();

    signals:
        void spinBoxValueChanged(int newValue);  // Declare the valueChanged signal
        void comboBoxValueChanged(int newValue);
        void directionComboBoxChanged(QString newDirection);
private:
    QLabel *label;
    QHBoxLayout *layout;
};


#endif //LABELED_INPUT_H
