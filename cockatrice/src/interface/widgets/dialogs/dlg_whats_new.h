#ifndef DLG_WHATSNEW_H
#define DLG_WHATSNEW_H

#include <QDialog>
#include <QString>
#include <QVector>

struct FeatureIntro;
class FeatureIntroService;
class QLabel;
class QPushButton;
class QVBoxLayout;

/**
 * @file dlg_whats_new.h
 * @brief The index of recent features, where a dismissed entry stays reachable.
 *
 * One place lists every feature the client may announce: what it does, where its
 * settings live, and the state the user left it in. The in-place banner is the
 * first ask; this dialog is the second chance, and the answer to "what have I
 * missed" without opening five settings pages.
 */
class DlgWhatsNew : public QDialog
{
    Q_OBJECT

public:
    explicit DlgWhatsNew(FeatureIntroService *service, QWidget *parent = nullptr);

    /**
     * @brief Narrows the index to the entries that apply to @a surface.
     *
     * Empty shows the whole catalog. The filtered view is the same list, with a
     * "See all changes" action that clears the filter in place, so the glyph leads
     * to what is relevant without hiding the rest.
     */
    void setSurfaceFilter(const QString &surface);

signals:
    /// Deep link into the settings page that owns a feature's controls.
    void settingsRequested(const QString &pageKey, const QString &query);

protected:
    void showEvent(QShowEvent *event) override;

private:
    void rebuild();
    void addEntry(QVBoxLayout *layout, const FeatureIntro &intro);
    [[nodiscard]] QVector<const FeatureIntro *> visibleEntries() const;
    [[nodiscard]] QString stateText(const FeatureIntro &intro) const;

    FeatureIntroService *service;
    QString surfaceFilter;
    /// Show-time only: a dialog that was built but never displayed must not clear a
    /// glyph's unread mark.
    bool markedShown = false;
    QLabel *lead = nullptr;
    QPushButton *seeAll = nullptr;
    QVBoxLayout *entriesLayout = nullptr;
};

#endif // DLG_WHATSNEW_H
