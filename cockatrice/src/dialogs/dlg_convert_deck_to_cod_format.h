/**
 * @file dlg_convert_deck_to_cod_format.h
 * @ingroup LocalDeckStorageDialogs
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DIALOG_CONVERT_DECK_TO_COD_FORMAT_H
#define DIALOG_CONVERT_DECK_TO_COD_FORMAT_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

class DialogConvertDeckToCodFormat : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConvertDeckToCodFormat(QWidget *parent);
    void retranslateUi();

    bool dontAskAgain() const;

private:
    QVBoxLayout *layout;
    QLabel *label;
    QCheckBox *dontAskAgainCheckbox;
    QDialogButtonBox *buttonBox;

    Q_DISABLE_COPY(DialogConvertDeckToCodFormat)
};

#endif // DIALOG_CONVERT_DECK_TO_COD_FORMAT_H
