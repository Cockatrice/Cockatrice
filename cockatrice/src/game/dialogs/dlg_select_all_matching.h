#ifndef COCKATRICE_DLG_SELECT_ALL_MATCHING_H
#define COCKATRICE_DLG_SELECT_ALL_MATCHING_H

#include <QDialog>

class FilterString;
class QCheckBox;
class QDialogButtonBox;
class QSpinBox;
class QComboBox;
class QLabel;

struct SelectAllMatchingOptions
{
    QStringList exprs = {};
    bool clearSelection = true;
    bool toggleSelected = false;
};

class DlgSelectAllMatching : public QDialog
{
    Q_OBJECT

    QLabel *exprLabel;
    QComboBox *exprComboBox;
    QCheckBox *clearSelectionCheckBox;
    QCheckBox *toggleSelectedCheckBox;
    QDialogButtonBox *buttonBox;

    void validateAndAccept();
    bool validateMatchExists(const FilterString &filterString);

    [[nodiscard]] QStringList getExprs() const;

public:
    explicit DlgSelectAllMatching(QWidget *parent, const SelectAllMatchingOptions &options);

    [[nodiscard]] QString getExpr() const;
    [[nodiscard]] SelectAllMatchingOptions getOptions() const;
};

#endif // COCKATRICE_DLG_SELECT_ALL_MATCHING_H
