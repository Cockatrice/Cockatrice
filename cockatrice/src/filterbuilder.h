#ifndef FILTERBUILDER_H
#define FILTERBUILDER_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLineEdit;
class CardFilter;

class FilterBuilder : public QWidget {
    Q_OBJECT

private:
    QComboBox *typeCombo;
    QComboBox *filterCombo;
    QLineEdit *edit;
    CardFilter *fltr;

    void destroyFilter();

public:
    FilterBuilder(QWidget *parent = 0);
    ~FilterBuilder();

signals:
    void add(const CardFilter *f);

public slots:
private slots:
    void emit_add();
protected:
};

#endif
