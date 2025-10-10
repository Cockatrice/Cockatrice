/**
 * @file dlg_edit_tokens.h
 * @ingroup GameDialogs
 * @brief TODO: Document this.
 */

#ifndef DLG_EDIT_TOKENS_H
#define DLG_EDIT_TOKENS_H

#include <QDialog>
#include <libcockatrice/card/card_info.h>

class QModelIndex;
class CardDatabaseModel;
class TokenEditModel;
class QLabel;
class QComboBox;
class QLineEdit;
class QTreeView;

class DlgEditTokens : public QDialog
{
    Q_OBJECT
private slots:
    void tokenSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void colorChanged(int _colorIndex);
    void ptChanged(const QString &_pt);
    void annotationChanged(const QString &_annotation);

    void actAddToken();
    void actRemoveToken();

private:
    CardInfoPtr currentCard;
    CardDatabaseModel *databaseModel;
    TokenEditModel *cardDatabaseDisplayModel;
    QStringList predefinedTokens;
    QLabel *nameLabel, *colorLabel, *ptLabel, *annotationLabel;
    QComboBox *colorEdit;
    QLineEdit *nameEdit, *ptEdit, *annotationEdit;
    QTreeView *chooseTokenView;

public:
    explicit DlgEditTokens(QWidget *parent = nullptr);
};

#endif
