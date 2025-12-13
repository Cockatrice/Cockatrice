#ifndef COCKATRICE_TUTORIAL_BUBBLE_WIDGET_H
#define COCKATRICE_TUTORIAL_BUBBLE_WIDGET_H
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

class BubbleWidget : public QFrame
{
    Q_OBJECT
public:
    QLabel *textLabel;
    QLabel *counterLabel;
    QPushButton *closeButton;
    QPushButton *previousSequenceButton;
    QPushButton *previousStepButton;
    QPushButton *nextStepButton;
    QPushButton *nextSequenceButton;

    BubbleWidget(QWidget *parent);
    void setText(const QString &text);
};

#endif // COCKATRICE_TUTORIAL_BUBBLE_WIDGET_H
