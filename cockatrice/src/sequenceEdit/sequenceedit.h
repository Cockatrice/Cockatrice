#ifndef SECUENCEEDIT_H
#define SECUENCEEDIT_H

#include <QKeySequence>
#include <QWidget>

class QLineEdit;
class QPushButton;
class QEvent;

class SequenceEdit : public QWidget
{
    Q_OBJECT
public:
    SequenceEdit(QString _shortcutName, QWidget *parent = nullptr);
    QString getSequence();
    void refreshShortcut();
    void clear();

private slots:
    void removeLastShortcut();
    void restoreDefault();

protected:
    bool eventFilter(QObject *, QEvent *event);

private:
    QString shortcutName;
    QLineEdit *lineEdit;
    QPushButton *clearButton;
    QPushButton *defaultButton;
    int keys;
    int currentKey;
    bool valid;

    void processKey(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    void finishShortcut();
    void updateSettings();
};

#endif // SECUENCEEDIT_H
