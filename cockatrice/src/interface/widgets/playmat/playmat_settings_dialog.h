#ifndef COCKATRICE_PLAYMAT_SETTINGS_DIALOG_H
#define COCKATRICE_PLAYMAT_SETTINGS_DIALOG_H

#include <QDialog>
#include <QPixmap>
#include <libcockatrice/deck_list/deck_list.h>

class QCheckBox;
class QComboBox;
class QCompleter;
class QDoubleSpinBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QWidget;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class CardSearchModel;
class CardCompleterProxyModel;
class PlaymatPreviewWidget;

/**
 * @brief Dialog for configuring the playmat card art for a deck.
 *
 * The crop surface is the primary control: drag to pan the visible art,
 * scroll (or +/- keys) to zoom, arrow keys to nudge. Card name and printing
 * are selected below. A checkbox reveals optional numerical editors for the
 * raw PlaymatParams. These controls edit the same stored PlaymatParams that
 * ship in deck files and player properties.
 */
class PlaymatSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlaymatSettingsDialog(const CardRef &initialCard = {},
                                   const PlaymatParams &initialParams = {},
                                   QWidget *parent = nullptr);

    CardRef card() const;
    PlaymatParams params() const;

private slots:
    void onCardNameChanged(const QString &name);
    void onParamChanged();
    void reloadPreview();

private:
    void setupUi();
    void populateProviderCombo(const QString &cardName);
    void initializeSearchBar();
    void retranslateUi();
    void setNumericEditorsVisible(bool visible);
    QDoubleSpinBox *makeSpinBox(double min, double max, double value, double step);

    QLineEdit *searchBar;
    QCompleter *completer;
    CardDatabaseModel *cardDatabaseModel;
    CardDatabaseDisplayModel *cardDatabaseDisplayModel;
    CardSearchModel *searchModel;
    CardCompleterProxyModel *proxyModel;

    QComboBox *providerComboBox;

    QMetaObject::Connection pixmapUpdatedConnection;

    QLabel *cardNameLabel;
    QLabel *printingLabel;
    QLabel *previewCaptionLabel;
    QCheckBox *showNumericEditorsCheck;
    QFormLayout *controlsForm;
    QLabel *leftMarginLabel;
    QLabel *rightMarginLabel;
    QLabel *verticalOffsetLabel;
    QLabel *zoomLabel;
    QGroupBox *controlsGroup;
    QGroupBox *previewGroup;
    QPushButton *removeButton;

    QDoubleSpinBox *marginLSpin;
    QDoubleSpinBox *marginRSpin;
    QDoubleSpinBox *verticalOffsetSpin;
    QDoubleSpinBox *zoomSpin;

    PlaymatPreviewWidget *preview;

    QPixmap currentPixmap;
    CardRef currentCard;
    PlaymatParams currentParams;
};

#endif // COCKATRICE_PLAYMAT_SETTINGS_DIALOG_H
