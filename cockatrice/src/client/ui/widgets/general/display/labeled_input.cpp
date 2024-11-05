#include "labeled_input.h"

LabeledInput::LabeledInput(const QString &labelText, QWidget *parent)
        : QWidget(parent)
{
    label = new QLabel(labelText, this);
    layout = new QHBoxLayout(this);
    layout->addWidget(label);

}

QSpinBox* LabeledInput::addSpinBox(int minValue, int maxValue, int defaultValue)
{
    QSpinBox *spinBox = new QSpinBox(this);
    spinBox->setRange(minValue, maxValue);
    spinBox->setValue(defaultValue);
    layout->addWidget(spinBox);
    connect(spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(spinBoxValueChanged(int)));
    return spinBox;
}

// Add a QComboBox (for arbitrary selections)
QComboBox* LabeledInput::addComboBox(const QStringList &items, const QString &defaultItem)
{
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(items);
    if (!defaultItem.isEmpty()) {
        comboBox->setCurrentText(defaultItem);
    }
    layout->addWidget(comboBox);
    return comboBox;
}

// Add a QComboBox specifically for Qt Directions
QComboBox* LabeledInput::addDirectionComboBox()
{
    QStringList directions = {"Qt::Horizontal", "Qt::Vertical"};
    auto comboBox = addComboBox(directions, "Qt::Vertical");
    connect(comboBox, SIGNAL(currentTextChanged(QString)), this, SIGNAL(directionComboBoxChanged(QString)));
    return comboBox;
}