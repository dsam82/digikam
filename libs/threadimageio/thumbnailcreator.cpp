/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes.

#include <QFileInfo>
#include <QFile>
#include <QPainter>

// KDE includes.

#include <kdebug.h>
#include <kurl.h>
#include <kcomponentdata.h>
#include <kimageio.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcodecs.h>
#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kmimetype.h>
#include <kio/global.h>
#include <kio/thumbcreator.h>
#include <ktemporaryfile.h>

// LibKDcraw includes.

#include <libkdcraw/rawfiles.h>
#include <libkdcraw/kdcraw.h>

// Local includes.

#include "ddebug.h"
#include "dimg.h"
#include "dmetadata.h"
#include "jpegutils.h"
#include "thumbnailcreator.h"
#include "thumbnailcreator_p.h"

// Definitions

#define DigiKamFingerPrint "Digikam Thumbnail Generator"

namespace Digikam
{

ThumbnailCreator::ThumbnailCreator()
{
    d = new ThumbnailCreatorPriv;
    initThumbnailDirs();
}

ThumbnailCreator::ThumbnailCreator(int thumbnailSize)
{
    d = new ThumbnailCreatorPriv;
    setThumbnailSize(thumbnailSize);
    initThumbnailDirs();
}

ThumbnailCreator::~ThumbnailCreator()
{
    delete d;
}

void ThumbnailCreator::setThumbnailSize(int thumbnailSize)
{
    d->thumbnailSize = thumbnailSize;
    // on-disk thumbnail sizes according to spec
    if (d->onlyLargeThumbnails)
        d->cachedSize = 256;
    else
        d->cachedSize = (thumbnailSize <= 128) ? 128 : 256;
}

void ThumbnailCreator::setExifRotate(bool rotate)
{
    d->exifRotate = rotate;
}

void ThumbnailCreator::setOnlyLargeThumbnails(bool onlyLarge)
{
    d->onlyLargeThumbnails = onlyLarge;
}

void ThumbnailCreator::setRemoveAlphaChannel(bool removeAlpha)
{
    d->removeAlphaChannel = removeAlpha;
}


void ThumbnailCreator::setLoadingProperties(DImgLoaderObserver *observer, DRawDecoding settings)
{
    d->observer = observer;
    d->rawSettings = settings;
}

int ThumbnailCreator::thumbnailSize() const
{
    return d->thumbnailSize;
}

int ThumbnailCreator::cachedSize() const
{
    return d->cachedSize;
}

QString ThumbnailCreator::errorString() const
{
    return d->error;
}

QImage ThumbnailCreator::load(const QString &path)
{
    if (d->cachedSize <= 0)
    {
        d->error = i18n("No or invalid size specified");
        kWarning(50003) << "No or invalid size specified" << endl;
        return QImage();
    }

    QString uri = thumbnailUri(path);
    QString thumbPath = thumbnailPath(path);

    // stat the original file
    struct stat st;
    if (::stat(QFile::encodeName(path), &st) != 0)
    {
        d->error = i18n("File does not exist");
        return QImage();
    }

    // NOTE: if thumbnail have not been generated by digiKam (konqueror for example),
    //       force to recompute it, else we use it.

    QImage qimage;
    bool regenerate = true;

    qimage = loadPNG(thumbPath);
    if (!qimage.isNull())
    {
        if (qimage.text("Thumb::MTime") == QString::number(st.st_mtime) &&
            qimage.text("Software")     == QString(DigiKamFingerPrint))
            regenerate = false;
    }

    if (regenerate)
    {
        bool fromEmbeddedPreview = false;
        bool failedAtDImg = false;
        bool failedAtJPEGScaled = false;

        // -- Get the image preview --------------------------------

        // Try to extract Exif/Iptc preview first.
        if (qimage.isNull())
        {
            qimage = loadImagePreview(path);
        }

        // To speed-up thumb extraction, we now try to load the images by the file extension.
        QFileInfo info(path);
        QString ext = info.suffix().toUpper();

        if (qimage.isNull() && !ext.isEmpty())
        {
            if (ext == QString("JPEG") || ext == QString("JPG") || ext == QString("JPE"))
            {
                // use jpegutils
                loadJPEGScaled(qimage, path, d->cachedSize);
                failedAtJPEGScaled = qimage.isNull();
            }
            else if (ext == QString("PNG")
                     || ext == QString("TIFF")
                     || ext == QString("TIF"))
            {
                qimage = loadWithDImg(path);
                failedAtDImg = qimage.isNull();
            }
        }

        // Trying to load with dcraw: RAW files.
        if (qimage.isNull())
        {
            if (KDcrawIface::KDcraw::loadEmbeddedPreview(qimage, path))
                fromEmbeddedPreview = true;
        }

        if (qimage.isNull())
        {
            //TODO: Use DImg based loader instead?
            KDcrawIface::KDcraw::loadHalfPreview(qimage, path);
        }

        // Try JPEG anyway
        if (qimage.isNull() && !failedAtJPEGScaled)
        {
            // use jpegutils
            loadJPEGScaled(qimage, path, d->cachedSize);
        }

        // DImg-dependent loading methods: TIFF, PNG, everything supported by QImage
        if (qimage.isNull() && !failedAtDImg)
        {
            qimage = loadWithDImg(path);
        }

        if (qimage.isNull())
        {
            d->error = i18n("Cannot create thumbnail for %1", path);
            kWarning(50003) << "Cannot create thumbnail for " << path << endl;
            return QImage();
        }

        if (qMax(qimage.width(),qimage.height()) != d->cachedSize)
            qimage = qimage.scaled(d->cachedSize, d->cachedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // required by spec
        if (qimage.format() != QImage::Format_ARGB32)
            qimage = qimage.convertToFormat(QImage::Format_ARGB32);

        if (d->exifRotate)
            exifRotate(path, qimage, fromEmbeddedPreview);

        qimage.setText(QString("Thumb::URI").toLatin1(),   0, uri);
        qimage.setText(QString("Thumb::MTime").toLatin1(), 0, QString::number(st.st_mtime));
        qimage.setText(QString("Software").toLatin1(),     0, QString(DigiKamFingerPrint));

        KTemporaryFile temp;
        temp.setPrefix(thumbPath + "-digikam-");
        temp.setSuffix(".png");
        if (temp.open())
        {
            qimage.save(temp.fileName(), "PNG", 0);
            ::rename(QFile::encodeName(temp.fileName()),
                     QFile::encodeName(thumbPath));
        }
    }

    qimage = qimage.scaled(d->thumbnailSize, d->thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    handleAlphaChannel(qimage);

    if (qimage.isNull())
    {
        d->error = i18n("Thumbnail is null");
        kWarning(50003) << "Thumbnail is null for " << path << endl;
    }

    return qimage;

}

QImage ThumbnailCreator::loadWithDImg(const QString &path)
{
    DImg img;
    if (d->observer)
        img.load(path, d->observer, d->rawSettings);
    else
        img.load(path);
    return img.copyQImage();
}

QImage ThumbnailCreator::loadImagePreview(const QString& path)
{
    QImage image;
    DMetadata metadata(path);
    if (metadata.getImagePreview(image))
    {
        kDebug(50003) << "Use Exif/Iptc preview extraction. Size of image: "
                  << image.width() << "x" << image.height() << endl;
    }

    return image;
}

void ThumbnailCreator::handleAlphaChannel(QImage &qimage)
{
    switch (qimage.format())
    {
        case QImage::Format_RGB32:
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
            if (d->removeAlphaChannel)
            {
                QImage newImage(qimage.size(), QImage::Format_RGB32);
                // use raster paint engine
                QPainter p(&newImage);
                // blend over white, or a checkerboard?
                p.fillRect(newImage.rect(), Qt::white);
                p.drawImage(0, 0, qimage);
                qimage = newImage;
            }
            break;
        }
        default: // indexed and monochrome formats
            qimage = qimage.convertToFormat(QImage::Format_RGB32);
    }
}

void ThumbnailCreator::exifRotate(const QString& filePath, QImage& thumb, bool fromEmbeddedPreview)
{
    // Keep in sync with main version in loadsavethread.cpp

    if (DImg::fileFormat(filePath) == DImg::RAW && !fromEmbeddedPreview )
        return;

    DMetadata metadata(filePath);
    DMetadata::ImageOrientation orientation = metadata.getImageOrientation();

    if (orientation == DMetadata::ORIENTATION_NORMAL ||
        orientation == DMetadata::ORIENTATION_UNSPECIFIED)
        return;

    QMatrix matrix;

    switch (orientation)
    {
        case DMetadata::ORIENTATION_NORMAL:
        case DMetadata::ORIENTATION_UNSPECIFIED:
            break;

        case DMetadata::ORIENTATION_HFLIP:
            matrix.scale(-1, 1);
            break;

        case DMetadata::ORIENTATION_ROT_180:
            matrix.rotate(180);
            break;

        case DMetadata::ORIENTATION_VFLIP:
            matrix.scale(1, -1);
            break;

        case DMetadata::ORIENTATION_ROT_90_HFLIP:
            matrix.scale(-1, 1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90:
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_90_VFLIP:
            matrix.scale(1, -1);
            matrix.rotate(90);
            break;

        case DMetadata::ORIENTATION_ROT_270:
            matrix.rotate(270);
            break;
    }

    // transform accordingly
    thumb = thumb.transformed( matrix );
}


void ThumbnailCreator::deleteThumbnailsFromDisk(const QString &filePath)
{
    QFile smallThumb(thumbnailPath(filePath, normalThumbnailDir()));
    QFile largeThumb(thumbnailPath(filePath, largeThumbnailDir()));

    smallThumb.remove();
    largeThumb.remove();
}


}  // namespace Digikam
