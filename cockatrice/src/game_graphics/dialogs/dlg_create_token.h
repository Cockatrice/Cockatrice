/**
 * @file dlg_create_token.h
 * @ingroup GameDialogs
 */
//! \todo Document this file.

#ifndef DLG_CREATETOKEN_H
#define DLG_CREATETOKEN_H

#include <QDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <qtmetamacros.h>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QTreeView;
class CardDatabaseModel;
class TokenDisplayModel;
class CardInfoPictureWidget;
class QModelIndex;
class QWidget;

struct TokenInfo
{
    QString name;
    QString color;
    QString pt;
    QString annotation;
    bool destroy = true;
    bool faceDown = false;
    QString providerId;
};

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
    CardInfoPictureWidget *pic;
    QTreeView *chooseTokenView;

    void updateSearchFieldWithoutUpdatingFilter(const QString &newValue) const;
};

#endif
