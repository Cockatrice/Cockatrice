#ifndef LINEEDITCOMPLETER_H
#define LINEEDITCOMPLETER_H

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QStringList>

class LineEditCompleter : public QLineEdit
{
    Q_OBJECT
private:
    QString cursorWord(const QString &line) const;
    void processMention() const;
    void processCard() const;
    QCompleter *mentionCompleter;
    QCompleter *cardCompleter;
private slots:
    void insertCompletion(QString);

protected:
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *e);

public:
    explicit LineEditCompleter(QWidget *parent = 0);
    void setMentionCompleter(QCompleter *mentionCompleter);
    void setCardCompleter(QCompleter *cardCompleter);
    void setMentionCompletionList(QStringList completionList);
};

#endif