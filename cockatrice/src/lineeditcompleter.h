#ifndef LINEEDITCOMPLETER_H
#define LINEEDITCOMPLETER_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QStringList>

class LineEditCompleter : public QLineEdit
{
    Q_OBJECT
private:
    QString cursorWord(const QString& line) const;
    QCompleter* c;
private slots:
    void insertCompletion(QString);
protected:
    void keyPressEvent(QKeyEvent * event);
    void focusOutEvent(QFocusEvent * e);
public:
    explicit LineEditCompleter(QWidget *parent = 0);
    void setCompleter(QCompleter*);
    void setCompletionList(QStringList);
};

#endif