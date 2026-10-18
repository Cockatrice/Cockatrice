#ifndef COCKATRICE_PALETTE_EDITOR_DIALOG_H
#define COCKATRICE_PALETTE_EDITOR_DIALOG_H

#include "../theme_config.h"

#include <QColor>
#include <QDialog>
#include <QMap>
#include <QString>
#include <qtmetamacros.h>

class QTimer;
class QLabel;
class QComboBox;
class QDialogButtonBox;
class QPushButton;
class PaletteGridWidget;
class FontGridWidget;
class QuickSetupPanel;
class QFrame;
class QWidget;
class QCloseEvent;

class PaletteEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PaletteEditorDialog(const QString &themeDirPath, const QString &themeName, QWidget *parent = nullptr);
    void loadSchemes();
    void reject() override;

private slots:
    void onSave();
    void onApply();
    void onReset();
    void onRevertToDefault();
    void onSchemeChanged(const QString &scheme);
    void onGenerateFromAccent(const QColor &accent, int intensity);

private:
    void setupUi();
    void retranslateUi();
    void refreshChromePalettes();
    void updateAdvancedHeight();
    void loadScheme(const QString &scheme); // snapshot current, switch, load
    void seedAccentFromScheme(const QString &scheme);
    void loadFonts(); // shared per theme, not split by light/dark

    // Sub-widgets
    QWidget *header;
    QLabel *titleLabel;
    QLabel *editingLabel;
    QuickSetupPanel *quickSetupPanel = nullptr;
    PaletteGridWidget *paletteGrid = nullptr;
    QPushButton *paletteGridToggleButton = nullptr;
    QFrame *paletteGridSeparator = nullptr;
    FontGridWidget *fontGrid = nullptr;
    QPushButton *fontToggleButton = nullptr;
    QFrame *fontSeparator = nullptr;
    QWidget *fontSection = nullptr;
    QLabel *fontNoteLabel = nullptr;
    QWidget *footer;
    QComboBox *schemeComboBox = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
    QPushButton *resetBtn = nullptr;
    QPushButton *saveBtn = nullptr;
    QPushButton *closeBtn = nullptr;
    QPushButton *revertButton = nullptr;

    // State
    QString themeDirPath;
    QString themeName;
    QString loadedScheme;
    // Directory writes are directed to; may differ from themeDirPath when the
    // latter is read-only (e.g. a built-in / system theme directory).
    QString saveDir;

    QMap<QString, PaletteConfig> workingConfig;
    QMap<QString, PaletteConfig> savedConfig;

    // Fonts are shared per theme (fonts.toml, not split by light/dark), so a
    // single saved config suffices; the live edits are read from fontGrid.
    FontConfig savedFontConfig;
    // Set only by user-driven fontsChanged after the initial load completes.
    // Gates both previewFonts and the fonts.toml write so a palette-only Save
    // never materializes fallback fonts to disk.
    bool fontsDirty = false;

    QTimer *autoApplyTimer = nullptr;

protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
};

#endif // COCKATRICE_PALETTE_EDITOR_DIALOG_H
