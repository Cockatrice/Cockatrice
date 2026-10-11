#pragma once

#include "draw_probability_config.h"

#include <QDialog>
#include <qtmetamacros.h>

class QComboBox;
class QSpinBox;
class QLabel;
class QFormLayout;
class QWidget;

class DrawProbabilityConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DrawProbabilityConfigDialog(QWidget *parent = nullptr);

    void retranslateUi();

    void setFromConfig(const DrawProbabilityConfig &_config);
    DrawProbabilityConfig result() const
    {
        return cfg;
    }

protected:
    void accept() override;

private:
    DrawProbabilityConfig cfg;

    QFormLayout *form;

    // Widgets
    QComboBox *criteria;
    QComboBox *exactness;
    QSpinBox *quantity;
    QSpinBox *drawn;

    QLabel *labelCriteria;
    QLabel *labelExactness;
    QLabel *labelQuantity;
    QLabel *labelDrawn;
};
