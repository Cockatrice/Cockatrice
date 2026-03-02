/**
 * @file sequence_edit.h
 * @ingroup UI
 * @brief TODO: Document this.
 */

#ifndef SEQUENCEEDIT_H
#define SEQUENCEEDIT_H

#include <QEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class SequenceEdit : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceEdit(const QString &_shortcutName, QWidget *parent = nullptr);
    QString getSequence();
    void setShortcutName(const QString &_shortcutName);
    void refreshShortcut();
    void clear();
    void retranslateUi();

private slots:
    void removeLastShortcut();
    void restoreDefault();

protected:
    bool eventFilter(QObject *, QEvent *event) override;

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
    bool validateShortcut(const QKeySequence &sequence);
    void finishShortcut();
    void updateSettings();
};

#endif // SEQUENCEEDIT_H
