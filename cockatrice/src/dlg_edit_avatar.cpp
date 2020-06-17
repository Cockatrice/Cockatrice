#include "dlg_edit_avatar.h"

#include <QBuffer>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DlgEditAvatar::DlgEditAvatar(QWidget *parent) : QDialog(parent), image()
{
    imageLabel = new QLabel(tr("No image chosen."));
    imageLabel->setFixedSize(400, 200);
    imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    imageLabel->setStyleSheet("border: 1px solid #000");

    textLabel = new QLabel(tr("To change your avatar, choose a new image.\nTo remove your current avatar, confirm "
                              "without choosing a new image."));
    browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, SIGNAL(clicked()), this, SLOT(actBrowse()));

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(imageLabel, 0, 0, 1, 2);
    grid->addWidget(textLabel, 1, 0);
    grid->addWidget(browseButton, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(grid);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Change avatar"));
    setFixedHeight(sizeHint().height());
    setMinimumWidth(300);
}

void DlgEditAvatar::actOk()
{
    accept();
}

void DlgEditAvatar::actBrowse()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty()) {
        imageLabel->setText(tr("No image chosen."));
        return;
    }

    QImageReader imgReader;
    imgReader.setDecideFormatFromContent(true);
    imgReader.setFileName(fileName);
    if (!imgReader.read(&image)) {
        qDebug() << "Avatar image loading failed for file:" << fileName;
        imageLabel->setText(tr("Invalid image chosen."));
        return;
    }
    imageLabel->setPixmap(QPixmap::fromImage(image).scaled(400, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QByteArray DlgEditAvatar::getImage()
{
    if (image.isNull()) {
        return QByteArray();
    }

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");
    return ba;
}
