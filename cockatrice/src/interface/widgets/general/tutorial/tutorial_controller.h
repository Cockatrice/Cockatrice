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
    std::function<void()> onEnter; // Optional function to run when this step starts
    std::function<void()> onExit;  // Optional function to run when step ends
};

class TutorialController : public QObject
{
    Q_OBJECT

public slots:
    void progressTutorial();

public:
    explicit TutorialController(QWidget *_tutorializedWidget);

    void addStep(const TutorialStep &step);
    void start();

private:
    QWidget *tutorializedWidget;
    QVector<TutorialStep> steps;
    int currentStep = -1;

    TutorialOverlay *tutorialOverlay;

    void showStep();
};

#endif // COCKATRICE_TUTORIAL_CONTROLLER_H
