#include "tutorial_controller.h"

#include "../../../../client/settings/cache_settings.h"

#include <QComboBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>
#include <libcockatrice/settings/interface_settings.h>

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

TutorialController::~TutorialController()
{
    // The overlay is parented to the top-level window, which outlives the widget
    // this controller is attached to (e.g. a closed tab), so it must be cleaned
    // up explicitly when the controller goes away without exitTutorial().
    if (tutorialOverlay) {
        tutorialOverlay->hide();
        tutorialOverlay->deleteLater();
    }
    tutorialOverlay = nullptr;
}

void TutorialController::addSequence(const TutorialSequence &seq)
{
    sequences.append(seq);
}

void TutorialController::start()
{
    if (sequences.isEmpty() || tutorialCompleted) {
        return;
    }

    QTimer::singleShot(0, this, [this]() {
        QWidget *win = tutorializedWidget->window();

        tutorialOverlay->setParent(win); // triggers changeEvent and installs filter
        tutorialOverlay->setGeometry(win->rect());
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
    if (currentSequence < 0) {
        return;
    }

    runExitForCurrentStep();

    if (currentStep >= sequences[currentSequence].steps.size() - 1) {
        nextSequence();
        return;
    }

    currentStep++;
    showStep();
}

void TutorialController::prevStep()
{
    if (currentSequence < 0) {
        return;
    }

    runExitForCurrentStep();

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
        // Already at the very first step — just re-show it
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
    cleanupValidationMonitoring();
    if (tutorialOverlay) {
        tutorialOverlay->hide();
        tutorialOverlay->deleteLater();
    }
    tutorialOverlay = nullptr;
    currentSequence = -1;
    currentStep = -1;
    tutorialCompleted = true;
    SettingsCache::instance().userInterface().setTutorialCompleted(true);
    deleteLater();
}

void TutorialController::runExitForCurrentStep()
{
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }
    const auto &steps = sequences[currentSequence].steps;
    if (currentStep < 0 || currentStep >= steps.size()) {
        return;
    }
    if (steps[currentStep].onExit) {
        steps[currentStep].onExit();
    }
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

    cleanupValidationMonitoring();
    advanceScheduled = false;

    const auto &step = seq.steps[currentStep];

    if (step.onEnter) {
        step.onEnter();
    }

    tutorialOverlay->setTargetWidget(step.targetWidget);
    tutorialOverlay->setText(step.text);
    tutorialOverlay->setInteractive(step.requiresInteraction, step.allowClickThrough);

    if (!step.customInteractionHint.isEmpty()) {
        tutorialOverlay->setInteractionHint(step.customInteractionHint);
    } else if (step.requiresInteraction) {
        tutorialOverlay->setInteractionHint("👆 Click the highlighted area to continue");
    } else {
        tutorialOverlay->setInteractionHint("");
    }

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
    if (step.validationTiming == ValidationTiming::OnSignal && step.signalHook) {
        validationConnection = step.signalHook(this);
        return;
    }

    // Handle OnChange validation - widget-specific
    if (step.validationTiming == ValidationTiming::OnChange && step.validator) {
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(step.targetWidget)) {
            validationConnection =
                connect(lineEdit, &QLineEdit::textChanged, this, &TutorialController::checkValidation);
        } else if (QTextEdit *textEdit = qobject_cast<QTextEdit *>(step.targetWidget)) {
            validationConnection =
                connect(textEdit, &QTextEdit::textChanged, this, &TutorialController::checkValidation);
        } else if (QPlainTextEdit *plainText = qobject_cast<QPlainTextEdit *>(step.targetWidget)) {
            validationConnection =
                connect(plainText, &QPlainTextEdit::textChanged, this, &TutorialController::checkValidation);
        } else if (QComboBox *combo = qobject_cast<QComboBox *>(step.targetWidget)) {
            validationConnection = connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                                           &TutorialController::checkValidation);
        }
    }
}

void TutorialController::cleanupValidationMonitoring()
{
    if (validationConnection) {
        disconnect(validationConnection);
        validationConnection = QMetaObject::Connection();
    }
}

void TutorialController::checkValidation()
{
    if (currentSequence < 0 || currentSequence >= sequences.size()) {
        return;
    }
    if (currentStep < 0 || currentStep >= sequences[currentSequence].steps.size()) {
        return;
    }

    const auto &step = sequences[currentSequence].steps[currentStep];

    if (step.validator) {
        bool isValid = step.validator();

        if (isValid) {
            // Clear any validation hints
            tutorialOverlay->showValidationHint("");

            // Auto-advance if enabled
            if (step.autoAdvanceOnValid && !advanceScheduled) {
                advanceScheduled = true;
                QTimer::singleShot(500, this, [this]() {
                    advanceScheduled = false;
                    nextStep();
                });
            }
        }
    }
}