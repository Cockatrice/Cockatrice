#include "shader_banner_widget.h"

#include "banner_shader_config.h"

#include <QPainter>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QResizeEvent>
#include <QStackedLayout>

namespace
{
// Near-black base palette -- the background is dark and quiet so the green
// accent stands out.
constexpr QRgb kColorA = 0x1A1A20;
constexpr QRgb kColorB = 0x0E0E12;
constexpr QRgb kAccent = 0x8BDD6B;
} // namespace

class GradientFallbackWidget : public QWidget
{
public:
    using QWidget::QWidget;

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        QLinearGradient gradient(0, 0, width(), height());
        gradient.setColorAt(0.0, QColor(kColorA));
        gradient.setColorAt(1.0, QColor(kColorB));
        painter.fillRect(rect(), gradient);
    }
};

BannerHost::BannerHost(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(150);

    stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    fallback = new GradientFallbackWidget(this);
    stack->addWidget(fallback);

    quickWidget = new QQuickWidget(this);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    config = new BannerShaderConfig(quickWidget->engine());
    quickWidget->rootContext()->setContextProperty("bannerConfig", config);
    quickWidget->setSource(QUrl("qrc:/onboarding/qml/BrandBanner.qml"));

    if (quickWidget->status() == QQuickWidget::Error) {
        activateFallback();
    } else {
        connect(quickWidget, &QQuickWidget::sceneGraphError, this, &BannerHost::onSceneGraphFailed);
        stack->addWidget(quickWidget);
        stack->setCurrentWidget(quickWidget);
    }

    connect(&clock, &QTimer::timeout, this, &BannerHost::tick);
    clock.setInterval(16); // ~60fps; the shader itself is cheap, this is just a wall clock

    applyMotifPreset(currentMotif);
    updateAspect();
}

void BannerHost::activateFallback()
{
    if (usingFallback) {
        return;
    }
    usingFallback = true;
    clock.stop();
    stack->setCurrentWidget(fallback);

    if (quickWidget) {
        quickWidget->deleteLater(); // takes BannerShaderConfig (parented to its engine) with it
        quickWidget = nullptr;
        config = nullptr;
    }
}

void BannerHost::onSceneGraphFailed()
{
    activateFallback();
}

void BannerHost::setMotif(Motif motif)
{
    currentMotif = motif;
    applyMotifPreset(motif);
}

BannerHost::Preset BannerHost::presetFor(Motif motif)
{
    // speed/seed tuned per motif so e.g. the network "pulse" (Account) reads
    // at a deliberately calmer cadence than the data "scan" lines
    // (Preferences), even though both come from the same shader.
    switch (motif) {
        case Motif::Welcome:
            return {0.0, 0.6, 0.15};
        case Motif::CardDatabase:
            return {1.0, 1.3, 0.42};
        case Motif::Theming:
            return {2.0, 1.2, 0.73};
        case Motif::Account:
            return {3.0, 0.8, 0.28};
        case Motif::Preferences:
            return {4.0, 1.0, 0.61};
        case Motif::Finish:
            return {5.0, 1.0, 0.91};
    }
    return {0.0, 0.6, 0.15};
}

void BannerHost::applyMotifPreset(Motif motif)
{
    if (usingFallback || !config) {
        return;
    }

    const Preset p = presetFor(motif);

    config->setColorA(QColor(kColorA));
    config->setColorB(QColor(kColorB));
    config->setAccent(QColor(kAccent));
    config->setLogoVisible(motif == Motif::Welcome);

    if (isFirstApply) {
        // Nothing on screen yet -- write straight into the front bank, no
        // crossfade needed for the very first paint.
        config->setModeA(p.mode);
        config->setSpeedA(p.speed);
        config->setSeedA(p.seed);
        config->setFrontIsA(true);
        isFirstApply = false;
        return;
    }

    // Write the new preset into whichever bank is currently hidden, then
    // flip which one is front. QML's opacity Behavior does the actual
    // crossfade -- BannerHost never animates anything itself.
    if (config->frontIsA()) {
        config->setModeB(p.mode);
        config->setSpeedB(p.speed);
        config->setSeedB(p.seed);
        config->setFrontIsA(false);
    } else {
        config->setModeA(p.mode);
        config->setSpeedA(p.speed);
        config->setSeedA(p.seed);
        config->setFrontIsA(true);
    }
}

void BannerHost::updateAspect()
{
    if (config && height() > 0) {
        config->setAspect(qreal(width()) / qreal(height()));
    }
}

void BannerHost::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateAspect();
}

void BannerHost::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!usingFallback) {
        elapsed.restart();
        clock.start();
    }
}

void BannerHost::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    clock.stop();
}

void BannerHost::tick()
{
    if (config) {
        qreal t = elapsed.elapsed() / 1000.0;
        config->setTime(t);
        // Visible breathing for the logo: oscillates between 0.0 and 1.0
        qreal glow = 0.5 + 0.5 * qSin(t * 0.4);
        config->setLogoGlow(glow);
    }
}
