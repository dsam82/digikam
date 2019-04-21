/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-18
 * Description : database worker interface
 *
 * Copyright (C) 2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "fileworkeriface.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "metaenginesettings.h"
#include "fileactionmngr_p.h"
#include "itemattributeswatch.h"
#include "iteminfotasksplitter.h"
#include "scancontroller.h"
#include "digikam_globals.h"
#include "jpegutils.h"
#include "dimg.h"
#include "facetagseditor.h"

namespace Digikam
{

void FileActionMngrFileWorker::writeOrientationToFiles(FileActionItemInfoList infos, int orientation)
{
    QStringList failedItems;

    foreach (const ItemInfo& info, infos)
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        QString path                  = info.filePath();
        DMetadata metadata(path);
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setItemOrientation(o);

        if (!metadata.applyChanges())
        {
            failedItems.append(info.name());
        }
        else
        {
            emit imageDataChanged(path, true, true);
            QUrl url = QUrl::fromLocalFile(path);
            ItemAttributesWatch::instance()->fileMetadataChanged(url);
        }

        infos.writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to revise Exif orientation these files:"), failedItems);
    }

    infos.finishedWriting();
}

void FileActionMngrFileWorker::writeMetadataToFiles(FileActionItemInfoList infos)
{
    d->startingToWrite(infos);

    ScanController::instance()->suspendCollectionScan();

    foreach (const ItemInfo& info, infos)
    {
        MetadataHub hub;

        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        hub.load(info);
        QString filePath = info.filePath();

        if (MetaEngineSettings::instance()->settings().useLazySync)
        {
            hub.write(filePath, MetadataHub::WRITE_ALL);
        }
        else
        {
            ScanController::FileMetadataWrite writeScope(info);
            writeScope.changed(hub.write(filePath, MetadataHub::WRITE_ALL));
        }

        // hub emits fileMetadataChanged
        infos.writtenToOne();
    }

    ScanController::instance()->resumeCollectionScan();

    infos.finishedWriting();
}

void FileActionMngrFileWorker::writeMetadata(FileActionItemInfoList infos, int flags)
{
    d->startingToWrite(infos);

    ScanController::instance()->suspendCollectionScan();

    foreach (const ItemInfo& info, infos)
    {
        MetadataHub hub;

        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        hub.load(info);
        // apply to file metadata
        if (MetaEngineSettings::instance()->settings().useLazySync)
        {
            hub.writeToMetadata(info, (MetadataHub::WriteComponents)flags);
        }
        else
        {
            ScanController::FileMetadataWrite writeScope(info);
            writeScope.changed(hub.writeToMetadata(info, (MetadataHub::WriteComponents)flags));
        }

        // hub emits fileMetadataChanged
        infos.writtenToOne();
    }

    ScanController::instance()->resumeCollectionScan();

    infos.finishedWriting();
}

