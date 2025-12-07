#include "tutorial_controller.h"

#include <QTimer>

TutorialController::TutorialController(QWidget *_tutorializedWidget)
    : QObject(_tutorializedWidget), tutorializedWidget(_tutorializedWidget)
{
    tutorialOverlay = new TutorialOverlay(tutorializedWidget->window());

    // Make it frameless + translucent
    tutorialOverlay->setWindowFlags(tutorialOverlay->windowFlags() | Qt::FramelessWindowHint);
    tutorialOverlay->setAttribute(Qt::WA_TranslucentBackground);

    // hide until start
    tutorialOverlay->hide();

    connect(tutorialOverlay, &TutorialOverlay::nextStep, this, &TutorialController::nextStep);
    connect(tutorialOverlay, &TutorialOverlay::skipTutorial, this, [this]() { tutorialOverlay->hide(); });
}

void TutorialController::addSequence(const TutorialSequence &seq)
{
    sequences.append(seq);
}

void TutorialController::start()
{
    if (sequences.isEmpty()) {
        return;
    }

    QTimer::singleShot(0, this, [this]() {
        QWidget *win = tutorializedWidget->window();
        tutorialOverlay->parentResized();
        tutorialOverlay->setGeometry(QRect(QPoint(0, 0), win->size()));
        tutorialOverlay->show();
        tutorialOverlay->raise();
        tutorialOverlay->parentResized();

        currentSequence = 0;
        currentStep = 0;
        showStep();
    });
}

void TutorialController::nextStep()
{
    // advance within sequence
    currentStep++;

    if (currentSequence < 0) {
        return; // defensive in case we haven't started yet
    }

    if (currentStep >= sequences[currentSequence].steps.size()) {
        // advance to next sequence
        nextSequence();
        return;
    }

    showStep();
}

void TutorialController::prevStep()
{
    if (currentSequence < 0) {
        return; // defensive in case we haven't started yet
    }

    if (currentStep == 0) {
        prevSequence();
        return;
    }

    currentStep--;
    showStep();
}

void TutorialController::nextSequence()
{
    if (currentSequence < 0) {
        return;
    }

    // run exit for the last step of the current sequence (showStep handles previous onExit,
    // but ensure we run it here because we're jumping sequence)
    // We'll increment sequence and then call showStep which will call the onEnter for the new step.
    currentSequence++;
    currentStep = 0;

    if (currentSequence >= sequences.size()) {
        exitTutorial();
        return;
    }

    showStep();
}

void TutorialController::prevSequence()
{
    if (currentSequence <= 0) {
        // already at first sequence -> stay
        return;
    }

    // go to last step of previous sequence
    currentSequence--;
    currentStep = sequences[currentSequence].steps.size() - 1;
    showStep();
}

void TutorialController::exitTutorial()
{
    // Run onExit for the current step if present
    if (currentSequence >= 0 && currentStep >= 0 && currentSequence < sequences.size() &&
        currentStep < sequences[currentSequence].steps.size()) {
        const auto &curStep = sequences[currentSequence].steps[currentStep];
        if (curStep.onExit) {
            curStep.onExit();
        }
    }

    tutorialOverlay->hide();

    // reset indices so start() can be called again cleanly
    currentSequence = -1;
    currentStep = -1;
}

void TutorialController::showStep()
{
    // bounds checks
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }
    const auto &seq = sequences[currentSequence];
    if (currentStep < 0 || currentStep >= seq.steps.size()) {
        return;
    }

    // run onExit for the previous step (including if previous step was in previous sequence)
    if (!(currentSequence == 0 && currentStep == 0)) {
        int prevSeq = currentSequence;
        int prevStepIndex = currentStep - 1;
        if (prevStepIndex < 0) {
            // previous is last step of previous sequence
            prevSeq = currentSequence - 1;
            if (prevSeq >= 0) {
                prevStepIndex = sequences[prevSeq].steps.size() - 1;
            } else {
                prevStepIndex = -1;
            }
        }

        if (prevSeq >= 0 && prevStepIndex >= 0) {
            const auto &previousStep = sequences[prevSeq].steps[prevStepIndex];
            if (previousStep.onExit) {
                previousStep.onExit();
            }
        }
    }

    // current step
    const auto &step = seq.steps[currentStep];

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
