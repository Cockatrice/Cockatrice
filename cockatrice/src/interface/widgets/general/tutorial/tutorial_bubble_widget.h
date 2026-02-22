#ifndef COCKATRICE_TUTORIAL_BUBBLE_WIDGET_H
#define COCKATRICE_TUTORIAL_BUBBLE_WIDGET_H
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

class BubbleWidget : public QFrame
{
    Q_OBJECT

public:
    explicit BubbleWidget(QWidget *parent = nullptr);

    void setText(const QString &text);
    void setProgress(int stepNum, int totalSteps, int overallStep, int overallTotal);
    void setInteractionHint(const QString &hint);
    void setValidationHint(const QString &hint);

private:
    void clearValidationHint();

    QLabel *counterLabel;
    QLabel *textLabel;
    QLabel *interactionLabel; // Shows "Click to continue"
    QLabel *validationLabel;  // Shows validation errors
    QLabel *progressLabel;    // Shows overall progress
    QGridLayout *layout;
    QTimer *validationTimer; // Auto-hide validation hint
};

#endif // COCKATRICE_TUTORIAL_BUBBLE_WIDGET_H
