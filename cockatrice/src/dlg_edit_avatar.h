#ifndef DLG_EDITAVATAR_H
#define DLG_EDITAVATAR_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgEditAvatar : public QDialog {
    Q_OBJECT
public:
    DlgEditAvatar(QWidget *parent = 0);
    QByteArray getImage();
private slots:
    void actOk();
    void actCancel();
    void actBrowse();
private:
    QLabel *textLabel, *imageLabel;
    QPushButton *browseButton;
};

#endif
