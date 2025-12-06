#include "tutorial_controller.h"

TutorialController::TutorialController(QWidget *_tutorializedWidget)
    : QObject(_tutorializedWidget), tutorializedWidget(_tutorializedWidget)
{
    tutorialOverlay = new TutorialOverlay(nullptr); // nullptr parent because we overlay this on top of *everything*

    // Stay above everything
    tutorialOverlay->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    tutorialOverlay->setAttribute(Qt::WA_TranslucentBackground);

    // Hide until started
    tutorialOverlay->hide();

    connect(tutorialOverlay, &TutorialOverlay::nextStep, this, &TutorialController::progressTutorial);

    connect(tutorialOverlay, &TutorialOverlay::skipTutorial, this, [this]() { tutorialOverlay->hide(); });
}

void TutorialController::addStep(const TutorialStep &step)
{
    steps.append(step);
}

void TutorialController::start()
{
    if (steps.isEmpty()) {
        return;
    }

    // Align ourselves and show
    QPoint topLeft = tutorializedWidget->mapToGlobal(QPoint(0, 0));
    QSize windowSize = tutorializedWidget->size();

    tutorialOverlay->setGeometry(QRect(topLeft, windowSize));
    tutorialOverlay->show();
    tutorialOverlay->raise();

    // Start the tutorial
    currentStep = 0;
    showStep();
}

void TutorialController::progressTutorial()
{
    currentStep++;
    if (currentStep >= steps.size()) {
        tutorialOverlay->hide();
        return;
    }
    showStep();
}

void TutorialController::showStep()
{
    const auto &step = steps[currentStep];

    // Run any action associated with previous steps
    if (currentStep < 0) {
        const auto &previousStep = steps[currentStep - 1];

        if (previousStep.onExit) {
            previousStep.onExit();
        }
    }

    // Run any action associated with this step
    if (step.onEnter) {
        step.onEnter();
    }

    tutorialOverlay->setTargetWidget(step.targetWidget);
    tutorialOverlay->setText(step.text);
    tutorialOverlay->parentResized();
    tutorialOverlay->raise();
    tutorialOverlay->update();
}
