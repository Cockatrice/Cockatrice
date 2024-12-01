#ifndef DLG_EDITAVATAR_H
#define DLG_EDITAVATAR_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class DlgEditAvatar : public QDialog
{
    Q_OBJECT
public:
    DlgEditAvatar(QWidget *parent = nullptr);
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
