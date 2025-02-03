#ifndef DLG_CREATETOKEN_H
#define DLG_CREATETOKEN_H

#include <QDialog>
#include <QModelIndex>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QRadioButton;
class QCloseEvent;
class QTreeView;
class DeckList;
class CardDatabaseModel;
class TokenDisplayModel;
class CardInfoPictureWidget;

class DlgCreateToken : public QDialog
{
    Q_OBJECT
public:
    explicit DlgCreateToken(const QStringList &_predefinedTokens, QWidget *parent = nullptr);
    QString getName() const;
    QString getColor() const;
    QString getPT() const;
    QString getAnnotation() const;
    bool getDestroy() const;

protected:
    void closeEvent(QCloseEvent *event);
private slots:
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
    QRadioButton *chooseTokenFromAllRadioButton, *chooseTokenFromDeckRadioButton;
    CardInfoPictureWidget *pic;
    QTreeView *chooseTokenView;

    void updateSearchFieldWithoutUpdatingFilter(const QString &newValue) const;
};

#endif
