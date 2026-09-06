#ifndef SHADER_BANNER_WIDGET_H
#define SHADER_BANNER_WIDGET_H

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>

class BannerShaderConfig;
class QQuickWidget;
class GradientFallbackWidget;
class QStackedLayout;

/** @brief Onboarding banner: a subtle, looping brand-shader animation, one of six
 *         per-page "motifs" driving the same prebaked fragment shader
 *         (onboarding/shaders/brand_banner.frag) with different uniform values, so
 *         every page feels distinct but unmistakably part of the same family.
 *
 * Motif switches crossfade smoothly (see BrandBanner.qml's two stacked
 * ShaderEffect layers + Behavior on opacity) rather than cutting instantly
 * -- BannerHost just writes the new preset into whichever layer is
 * currently hidden and flips BannerShaderConfig::frontIsA; QML handles the
 * actual animation declaratively.
 *
 * Falls back to a static two-stop gradient (no shader, no QQuickWidget) if
 * the platform's Qt Quick scenegraph can't initialize -- e.g. software
 * rendering only, or a CI/VM environment with no GPU -- so onboarding
 * never blocks or blanks out over a graphics driver problem. The fallback
 * is permanent for the lifetime of this widget once triggered. */
class BannerHost : public QWidget
{
    Q_OBJECT

public:
    enum class Motif
    {
        Welcome,
        CardDatabase,
        Theming,
        Account,
        Preferences,
        Finish,
    };

    explicit BannerHost(QWidget *parent = nullptr);

    void setMotif(Motif motif);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void tick();
    void onSceneGraphFailed();

private:
    struct Preset
    {
        qreal mode;
        qreal speed;
        qreal seed;
    };

    static Preset presetFor(Motif motif);

    void applyMotifPreset(Motif motif);
    void updateAspect();
    void activateFallback();

    QStackedLayout *stack;
    QQuickWidget *quickWidget = nullptr;
    BannerShaderConfig *config = nullptr;
    GradientFallbackWidget *fallback = nullptr;

    QTimer clock;
    QElapsedTimer elapsed;
    Motif currentMotif = Motif::Welcome;
    bool usingFallback = false;
    bool isFirstApply = true;
};

#endif // SHADER_BANNER_WIDGET_H
