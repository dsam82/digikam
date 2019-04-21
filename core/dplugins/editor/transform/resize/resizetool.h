/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 *
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_EDITOR_RESIZE_TOOL_H
#define DIGIKAM_EDITOR_RESIZE_TOOL_H

// Qt includes

#include <QString>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorResizeToolPlugin
{

class ResizeTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ResizeTool(QObject* const parent);
    ~ResizeTool();

private:

    void writeSettings();
    void readSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();
    void renderingFinished();
    void blockWidgetSignals(bool b);

    double pixelsToUnits(int pix);
    int    unitsToPixels(double val);

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotValuesChanged();
    void slotUnitsChanged();
    void slotPresetsChanged();
    void slotRestorationToggled(bool);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamEditorResizeToolPlugin

#endif // DIGIKAM_EDITOR_RESIZE_TOOL_H
