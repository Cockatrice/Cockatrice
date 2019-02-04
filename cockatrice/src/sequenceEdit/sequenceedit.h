#ifndef SEQUENCEEDIT_H
#define SEQUENCEEDIT_H

#include <QEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class SequenceEdit : public QWidget
{
    Q_OBJECT
public:
    SequenceEdit(const QString &_shortcutName, QWidget *parent = nullptr);
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
    int keys = 0;
    int currentKey = 0;
    bool valid = false;

    void processKey(QKeyEvent *e);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    void finishShortcut();
    void updateSettings();
};

#endif // SEQUENCEEDIT_H
