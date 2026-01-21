#include "tutorial_controller.h"

#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>

TutorialController::TutorialController(QWidget *_tutorializedWidget)
    : QObject(_tutorializedWidget), tutorializedWidget(_tutorializedWidget)
{
    tutorialOverlay = new TutorialOverlay(tutorializedWidget->window());

    tutorialOverlay->setWindowFlags(tutorialOverlay->windowFlags() | Qt::FramelessWindowHint);
    tutorialOverlay->hide();

    connect(tutorialOverlay, &TutorialOverlay::nextStep, this, &TutorialController::attemptAdvance);
    connect(tutorialOverlay, &TutorialOverlay::prevStep, this, &TutorialController::prevStep);
    connect(tutorialOverlay, &TutorialOverlay::nextSequence, this, &TutorialController::nextSequence);
    connect(tutorialOverlay, &TutorialOverlay::prevSequence, this, &TutorialController::prevSequence);
    connect(tutorialOverlay, &TutorialOverlay::skipTutorial, this, &TutorialController::exitTutorial);
    connect(tutorialOverlay, &TutorialOverlay::targetClicked, this, &TutorialController::handleTargetClicked);
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

        // Reparent to make absolutely sure
        tutorialOverlay->setParent(win);
        tutorialOverlay->setGeometry(0, 0, win->width(), win->height());

        // Stack order
        tutorialOverlay->stackUnder(nullptr);
        tutorialOverlay->show();
        tutorialOverlay->raise();

        currentSequence = 0;
        currentStep = 0;
        showStep();
    });
}

void TutorialController::handleTargetClicked()
{
    if (currentSequence < 0 || currentStep < 0) {
        return;
    }

    const auto &step = sequences[currentSequence].steps[currentStep];

    // If this step requires interaction AND uses OnAdvance validation, advance when clicked
    // For OnSignal/OnChange, the click just triggers the action - validation happens via signal
    if (step.requiresInteraction && step.validationTiming == ValidationTiming::OnAdvance) {
        attemptAdvance();
    }
}

void TutorialController::attemptAdvance()
{
    if (currentSequence < 0 || currentStep < 0) {
        return;
    }

    const auto &step = sequences[currentSequence].steps[currentStep];

    // Only validate on advance if timing is set to OnAdvance
    if (step.validationTiming == ValidationTiming::OnAdvance) {
        if (!validateCurrentStep()) {
            return; // Validation failed, stay on current step
        }
    }

    // Validation passed or not required, proceed to next step
    nextStep();
}

bool TutorialController::validateCurrentStep()
{
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return true; // No validation needed
    }

    const auto &step = sequences[currentSequence].steps[currentStep];

    // If there's a validator function, check it
    if (step.validator) {
        bool valid = step.validator();
        if (!valid) {
            // Show validation hint
            tutorialOverlay->showValidationHint(step.validationHint);
            return false;
        }
    }

    return true;
}

void TutorialController::nextStep()
{
    currentStep++;

    if (currentSequence < 0) {
        return;
    }

    if (currentStep >= sequences[currentSequence].steps.size()) {
        nextSequence();
        return;
    }

    showStep();
}

void TutorialController::prevStep()
{
    if (currentSequence < 0) {
        return;
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
        currentStep = 0;
        showStep();
        return;
    }

    currentSequence--;
    currentStep = 0;
    showStep();
}

void TutorialController::exitTutorial()
{
    if (currentSequence >= 0 && currentStep >= 0 && currentSequence < sequences.size() &&
        currentStep < sequences[currentSequence].steps.size()) {
        const auto &curStep = sequences[currentSequence].steps[currentStep];
        if (curStep.onExit) {
            curStep.onExit();
        }
    }

    cleanupValidationMonitoring();
    tutorialOverlay->hide();
    currentSequence = -1;
    currentStep = -1;
}

void TutorialController::updateProgress()
{
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }

    const auto &seq = sequences[currentSequence];

    // Calculate total steps across all sequences
    int totalSteps = 0;
    int currentOverallStep = 0;

    for (int i = 0; i < sequences.size(); ++i) {
        int seqSteps = sequences[i].steps.size();
        totalSteps += seqSteps;

        if (i < currentSequence) {
            currentOverallStep += seqSteps;
        }
    }

    currentOverallStep += currentStep + 1; // +1 because steps are 0-indexed

    // Update overlay with progress info
    tutorialOverlay->setProgress(currentStep + 1,    // Current step in sequence (1-indexed)
                                 seq.steps.size(),   // Total steps in sequence
                                 currentOverallStep, // Overall step number
                                 totalSteps,         // Total steps in tutorial
                                 seq.name);          // Sequence title
}

