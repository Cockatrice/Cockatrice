/**
 * @file filter_builder.h
 * @ingroup CardDatabaseModelFilters
 * @brief TODO: Document this.
 */

#ifndef FILTERBUILDER_H
#define FILTERBUILDER_H

#include <QWidget>

class QCheckBox;
class QComboBox;
class LineEditUnfocusable;
class CardFilter;

class FilterBuilder : public QWidget
{
    Q_OBJECT

private:
    QComboBox *typeCombo;
    QComboBox *filterCombo;
    LineEditUnfocusable *edit;
    CardFilter *fltr;

    void destroyFilter();

public:
    explicit FilterBuilder(QWidget *parent = nullptr);
    ~FilterBuilder() override;

signals:
    void add(const CardFilter *f);

public slots:
private slots:
    void emit_add();

protected:
};

#endif
