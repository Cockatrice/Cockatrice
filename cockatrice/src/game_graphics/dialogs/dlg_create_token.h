/**
 * @file dlg_create_token.h
 * @ingroup GameDialogs
 */
//! \todo Document this file.

#ifndef DLG_CREATETOKEN_H
#define DLG_CREATETOKEN_H

#include "../../interface/widgets/printing_selector/printing_selector_selection_host.h"

#include <QDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <libcockatrice/game/player/token_info.h>
#include <qtmetamacros.h>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QTabWidget;
class QTreeView;
class CardDatabaseModel;
class TokenDisplayModel;
class TokenPreviewWidget;
class PrintingSelector;
class QModelIndex;
class QWidget;

class DlgCreateToken : public QDialog
{
    Q_OBJECT
public:
    explicit DlgCreateToken(const QStringList &_predefinedTokens, QWidget *parent = nullptr);
    [[nodiscard]] TokenInfo getTokenInfo() const;

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void faceDownCheckBoxToggled(bool checked);
    void tokenSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void updateSearch(const QString &search);
    void syncPrintingsToToken();
    void updatePreview();
    void actChooseTokenFromAll(bool checked);
    void actChooseTokenFromDeck(bool checked);
    void actOk();
    void actReject();

private:
    CardDatabaseModel *cardDatabaseModel;
    TokenDisplayModel *cardDatabaseDisplayModel;
    QStringList predefinedTokens;
    QLabel *nameLabel, *colorLabel, *ptLabel, *annotationLabel;
    QComboBox *colorEdit;
    QLineEdit *nameEdit, *ptEdit, *annotationEdit;
    QCheckBox *destroyCheckBox;
    QCheckBox *faceDownCheckBox;
    QRadioButton *chooseTokenFromAllRadioButton, *chooseTokenFromDeckRadioButton;
    TokenPreviewWidget *pic;
    QTreeView *chooseTokenView;
    QTabWidget *tokenSourceTabs;
    PrintingSelector *printingSelector;
    PrintingSelectorSelectionHost printingSelectorHost;
    QString explicitProviderId;
    QString explicitProviderName;

    [[nodiscard]] TokenInfo tokenDraft() const;
    void updateSearchFieldWithoutUpdatingFilter(const QString &newValue) const;
};

#endif
