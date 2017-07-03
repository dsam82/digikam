/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-30
 * Description : Similarity DB schema update
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SIMILARITYDBCHEMAUPDATER_H
#define SIMILARITYDBCHEMAUPDATER_H

namespace Digikam
{

class SimilarityDbAccess;
class InitializationObserver;

class SimilarityDbSchemaUpdater
{
public:

    static int schemaVersion();

public:

    explicit SimilarityDbSchemaUpdater(SimilarityDbAccess* access);

    bool update();
    void setObserver(InitializationObserver* observer);

private:

    bool startUpdates();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    

private:

    bool                     m_setError;

    int                      m_currentVersion;
    int                      m_currentRequiredVersion;

    SimilarityDbAccess* m_access;

    InitializationObserver*  m_observer;
};

}  // namespace Digikam

#endif // SIMILARITYDBCHEMAUPDATER_H
