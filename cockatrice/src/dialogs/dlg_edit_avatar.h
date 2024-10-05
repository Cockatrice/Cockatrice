#ifndef DLG_EDITAVATAR_H
#define DLG_EDITAVATAR_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;

class EditAvatar : public QDialog
{
    Q_OBJECT
public:
    EditAvatar(QWidget *parent = nullptr);
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
