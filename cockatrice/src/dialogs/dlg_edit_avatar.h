/**
 * @file dlg_edit_avatar.h
 * @ingroup AccountDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_EDITAVATAR_H
#define DLG_EDITAVATAR_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(DlgEditAvatarLog, "dlg_edit_avatar");

class QLabel;
class QPushButton;
class QCheckBox;

class DlgEditAvatar : public QDialog
{
    Q_OBJECT
public:
    explicit DlgEditAvatar(QWidget *parent = nullptr);
    QByteArray getImage();
private slots:
    void actOk();
    void actBrowse();

private:
    QImage image;
    QLabel *textLabel, *imageLabel;
    QPushButton *browseButton;
};

#endif
