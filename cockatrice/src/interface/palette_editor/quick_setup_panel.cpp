#include "quick_setup_panel.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

QuickSetupPanel::QuickSetupPanel(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(10);

    heading = new QLabel(this);
    heading->setTextFormat(Qt::RichText);

    accentLabel = new QLabel(this);
    accentButton = new ColorButton(this);
    accentButton->setColor(QColor(20, 140, 60));

    intensityLabel = new QLabel(this);

    labelLow = new QLabel(this);
    labelHigh = new QLabel(this);
    QFont small = labelLow->font();
    small.setPointSizeF(small.pointSizeF() * 0.82);
    labelLow->setFont(small);
    labelHigh->setFont(small);
    QPalette dimmed = labelLow->palette();
    dimmed.setColor(QPalette::WindowText, qApp->palette().color(QPalette::Mid));
    labelLow->setPalette(dimmed);
    labelHigh->setPalette(dimmed);

    intensitySlider = new QSlider(Qt::Horizontal, this);
    intensitySlider->setRange(0, 100);
    intensitySlider->setValue(70);
    intensitySlider->setFixedWidth(160);

    intensityPercentageLabel = new QLabel(this);
    intensityPercentageLabel->setFixedWidth(34);
    intensityPercentageLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(heading);
    layout->addSpacing(6);
    layout->addWidget(accentLabel);
    layout->addWidget(accentButton);
    layout->addSpacing(12);
    layout->addWidget(intensityLabel);
    layout->addWidget(labelLow);
    layout->addWidget(intensitySlider);
    layout->addWidget(labelHigh);
    layout->addWidget(intensityPercentageLabel);
    layout->addStretch();

    connect(intensitySlider, &QSlider::valueChanged, this, [this](int v) {
        intensityPercentageLabel->setText(tr("%1%").arg(v));
        emit valueChanged(accentButton->getColor(), v);
    });
    connect(accentButton, &ColorButton::colorChanged, this,
            [this](const QColor &c) { emit valueChanged(c, intensitySlider->value()); });
    retranslateUi();
}

void QuickSetupPanel::retranslateUi()
{
    heading->setText(tr("<b>Quick Setup</b>"));
    heading->setToolTip(tr("Generate all palette roles automatically from a single accent colour"));
    accentLabel->setText(tr("Accent:"));
    accentButton->setToolTip(tr("Primary hue. Used directly for highlights and links.\n"
                                "At high intensity it also tints buttons and backgrounds."));
    intensityLabel->setText(tr("Intensity:"));
    labelLow->setText(tr("Subtle"));
    labelHigh->setText(tr("Full colour"));
    intensitySlider->setToolTip(tr("0–30  Subtle tint — only highlights and links change hue\n"
                                   "30–70  Accented — buttons, tooltips, and borders join in\n"
                                   "70–100 Full colour — backgrounds, everything"));
    intensityPercentageLabel->setText(tr("70%"));
}

QColor QuickSetupPanel::accentColor() const
{
    return accentButton->getColor();
}

int QuickSetupPanel::intensity() const
{
    return intensitySlider->value();
}

void QuickSetupPanel::setAccentColor(const QColor &c)
{
    accentButton->setColor(c);
}