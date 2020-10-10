
#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>

class QKeyEvent;
class QWidget;
class QString;

// Should be used when the there is a risk of conflict between line editor
// shortcuts and other shortcuts
class LineEditUnfocusable : public QLineEdit
{
public:
    LineEditUnfocusable(QWidget *parent = nullptr);
    LineEditUnfocusable(const QString &contents, QWidget *parent = nullptr);

private:
    bool isUnfocusShortcut(QKeyEvent *key);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif
