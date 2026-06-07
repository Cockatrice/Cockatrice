#ifndef COCKATRICE_USER_CARD_ART_SETTINGS_DIALOG_H
#define COCKATRICE_USER_CARD_ART_SETTINGS_DIALOG_H

#include "user_list_painter.h"

#include <QDialog>
#include <QPixmap>

class QCompleter;
class QLineEdit;
class QDoubleSpinBox;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class CardSearchModel;
class CardCompleterProxyModel;

class CardArtPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardArtPreviewWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    void setParams(const CardArtParams &params);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap sourcePixmap;
    CardArtParams params;
};

class UserCardArtSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserCardArtSettingsDialog(const CardArtParams &initial = {}, QWidget *parent = nullptr);

    CardArtParams params() const;

private slots:
    void onCardNameChanged(const QString &name);
    void onParamChanged();

private:
    void setupUi();
    void initializeSearchBar();
    QDoubleSpinBox *makeSpinBox(double min, double max, double value, double step);

    QLineEdit *searchBar;
    QCompleter *completer;
    CardDatabaseModel *cardDatabaseModel;
    CardDatabaseDisplayModel *cardDatabaseDisplayModel;
    CardSearchModel *searchModel;
    CardCompleterProxyModel *proxyModel;

    QDoubleSpinBox *marginLSpin;
    QDoubleSpinBox *marginRSpin;
    QDoubleSpinBox *verticalOffsetSpin;
    QDoubleSpinBox *zoomSpin;
    CardArtPreviewWidget *preview;

    QPixmap currentPixmap;
    CardArtParams currentParams;
};

#endif // COCKATRICE_USER_CARD_ART_SETTINGS_DIALOG_H