#ifndef SECUENCEEDIT_H
#define SECUENCEEDIT_H

#include <QWidget>
#include <QKeySequence>

class QLineEdit;
class QPushButton;
class QEvent;

class SequenceEdit : public QWidget
{
    Q_OBJECT
public:
    SequenceEdit(QString _shorcutName, QWidget *parent = 0);
    QString getSecuence();
    void refreshShortcut();
    void clear();
signals:

private slots:
    void removeLastShortcut();
    void restoreDefault();

protected:
    bool eventFilter(QObject *, QEvent *event);
private:
    QString shorcutName;
    QLineEdit *lineEdit;
    QPushButton *clearButton;
    QPushButton *defaultButton;
    int keys;
    int currentKey;
    int maxKeys;
    bool valid;

    void processKey(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    void finishShortcut();
    void updateSettings();
};

#endif // SECUENCEEDIT_H
