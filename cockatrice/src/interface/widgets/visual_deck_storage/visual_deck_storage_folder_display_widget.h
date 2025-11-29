/**
 * @file visual_deck_storage_folder_display_widget.h
 * @ingroup VisualDeckStorageWidgets
 * @brief TODO: Document this.
 */

#ifndef VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
#define VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H

#include "../general/display/banner_widget.h"
#include "../general/layout_containers/flow_widget.h"

class VisualDeckStorageWidget;
class VisualDeckStorageFolderDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    VisualDeckStorageFolderDisplayWidget(QWidget *parent,
                                         VisualDeckStorageWidget *_visualDeckStorageWidget,
                                         QString _filePath,
                                         bool canBeHidden,
                                         bool _showFolders);
    void refreshUi();
    void createWidgetsForFiles();
    void createWidgetsForFolders();
    void flattenFolderStructure();
    [[nodiscard]] QStringList gatherAllTagsFromFlowWidget() const;
    [[nodiscard]] FlowWidget *getFlowWidget() const
    {
        return flowWidget;
    }

public slots:
    void updateVisibility(bool recursive = true);
    bool checkVisibility();
    void updateShowFolders(bool enabled);

private:
    bool showFolders;
    QVBoxLayout *layout;
    VisualDeckStorageWidget *visualDeckStorageWidget;
    QString filePath;
    BannerWidget *header;
    QWidget *container;
    QVBoxLayout *containerLayout;
    FlowWidget *flowWidget;
};

#endif // VISUAL_DECK_STORAGE_FOLDER_DISPLAY_WIDGET_H
