#ifndef DLG_DEFAULT_TAGS_EDITOR_H
#define DLG_DEFAULT_TAGS_EDITOR_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>

class DlgDefaultTagsEditor : public QDialog {
    Q_OBJECT

public:
    explicit DlgDefaultTagsEditor(QWidget *parent = nullptr);

    private slots:
        void addItem();
    void deleteItem(QListWidgetItem *item);
    void confirmChanges();

private:
    QListWidget *listWidget;
    QLineEdit *inputField;
    QPushButton *addButton;
    QPushButton *confirmButton;
    QPushButton *cancelButton;

    void loadStringList();
    void retranslateUi();
};

#endif //DLG_DEFAULT_TAGS_EDITOR_H
