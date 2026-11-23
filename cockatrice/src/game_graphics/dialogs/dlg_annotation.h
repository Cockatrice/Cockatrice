/**
 * @file dlg_annotation.h
 * @ingroup GameDialogs
 */

#ifndef COCKATRICE_DLG_ANNOTATION_H
#define COCKATRICE_DLG_ANNOTATION_H

#include <QInputDialog>

class AnnotationDialog : public QInputDialog
{
    Q_OBJECT
    void keyPressEvent(QKeyEvent *e) override;

public:
    explicit AnnotationDialog(QWidget *parent = nullptr) : QInputDialog(parent)
    {
    }
};

#endif // COCKATRICE_DLG_ANNOTATION_H
