#include "labeled_input.h"

LabeledInput::LabeledInput(QWidget *parent, const QString &labelText) : QWidget(parent)
{
    label = new QLabel(labelText, this);
    layout = new QHBoxLayout(this);
    layout->addWidget(label);
}

QSpinBox *LabeledInput::addSpinBox(const int minValue, const int maxValue, const int defaultValue)
{
    auto *spinBox = new QSpinBox(this);
    spinBox->setRange(minValue, maxValue);
    spinBox->setValue(defaultValue);
    layout->addWidget(spinBox);
    connect(spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(spinBoxValueChanged(int)));
    return spinBox;
}

// Add a QComboBox (for arbitrary selections)
QComboBox *LabeledInput::addComboBox(const QStringList &items, const QString &defaultItem)
{
    auto *comboBox = new QComboBox(this);
    comboBox->addItems(items);
    if (!defaultItem.isEmpty()) {
        comboBox->setCurrentText(defaultItem);
    }
    layout->addWidget(comboBox);
    return comboBox;
}

// Add a QComboBox specifically for Qt Directions
QComboBox *LabeledInput::addDirectionComboBox()
{
    const QStringList directions = {"Qt::Horizontal", "Qt::Vertical"};
    const auto comboBox = addComboBox(directions, "Qt::Vertical");
    connect(comboBox, SIGNAL(currentTextChanged(QString)), this, SIGNAL(directionComboBoxChanged(QString)));
    return comboBox;
}