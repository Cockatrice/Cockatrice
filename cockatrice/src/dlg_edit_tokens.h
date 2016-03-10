#ifndef DLG_EDIT_TOKENS_H
#define DLG_EDIT_TOKENS_H

#include <QDialog>

class QModelIndex;
class CardDatabaseModel;
class TokenDisplayModel;
class QLabel;
class QComboBox;
class QLineEdit;
class QTreeView;
class CardInfo;

class DlgEditTokens : public QDialog {
    Q_OBJECT
private slots:
    void tokenSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void colorChanged(int _colorIndex);
    void ptChanged(const QString &_pt);
    void annotationChanged(const QString &_annotation);
    
    void actAddToken();
    void actRemoveToken();
private:
    CardInfo *currentCard;
    CardDatabaseModel *databaseModel;
    TokenDisplayModel *cardDatabaseDisplayModel;
    QStringList predefinedTokens;
    QLabel *nameLabel, *colorLabel, *ptLabel, *annotationLabel;
    QComboBox *colorEdit;
    QLineEdit *nameEdit, *ptEdit, *annotationEdit;
    QTreeView *chooseTokenView;
public:
    DlgEditTokens(QWidget *parent = 0);
};

#endif
