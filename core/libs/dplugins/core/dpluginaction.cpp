/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : action container for external plugin
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginaction.h"

// Qt includes

#include <QVariant>
#include <QCollator>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DPluginAction::DPluginAction(QObject* const parent)
    : QAction(parent)
{
}

DPluginAction::~DPluginAction()
{
}

DPluginAction::ActionType DPluginAction::actionType() const
{
    switch (actionCategory())
    {
        case GenericExport:
        case GenericImport:
        case GenericTool:
        case GenericMetadata:
        case GenericView:
            return Generic;

        case EditorFile:
        case EditorColors:
        case EditorEnhance:
        case EditorTransform:
        case EditorDecorate:
        case EditorFilters:
            return Editor;

        default:
            break;
    }

    return InvalidType;
}

void DPluginAction::setActionCategory(ActionCategory cat)
{
    setProperty("DPluginActionCategory", (int)cat);
}

DPluginAction::ActionCategory DPluginAction::actionCategory() const
{
    bool b = false;
    int v  = property("DPluginActionCategory").toInt(&b);

    if (b) return (ActionCategory)v;

    return InvalidCat;
}

QString DPluginAction::actionCategoryToString() const
{
    switch (actionCategory())
    {
        case GenericExport:
            return i18n("Export");
        case GenericImport:
            return i18n("Import");
        case GenericTool:
            return i18n("Tool");
        case GenericMetadata:
            return i18n("Metadata");
        case GenericView:
            return i18n("View");

        case EditorFile:
            return i18n("File");
        case EditorColors:
            return i18n("Color");
        case EditorEnhance:
            return i18n("Enhance");
        case EditorTransform:
            return i18n("Transform");
        case EditorDecorate:
            return i18n("Decorate");
        case EditorFilters:
            return i18n("Effects");

        default:
            break;
    }

    return i18n("Invalid");
}

QString DPluginAction::xmlSection() const
{
    return QString::fromLatin1("<Action name=\"%1\" />\n").arg(objectName());
}

QString DPluginAction::toString() const
{
    return QString::fromUtf8("%1: \"%2\" - %3").arg(objectName())
                                               .arg(text())
                                               .arg(actionCategoryToString());
}

QString DPluginAction::pluginId() const
{
    return property("DPluginId").toString();
}

bool DPluginAction::pluginActionLessThan(DPluginAction* const a, DPluginAction* const b)
{
    QCollator collator;
    collator.setNumericMode(true);
    collator.setIgnorePunctuation(true);
    collator.setCaseSensitivity(Qt::CaseSensitive);

    return (collator.compare(a->text(), b->text()) < 0);
}

} // namespace Digikam
