#include "shader_banner_widget.h"

#include "../../theme_manager.h"
#include "banner_shader_config.h"
#include "brand_colors.h"

#include <QApplication>
#include <QPainter>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QResizeEvent>
#include <QStackedLayout>

namespace
{
// Curated near-black stage -- used only when the active palette resolves no
// usable window colour. Matches the banner's original design (dark and quiet
// so the accent stands out) and satisfies design-plans §2.1's "identity
// survives a bare palette".
constexpr QRgb kFallbackColorA = 0x1A1A20;
constexpr QRgb kFallbackColorB = 0x0E0E12;

struct SuggestedColors
{
    QColor colorA;
    QColor colorB;
    QColor accent;
    QColor glowColor;
    qreal vignetteMin = 0.62;
    bool lightStage = false;
};

SuggestedColors suggestedBannerColors()
{
    const QPalette &pal = qApp->palette();
    const QColor window = pal.color(QPalette::Active, QPalette::Window);
    const QColor highlight = pal.color(QPalette::Active, QPalette::Highlight);
    if (!window.isValid() || !highlight.isValid()) {
        return {
            QColor(kFallbackColorA), QColor(kFallbackColorB), kCockatriceBrandGreen, QColor(Qt::white), 0.62, false};
    }

    // Dress the stage for the scheme so the banner never fights the
    // surrounding window in either mode. Dark palettes keep the original
    // quiet near-black stage (lightness 29 → 16) with the theme's window
    // hue; light palettes get a pastel "frosted accent" treatment built from
    // the Highlight hue instead of a plain near-white copy: a gentle mint
    // wash that clearly belongs to the theme.
    const qreal luma = 0.299 * window.red() + 0.587 * window.green() + 0.114 * window.blue();
    const bool lightStage = luma > 115.0;
    if (lightStage) {
        const int hue = highlight.hslHue();
        // Achromatic accents (grey) get a neutral near-white stage instead.
        const int stageSat = hue < 0 ? 0 : 35;
        const int hueSafe = hue < 0 ? 0 : hue;
        auto pastel = [hueSafe, stageSat](int lightness) { return QColor::fromHsl(hueSafe, stageSat, lightness); };
        // Brightness-lifted accent for additive glows: Highlight on a light
        // stage must be mid-bright to read (the shipped light Highlight is a
        // deep green that washes out additively against white).
        const int accentLightness = qBound(120, highlight.lightness() + 70, 165);
        const int accentSaturation = hue < 0 ? 0 : qMax(highlight.hslSaturation(), 140);
        const QColor liftedAccent = hue < 0 ? highlight : QColor::fromHsl(hueSafe, accentSaturation, accentLightness);
        // The centre glow uses the deep Highlight itself -- a coloured halo
        // behind the dark logo instead of a white blowout.
        return {pastel(247), pastel(231), liftedAccent, highlight, 0.88, true};
    }

    // Dark stage: force the window hue down to the banner's curated darkness,
    // scaling saturation away so chromatic palettes tint it without going
    // muddy. White glow and the original strong vignette stay untouched.
    auto stage = [&window](int lightness) {
        const int hue = window.hslHue();
        const int saturation = hue < 0 ? 0 : qBound(0, qRound(window.hslSaturation() * (lightness / 40.0)), 255);
        return QColor::fromHsl(hue, saturation, lightness);
    };
    return {stage(29), stage(16), QColor(highlight), QColor(Qt::white), 0.62, false};
}
} // namespace

class GradientFallbackWidget : public QWidget
{
public:
    using QWidget::QWidget;

    void setColors(const QColor &a, const QColor &b)
    {
        if (a != colorA || b != colorB) {
            colorA = a;
            colorB = b;
        }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        QLinearGradient gradient(0, 0, width(), height());
        gradient.setColorAt(0.0, colorA);
        gradient.setColorAt(1.0, colorB);
        painter.fillRect(rect(), gradient);
    }

private:
    QColor colorA{QColor(kFallbackColorA)};
    QColor colorB{QColor(kFallbackColorB)};
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

    connect(themeManager, &ThemeManager::themeChanged, this, &BannerHost::applyThemeColors);
    applyThemeColors();

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

    config->setColorA(bannerColorA);
    config->setColorB(bannerColorB);
    config->setAccent(bannerAccent);
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

void BannerHost::applyThemeColors()
{
    const SuggestedColors colors = suggestedBannerColors();
    bannerColorA = colors.colorA;
    bannerColorB = colors.colorB;
    bannerAccent = colors.accent;

    if (usingFallback) {
        fallback->setColors(bannerColorA, bannerColorB);
        fallback->update();
    } else if (config) {
        config->setColorA(bannerColorA);
        config->setColorB(bannerColorB);
        config->setAccent(bannerAccent);
        config->setGlowColor(colors.glowColor);
        config->setVignetteMin(colors.vignetteMin);
        config->setLogoDark(colors.lightStage);
    }
}

void BannerHost::tick()
{
    // Palette previews (e.g. accent drags in the wizard's QuickSetupPanel)
    // apply qApp->palette() without firing themeChanged, so re-derive here;
    // BannerShaderConfig's setters are equality-guarded, so this is a no-op
    // unless the colours actually changed.
    applyThemeColors();
    if (config) {
        qreal t = elapsed.elapsed() / 1000.0;
        config->setTime(t);
        // Visible breathing for the logo: oscillates between 0.0 and 1.0
        qreal glow = 0.5 + 0.5 * qSin(t * 0.4);
        config->setLogoGlow(glow);
    }
}
