#include "dlg_edit_avatar.h"

#include <QBuffer>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QImageReader>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <libcockatrice/utility/string_limits.h>

DlgEditAvatar::DlgEditAvatar(QWidget *parent) : QDialog(parent), image()
{
    imageLabel = new QLabel(tr("No image chosen."));
    imageLabel->setFixedSize(400, 200);
    imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    imageLabel->setStyleSheet("border: 1px solid #000");

    textLabel = new QLabel(tr("To change your avatar, choose a new image.\nTo remove your current avatar, confirm "
                              "without choosing a new image."));
    hintLabel = new QLabel(
        tr("Images are automatically downscaled to at most %1x%1 pixels.").arg(QString::number(MAX_AVATAR_DIMENSION)));
    hintLabel->setStyleSheet("color: palette(placeholderText);");
    browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &DlgEditAvatar::actBrowse);

    auto *grid = new QGridLayout;
    grid->addWidget(imageLabel, 0, 0, 1, 2);
    grid->addWidget(textLabel, 1, 0);
    grid->addWidget(browseButton, 1, 1);
    grid->addWidget(hintLabel, 2, 0, 1, 2);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgEditAvatar::actOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgEditAvatar::reject);

    auto *mainLayout = new QVBoxLayout;
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
        qCWarning(DlgEditAvatarLog) << "Avatar image loading failed for file:" << fileName;
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

    if (image.width() > MAX_AVATAR_DIMENSION || image.height() > MAX_AVATAR_DIMENSION) {
        image = image.scaled(MAX_AVATAR_DIMENSION, MAX_AVATAR_DIMENSION, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    for (;;) {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "JPG");
        if (ba.length() > MAX_FILE_LENGTH) {
            image = image.scaledToWidth(image.width() / 2); // divide the amount of pixels in four to get the size down
        } else {
            return ba;
        }
    }
}
