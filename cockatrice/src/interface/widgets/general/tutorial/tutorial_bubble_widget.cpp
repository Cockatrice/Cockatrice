#include "tutorial_bubble_widget.h"

BubbleWidget::BubbleWidget(QWidget *parent) : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setStyleSheet("QFrame { background:white; border-radius:8px; }"
                  "QLabel { color:black; }");

    layout = new QGridLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(8);

    // Step counter (e.g., "Step 2 of 5")
    counterLabel = new QLabel(this);
    counterLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    counterLabel->setStyleSheet("color: #555; font-size: 11px;");

    // Overall progress (e.g., "12 of 45 total")
    progressLabel = new QLabel(this);
    progressLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    progressLabel->setStyleSheet("color: #888; font-size: 10px;");
    progressLabel->setAlignment(Qt::AlignRight);

    // Main tutorial text
    textLabel = new QLabel(this);
    textLabel->setWordWrap(true);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    textLabel->setStyleSheet("color:black;");

    // Interaction hint (e.g., "Click the highlighted area")
    interactionLabel = new QLabel(this);
    interactionLabel->setWordWrap(true);
    interactionLabel->setStyleSheet("color: #0066cc; font-style: italic; font-size: 11px;");
    interactionLabel->hide();

    // Validation hint (error message)
    validationLabel = new QLabel(this);
    validationLabel->setWordWrap(true);
    validationLabel->setStyleSheet("color: #cc3300; background: #ffe6e6; padding: 6px; "
                                   "border-radius: 4px; font-size: 11px;");
    validationLabel->hide();

    // Layout
    layout->addWidget(counterLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(progressLabel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(textLabel, 1, 0, 1, 2);
    layout->addWidget(interactionLabel, 2, 0, 1, 2);
    layout->addWidget(validationLabel, 3, 0, 1, 2);

    layout->setColumnStretch(1, 1);

    setMaximumWidth(420);

    // Timer for auto-hiding validation hints
    validationTimer = new QTimer(this);
    validationTimer->setSingleShot(true);
    connect(validationTimer, &QTimer::timeout, this, &BubbleWidget::clearValidationHint);
}

void BubbleWidget::setText(const QString &text)
{
    textLabel->setText(text);
    update();
}

void BubbleWidget::setProgress(int stepNum, int totalSteps, int overallStep, int overallTotal)
{
    // Per-sequence progress
    counterLabel->setText(QString("Step %1 of %2").arg(stepNum).arg(totalSteps));

    // Overall progress across all sequences
    progressLabel->setText(QString("(%1 of %2 total)").arg(overallStep).arg(overallTotal));
    progressLabel->show();
}

void BubbleWidget::setInteractionHint(const QString &hint)
{
    if (hint.isEmpty()) {
        interactionLabel->hide();
    } else {
        interactionLabel->setText(hint);
        interactionLabel->show();
    }
    adjustSize();
}

void BubbleWidget::setValidationHint(const QString &hint)
{
    if (hint.isEmpty()) {
        clearValidationHint();
    } else {
        validationLabel->setText("⚠️ " + hint);
        validationLabel->show();
        adjustSize();

        // Auto-hide after 4 seconds
        validationTimer->start(4000);
    }
}

void BubbleWidget::clearValidationHint()
{
    validationLabel->hide();
    adjustSize();
}