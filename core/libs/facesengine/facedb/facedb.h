/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_FACE_DB_TRAINING_INTERFACE_H
#define DIGIKAM_FACE_DB_TRAINING_INTERFACE_H

// C++ includes

#include <vector>

// Qt includes

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>

// Local includes

#include "digikam_config.h"
#include "identity.h"
#include "facedbbackend.h"
#include "opencvmatdata.h"

namespace Digikam
{

class LBPHFaceModel;
class EigenFaceModel;
class FisherFaceModel;

#ifdef HAVE_FACESENGINE_DNN
class DNNFaceModel;
#endif

class FaceDb
{
public:

    FaceDb();
    explicit FaceDb(FaceDbBackend* const db);
    ~FaceDb();

    BdEngineBackend::QueryState setSetting(const QString& keyword, const QString& value);
    QString setting(const QString& keyword) const;

    int  addIdentity() const;
    void updateIdentity(const Identity& p);
    void deleteIdentity(int id);
    void deleteIdentity(const QString& uuid);
    QList<Identity> identities()  const;
    QList<int>      identityIds() const;

    /// OpenCV LBPH

    void updateLBPHFaceModel(LBPHFaceModel& model);
    LBPHFaceModel lbphFaceModel() const;
    void clearLBPHTraining(const QString& context = QString());
    void clearLBPHTraining(const QList<int>& identities, const QString& context = QString());

    /// OpenCV EIGEN

    void updateEIGENFaceModel(EigenFaceModel& model, const std::vector<cv::Mat>& images_rgb);
    EigenFaceModel eigenFaceModel() const;
    void clearEIGENTraining(const QString& context = QString());
    void clearEIGENTraining(const QList<int>& identities, const QString& context = QString());

    /// OpenCV FISHER
    FisherFaceModel fisherFaceModel() const;

#ifdef HAVE_FACESENGINE_DNN
    /// DNN
    void updateDNNFaceModel(DNNFaceModel& model);
    void getFaceVector(cv::Mat data, std::vector<float>& vecdata);
    DNNFaceModel dnnFaceModel() const;
#endif

    // ----------- Database shrinking methods ----------

    /**
     * Returns true if the integrity of the database is preserved.
     */
    bool integrityCheck();

    /**
     * Shrinks the database.
     */
    void vacuum();

private:

    FaceDb(const FaceDb&); // Disable

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FACE_DB_TRAINING_INTERFACE_H
