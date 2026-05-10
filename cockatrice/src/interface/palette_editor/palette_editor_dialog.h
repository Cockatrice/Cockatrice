#ifndef COCKATRICE_PALETTE_EDITOR_DIALOG_H
#define COCKATRICE_PALETTE_EDITOR_DIALOG_H

#include "../theme_config.h"

#include <QDialog>
#include <QFrame>
#include <QMap>

class QLabel;
class QComboBox;
class QDialogButtonBox;
class QPushButton;
class PaletteGridWidget;
class QuickSetupPanel;

class PaletteEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PaletteEditorDialog(const QString &themeDirPath, const QString &themeName, QWidget *parent = nullptr);
    void loadSchemes();

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
    void loadScheme(const QString &scheme); // snapshot current, switch, load
    void seedAccentFromScheme(const QString &scheme);

    // Sub-widgets
    QWidget *header;
    QLabel *titleLabel;
    QLabel *editingLabel;
    QuickSetupPanel *quickSetupPanel = nullptr;
    PaletteGridWidget *paletteGrid = nullptr;
    QPushButton *paletteGridToggleButton = nullptr;
    QFrame *paletteGridSeparator = nullptr;
    QWidget *footer;
    QComboBox *schemeComboBox = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
    QPushButton *resetBtn = nullptr;
    QPushButton *applyBtn = nullptr;
    QPushButton *saveBtn = nullptr;
    QPushButton *closeBtn = nullptr;
    QPushButton *revertButton = nullptr;

    // State
    QString themeDirPath;
    QString themeName;
    QString loadedScheme;

    QMap<QString, PaletteConfig> workingConfig;
    QMap<QString, PaletteConfig> savedConfig;

protected:
    void changeEvent(QEvent *e) override;
};

#endif // COCKATRICE_PALETTE_EDITOR_DIALOG_H