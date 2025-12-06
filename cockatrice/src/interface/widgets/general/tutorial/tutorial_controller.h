#ifndef COCKATRICE_TUTORIAL_CONTROLLER_H
#define COCKATRICE_TUTORIAL_CONTROLLER_H

#include "tutorial_overlay.h"

#include <QObject>
#include <QVector>
#include <functional>

enum class ValidationTiming
{
    OnAdvance, // Validate when user clicks next/clicks target (default)
    OnChange,  // Validate whenever target widget changes (for text input)
    OnSignal,  // Validate when a specific signal is emitted
    Manual     // Only validate when explicitly triggered
};

struct TutorialStep
{
    QWidget *targetWidget = nullptr;
    QString text;
    std::function<void()> onEnter = nullptr;
    std::function<void()> onExit = nullptr;

    // Interactive features
    bool requiresInteraction = false;          // Must click target to advance
    bool allowClickThrough = false;            // Clicks pass through to target widget
    std::function<bool()> validator = nullptr; // Check if task completed
    QString validationHint = "";               // Show if validation fails
    ValidationTiming validationTiming = ValidationTiming::OnAdvance;

    // Auto-advance when validation passes (useful for text input)
    bool autoAdvanceOnValid = false;

    // Custom interaction hint (overrides default "Click to continue")
    QString customInteractionHint = nullptr;

    // Signal-based validation (for ValidationTiming::OnSignal)
    QObject *signalSource = nullptr;  // Object that emits the signal
    const char *signalName = nullptr; // Signal to connect to (use SIGNAL() macro)
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

public:
    explicit TutorialController(QWidget *_tutorializedWidget);

    void addSequence(const TutorialSequence &seq);
    void start();

    TutorialOverlay *getOverlay()
    {
        return tutorialOverlay;
    };

public slots:
    void nextStep();
    void prevStep();
    void nextSequence();
    void prevSequence();
    void exitTutorial();
    void handleTargetClicked(); // Handle clicks on highlighted widget
    void attemptAdvance();      // Try to advance with validation
    void checkValidation();     // Check validation for OnChange timing

private:
    void showStep();
    void updateProgress();              // Update progress indicators
    bool validateCurrentStep();         // Check if step requirements met
    void setupValidationMonitoring();   // Setup automatic validation checking
    void cleanupValidationMonitoring(); // Cleanup validation watchers

    QWidget *tutorializedWidget;
    TutorialOverlay *tutorialOverlay;
    QVector<TutorialSequence> sequences;

    int currentSequence = -1;
    int currentStep = -1;

    // For OnChange validation monitoring
    QMetaObject::Connection validationConnection;
};

#endif // COCKATRICE_TUTORIAL_CONTROLLER_H
