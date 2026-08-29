/**
 * @file dlg_convert_deck_to_cod_format.h
 * @ingroup LocalDeckStorageDialogs
 * @ingroup Lobby
 */
//! \todo Document this file.

#ifndef DIALOG_CONVERT_DECK_TO_COD_FORMAT_H
#define DIALOG_CONVERT_DECK_TO_COD_FORMAT_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <functional>

class QWidget;

class DialogConvertDeckToCodFormat : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConvertDeckToCodFormat(QWidget *parent);
    void retranslateUi();

    [[nodiscard]] bool dontAskAgain() const;

    /**
     * @brief Checks whether the deck file at \a filePath can store tags.
     *
     * If the file is not a .cod deck, prompts the user for conversion to the
     * Cockatrice format, honoring the saved "always convert / don't ask again"
     * preference. On acceptance \a convert is called to perform the conversion.
     *
     * @param parent The widget to parent the prompt to.
     * @param filePath The path of the deck file to check.
     * @param convert Called to convert the deck once the user agrees.
     * @return true if tags can be stored (no conversion needed, or the conversion
     *         was performed), false if the user declined to convert.
     */
    static bool promptIfRequired(QWidget *parent, const QString &filePath, const std::function<bool()> &convert);

private:
    QVBoxLayout *layout;
    QLabel *label;
    QCheckBox *dontAskAgainCheckbox;
    QDialogButtonBox *buttonBox;

    Q_DISABLE_COPY(DialogConvertDeckToCodFormat)
};

#endif // DIALOG_CONVERT_DECK_TO_COD_FORMAT_H
