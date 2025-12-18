#ifndef COCKATRICE_TUTORIAL_CONTROLLER_H
#define COCKATRICE_TUTORIAL_CONTROLLER_H

#include "tutorial_overlay.h"

#include <QObject>
#include <QVector>
#include <functional>

struct TutorialStep
{
    QWidget *targetWidget;
    QString text;
    std::function<void()> onEnter = nullptr; // Optional function to run when this step starts
    std::function<void()> onExit = nullptr;  // Optional function to run when step ends
};

struct TutorialSequence
{
    QString name;
    QVector<TutorialStep> steps;

    void addStep(const TutorialStep &step)
    {
        steps.append(step);
    }
};

class TutorialController : public QObject
{
    Q_OBJECT

public slots:
    void start();
    void nextStep();
    void prevStep();
    void nextSequence();
    void prevSequence();
    void exitTutorial();

public:
    explicit TutorialController(QWidget *_tutorializedWidget);

    void addSequence(const TutorialSequence &step);

private:
    QWidget *tutorializedWidget;
    QVector<TutorialSequence> sequences;
    int currentSequence = -1;
    int currentStep = -1;

    TutorialOverlay *tutorialOverlay;

    void showStep();
};

#endif // COCKATRICE_TUTORIAL_CONTROLLER_H