void TutorialController::showStep()
{
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }
    const auto &seq = sequences[currentSequence];
    if (currentStep < 0 || currentStep >= seq.steps.size()) {
        return;
    }

    // Clean up validation monitoring from previous step
    cleanupValidationMonitoring();

    // Run onExit for the previous step
    if (!(currentSequence == 0 && currentStep == 0)) {
        int prevSeq = currentSequence;
        int prevStepIndex = currentStep - 1;
        if (prevStepIndex < 0) {
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

    const auto &step = seq.steps[currentStep];

    if (step.onEnter) {
        step.onEnter();
    }

    tutorialOverlay->setTargetWidget(step.targetWidget);
    tutorialOverlay->setText(step.text);
    tutorialOverlay->setInteractive(step.requiresInteraction, step.allowClickThrough);

    // Set custom interaction hint if provided
    if (!step.customInteractionHint.isEmpty()) {
        tutorialOverlay->setInteractionHint(step.customInteractionHint);
    } else if (step.requiresInteraction) {
        tutorialOverlay->setInteractionHint("👆 Click the highlighted area to continue");
    } else {
        tutorialOverlay->setInteractionHint("");
    }

    // Setup validation monitoring for this step
    setupValidationMonitoring();

    updateProgress();

    tutorialOverlay->parentResized();
    tutorialOverlay->raise();
    tutorialOverlay->update();
}

void TutorialController::setupValidationMonitoring()
{
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }
    if (currentStep < 0 || currentStep >= sequences[currentSequence].steps.size()) {
        return;
    }

    const auto &step = sequences[currentSequence].steps[currentStep];

    // Handle OnSignal validation - connect to any custom signal
    if (step.validationTiming == ValidationTiming::OnSignal && step.validator) {
        if (step.signalSource && step.signalName) {
            qInfo() << "Setting up signal-based validation for signal:" << step.signalName;
            validationConnection = connect(step.signalSource, step.signalName, this, SLOT(checkValidation()));
            if (!validationConnection) {
                qInfo() << "Warning: Failed to connect to signal" << step.signalName;
            }
        } else {
            qInfo() << "Warning: OnSignal validation timing set but signalSource or signalName is null";
        }
        return;
    }

    // Handle OnChange validation - widget-specific
    if (step.validationTiming == ValidationTiming::OnChange && step.validator) {
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(step.targetWidget)) {
            qInfo() << "Setting up validation monitoring for QLineEdit";
            validationConnection =
                connect(lineEdit, &QLineEdit::textChanged, this, &TutorialController::checkValidation);
        } else if (QTextEdit *textEdit = qobject_cast<QTextEdit *>(step.targetWidget)) {
            qInfo() << "Setting up validation monitoring for QTextEdit";
            validationConnection =
                connect(textEdit, &QTextEdit::textChanged, this, &TutorialController::checkValidation);
        } else if (QPlainTextEdit *plainText = qobject_cast<QPlainTextEdit *>(step.targetWidget)) {
            qInfo() << "Setting up validation monitoring for QPlainTextEdit";
            validationConnection =
                connect(plainText, &QPlainTextEdit::textChanged, this, &TutorialController::checkValidation);
        } else if (QComboBox *combo = qobject_cast<QComboBox *>(step.targetWidget)) {
            qInfo() << "Setting up validation monitoring for QComboBox";
            validationConnection = connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                                           &TutorialController::checkValidation);
        } else {
            qInfo() << "Warning: OnChange validation timing set but widget type not supported:"
                    << (step.targetWidget ? step.targetWidget->metaObject()->className() : "null");
        }
    }
}

void TutorialController::cleanupValidationMonitoring()
{
    if (validationConnection) {
        qInfo() << "Cleaning up validation connection";
        disconnect(validationConnection);
        validationConnection = QMetaObject::Connection();
    }
}

void TutorialController::checkValidation()
{
    qInfo() << "checkValidation() called";

    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }
    if (currentStep < 0 || currentStep >= sequences[currentSequence].steps.size()) {
        return;
    }

    const auto &step = sequences[currentSequence].steps[currentStep];

    if (step.validator) {
        bool isValid = step.validator();
        qInfo() << "Validation result:" << isValid;

        if (isValid) {
            // Clear any validation hints
            tutorialOverlay->showValidationHint("");

            // Auto-advance if enabled
            if (step.autoAdvanceOnValid) {
                qInfo() << "Auto-advancing to next step";
                QTimer::singleShot(500, this, &TutorialController::nextStep);
            }
        }
    }
}