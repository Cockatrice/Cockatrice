#ifndef COCKATRICE_PLAYMAT_SETTINGS_DIALOG_H
#define COCKATRICE_PLAYMAT_SETTINGS_DIALOG_H

#include <QDialog>
#include <QPixmap>
#include <libcockatrice/deck_list/deck_list.h>

class QComboBox;
class QCompleter;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class CardSearchModel;
class CardCompleterProxyModel;
class PlaymatPreviewWidget;

/**
 * @brief Dialog for configuring the playmat card art for a deck.
 *
 * Allows the user to select a card from the database and adjust
 * positioning parameters (margins, zoom, vertical offset) for how
 * the card art appears as a playmat background across the
 * combined table + stack play area.
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
    void reloadPreview();
    void onParamChanged();

private:
    void setupUi();
    void populateProviderCombo(const QString &cardName);
    void initializeSearchBar();
    void retranslateUi();
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
