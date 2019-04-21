/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-02-20
 * Description : Synchronized container for maintenance data.
 *
 * Copyright (C) 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#ifndef DIGIKAM_MAINTENANCE_DATA_H
#define DIGIKAM_MAINTENANCE_DATA_H

// Local includes

#include "iteminfo.h"

namespace Digikam
{

class Identity;
class ImageQualitySettings;

class MaintenanceData
{
public:

    explicit MaintenanceData();
    ~MaintenanceData();

    void      setImageIds(const QList<qlonglong>& ids);
    void      setThumbnailIds(const QList<int>& ids);
    void      setImagePaths(const QList<QString>& paths);
    void      setItemInfos(const QList<ItemInfo>& infos);
    void      setIdentities(const QList<Identity>& identities);
    void      setSimilarityImageIds(const QList<qlonglong>& ids);

    void      setRebuildAllFingerprints(bool b);

    qlonglong getImageId()                 const;
    int       getThumbnailId()             const;
    QString   getImagePath()               const;
    ItemInfo getItemInfo()                 const;
    Identity  getIdentity()                const;
    qlonglong getSimilarityImageId()       const;

    bool      getRebuildAllFingerprints() const;

private:

    MaintenanceData(const MaintenanceData&); // Disable

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_MAINTENANCE_DATA_H