void FileActionMngrFileWorker::transform(FileActionItemInfoList infos, int action)
{
    d->startingToWrite(infos);

    QStringList failedItems;
    ScanController::instance()->suspendCollectionScan();

    foreach (const ItemInfo& info, infos)
    {
        if (state() == WorkerObject::Deactivating)
        {
            break;
        }

        QString path                                    = info.filePath();
        QString format                                  = info.format();
        MetaEngine::ImageOrientation currentOrientation = (MetaEngine::ImageOrientation)info.orientation();
        bool isRaw                                      = info.format().startsWith(QLatin1String("RAW"));
        bool rotateAsJpeg                               = false;
        bool rotateLossy                                = false;

        MetaEngineSettingsContainer::RotationBehaviorFlags behavior;
        behavior              = MetaEngineSettings::instance()->settings().rotationBehavior;
        bool rotateByMetadata = (behavior & MetaEngineSettingsContainer::RotateByMetadataFlag);

        // Check if rotation by content, as desired, is feasible
        // We'll later check again if it was successful
        if (behavior & MetaEngineSettingsContainer::RotatingPixels)
        {
            if (format == QLatin1String("JPG") && JPEGUtils::isJpegImage(path))
            {
                rotateAsJpeg = true;
            }

            if (behavior & MetaEngineSettingsContainer::RotateByLossyRotation)
            {
                DImg::FORMAT format = DImg::fileFormat(path);

                switch (format)
                {
                    case DImg::JPEG:
                    case DImg::PNG:
                    case DImg::TIFF:
                    case DImg::JP2K:
                    case DImg::PGF:
                        rotateLossy = true;
                    default:
                        break;
                }
            }
        }

        MetaEngineRotation matrix;
        matrix                                        *= currentOrientation;
        matrix                                        *= (MetaEngineRotation::TransformationAction)action;
        MetaEngine::ImageOrientation finalOrientation  = matrix.exifOrientation();
        bool rotatedPixels                             = false;

        if (rotateAsJpeg)
        {
            JPEGUtils::JpegRotator rotator(path);
            rotator.setCurrentOrientation(currentOrientation);

            if (action == MetaEngineRotation::NoTransformation)
            {
                rotatedPixels = rotator.autoExifTransform();
            }
            else
            {
                rotatedPixels = rotator.exifTransform((MetaEngineRotation::TransformationAction)action);
            }

            if (!rotatedPixels)
            {
                failedItems.append(info.name());
            }
        }
        else if (rotateLossy)
        {
            // Non-JPEG image: DImg
            DImg image;

            if (!image.load(path))
            {
                failedItems.append(info.name());
            }
            else
            {
                if (action == MetaEngineRotation::NoTransformation)
                {
                    image.rotateAndFlip(currentOrientation);
                }
                else
                {
                    image.transform(action);
                }

                // TODO: Atomic operation!!
                // prepare metadata, including to reset Exif tag
                image.prepareMetadataToSave(path, image.format(), true);

                if (image.save(path, image.detectedFormat()))
                {
                    rotatedPixels = true;
                }
                else
                {
                    failedItems.append(info.name());
                }
            }
        }

        adjustFaceRectangles(info, rotatedPixels,
                                   finalOrientation,
                                   currentOrientation);

        if (rotatedPixels)
        {
            // reset for DB. Metadata is already edited.
            finalOrientation = MetaEngine::ORIENTATION_NORMAL;
        }

        if (rotateByMetadata)
        {
            // Setting the rotation flag on Raws with embedded JPEG is a mess
            // Can apply to the RAW data, or to the embedded JPEG, or to both.
            if (!isRaw)
            {
                DMetadata metadata(path);
                metadata.setItemOrientation(finalOrientation);
                metadata.applyChanges();
            }
        }

        // DB rotation
        ItemInfo(info).setOrientation(finalOrientation);

        if (!failedItems.contains(info.name()))
        {
            emit imageDataChanged(path, true, true);
            ItemAttributesWatch::instance()->fileMetadataChanged(info.fileUrl());
        }

        infos.writtenToOne();
    }

    if (!failedItems.isEmpty())
    {
        emit imageChangeFailed(i18n("Failed to transform these files:"), failedItems);
    }

    infos.finishedWriting();

    ScanController::instance()->resumeCollectionScan();
}

void FileActionMngrFileWorker::adjustFaceRectangles(const ItemInfo& info, bool rotatedPixels,
                                                                          int newOrientation,
                                                                          int oldOrientation)
{
    /**
     *  Get all faces from database and rotate them
     */
    QList<FaceTagsIface> facesList = FaceTagsEditor().databaseFaces(info.id());

    if (facesList.isEmpty())
    {
        return;
    }

    QSize newSize = info.dimensions();
    QMultiMap<QString, QRect> adjustedFaces;

    foreach (const FaceTagsIface& dface, facesList)
    {
        QRect faceRect = dface.region().toRect();
        QString name   = FaceTags::faceNameForTag(dface.tagId());

        TagRegion::reverseToOrientation(faceRect,
                                        oldOrientation,
                                        info.dimensions());

        newSize = TagRegion::adjustToOrientation(faceRect,
                                                 newOrientation,
                                                 info.dimensions());

        if (dface.tagId() == FaceTags::unknownPersonTagId())
        {
            name.clear();
        }

        adjustedFaces.insertMulti(name, faceRect);
    }

    /**
     *  Delete all old faces and add rotated ones
     */
    FaceTagsEditor().removeAllFaces(info.id());

    QMultiMap<QString, QRect>::ConstIterator it = adjustedFaces.constBegin();

    for ( ; it != adjustedFaces.constEnd() ; ++it)
    {
        TagRegion region(it.value());

        if (it.key().isEmpty())
        {
            int tagId = FaceTags::unknownPersonTagId();
            FaceTagsIface face(FaceTagsIface::UnknownName, info.id(), tagId, region);

            FaceTagsEditor().addManually(face);
        }
        else
        {
            int tagId = FaceTags::getOrCreateTagForPerson(it.key());

            if (!tagId)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to create a person tag for name" << it.key();
            }

            FaceTagsEditor().add(info.id(), tagId, region, false);
        }
    }

    if (!rotatedPixels)
    {
        newSize = info.dimensions();
    }

    /**
     * Write medatada
     */
    MetadataHub hub;
    hub.load(info);
    // Adjusted newSize
    hub.loadFaceTags(info, newSize);
    hub.write(info.filePath(), MetadataHub::WRITE_ALL);
}

} // namespace Digikam
