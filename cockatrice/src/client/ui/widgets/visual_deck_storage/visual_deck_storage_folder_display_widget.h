#ifndef VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
#define VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H

#include "../general/display/banner_widget.h"
#include "../general/layout_containers/flow_widget.h"
#include "visual_deck_storage_widget.h"

#include <QVBoxLayout>
#include <QWidget>

class VisualDeckStorageFolderDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    VisualDeckStorageFolderDisplayWidget(QWidget *parent,
                                         VisualDeckStorageWidget *_visualDeckStorageWidget,
                                         QString _filePath,
                                         bool canBeHidden);
    void refreshUi();
    void createWidgetsForFiles();
    void createWidgetsForFolders();
    QStringList gatherAllTagsFromFlowWidget() const;
    [[nodiscard]] QStringList getAllFiles() const;
    [[nodiscard]] QStringList getAllSubFolders() const;
    FlowWidget *getFlowWidget() const
    {
        return flowWidget;
    };

public slots:
    void updateVisibility();
    bool checkVisibility();

private:
    QVBoxLayout *layout;
    VisualDeckStorageWidget *visualDeckStorageWidget;
    QString filePath;
    BannerWidget *header;
    QWidget *container;
    QVBoxLayout *containerLayout;
    FlowWidget *flowWidget;
};

#endif // VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
