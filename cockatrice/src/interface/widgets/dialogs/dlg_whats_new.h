#ifndef DLG_WHATSNEW_H
#define DLG_WHATSNEW_H

#include <QDialog>
#include <QString>

class FeatureIntro;
class FeatureIntroService;
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

signals:
    /// Deep link into the settings page that owns a feature's controls.
    void settingsRequested(const QString &pageKey, const QString &query);

private:
    void rebuild();
    void addEntry(QVBoxLayout *layout, const FeatureIntro &intro);
    [[nodiscard]] QString stateText(const FeatureIntro &intro) const;

    FeatureIntroService *service;
    QVBoxLayout *entriesLayout = nullptr;
};

#endif // DLG_WHATSNEW_H
