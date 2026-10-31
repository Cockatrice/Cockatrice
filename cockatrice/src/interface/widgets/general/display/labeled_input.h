/**
 * @file labeled_input.h
 * @ingroup DeckEditorCardGroupWidgets
 */
//! \todo Document this file.

#ifndef LABELED_INPUT_H
#define LABELED_INPUT_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <qtmetamacros.h>

class QComboBox;
class QHBoxLayout;
class QLabel;
class QSpinBox;

class LabeledInput final : public QWidget
{
    Q_OBJECT

public:
    explicit LabeledInput(QWidget *parent, const QString &labelText);

    // Add a QSpinBox (for arbitrary numbers)
    QSpinBox *addSpinBox(int minValue, int maxValue, int defaultValue = 0);

    // Add a QComboBox (for arbitrary selections)
    QComboBox *addComboBox(const QStringList &items, const QString &defaultItem = QString());

    // Add a QComboBox specifically for Qt Directions
    QComboBox *addDirectionComboBox();

signals:
    void spinBoxValueChanged(int newValue); // Declare the valueChanged signal
    void comboBoxValueChanged(int newValue);
    void directionComboBoxChanged(QString newDirection);

private:
    QLabel *label;
    QHBoxLayout *layout;
};

#endif // LABELED_INPUT_H
