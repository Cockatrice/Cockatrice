/**
 * @file dlg_login_prompt.h
 * @ingroup ConnectionDialogs
 */
//! \todo Document this file.

#ifndef DLG_LOGIN_PROMPT_H
#define DLG_LOGIN_PROMPT_H

#include <QDialog>

class QCheckBox;
class QLineEdit;

/**
 * @brief Small sign-in dialog used when a cockatrice:// link needs credentials
 * that are not saved for the target server.
 *
 * The entered name and password are handed to the intent chain; when the user
 * opts to save them, they are stored in the server settings so that later links
 * to the same server connect seamlessly.
 */
class DlgLoginPrompt : public QDialog
{
    Q_OBJECT

public:
    explicit DlgLoginPrompt(const QString &serverText, QWidget *parent = nullptr);

    [[nodiscard]] QString username() const;
    [[nodiscard]] QString password() const;
    [[nodiscard]] bool savePassword() const;

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QCheckBox *savePasswordCheckBox;
};

#endif // DLG_LOGIN_PROMPT_H