#ifndef DLG_CONNECT_H
#define DLG_CONNECT_H

#include <QDialog>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;
class QRadioButton;

class DeleteHighlightedItemWhenShiftDelPressedEventFilter : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};


class DlgConnect : public QDialog {
    Q_OBJECT
public:
    DlgConnect(QWidget *parent = 0);
    QString getHost() const;
    int getPort() const { return portEdit->text().toInt(); }
    QString getPlayerName() const { return playernameEdit->text(); }
    QString getPassword() const { return passwordEdit->text(); }
private slots:
    void actOk();
    void actCancel();
    void passwordSaved(int state);
    void previousHostSelected(bool state);
    void newHostSelected(bool state);
private:
    QLabel *hostLabel, *portLabel, *playernameLabel, *passwordLabel;
    QLineEdit *hostEdit, *portEdit, *playernameEdit, *passwordEdit;
    QCheckBox *savePasswordCheckBox, *autoConnectCheckBox;
    QComboBox *previousHosts;
    QRadioButton *newHostButton, *previousHostButton;
};

#endif
