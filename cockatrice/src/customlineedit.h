
#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>

class QKeyEvent;
class QWidget;
class QString;

// Should be used when the there is a risk of conflict between line editor
// shortcuts and other shortcuts
class CustomLineEdit : public QLineEdit
{
public:
    CustomLineEdit(QWidget *parent = nullptr);
    CustomLineEdit(const QString &contents, QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif
