/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-08
 * Description : global macros, variables and flags
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikam_globals.h"

// Qt includes

#include <QObject>
#include <QList>
#include <QImageReader>
#include <QImageWriter>
#include <QByteArray>
#include <QShortcut>
#include <QApplication>
#include <QStandardPaths>
#include <QLibrary>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "drawdecoder.h"
#include "rawcameradlg.h"

// Windows includes

#ifdef HAVE_DRMINGW
#   include <windows.h>
#endif

namespace Digikam
{

QShortcut* defineShortcut(QWidget* const w, const QKeySequence& key, const QObject* receiver, const char* slot)
{
    QShortcut* const s = new QShortcut(w);
    s->setKey(key);
    s->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(s, SIGNAL(activated()), receiver, slot);

    return s;
}

QStringList supportedImageMimeTypes(QIODevice::OpenModeFlag mode, QString& allTypes)
{
    QStringList       formats;
    QList<QByteArray> supported;

    switch(mode)
    {
        case QIODevice::ReadOnly:
            supported = QImageReader::supportedImageFormats();
            break;
        case QIODevice::WriteOnly:
            supported = QImageWriter::supportedImageFormats();
            break;
        case QIODevice::ReadWrite:
            supported = QImageWriter::supportedImageFormats() + QImageReader::supportedImageFormats();
            break;
        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unsupported mode!";
            break;
    }

    bool tiff = false;
    bool jpeg = false;
#ifdef HAVE_JASPER
    bool jp2k = false;
#endif // HAVE_JASPER

    foreach(const QByteArray& frm, supported)
    {
        if (QString::fromLatin1(frm).contains(QLatin1String("tif"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("tiff"), Qt::CaseInsensitive))
        {
            tiff = true;
            continue;
        }

        if (QString::fromLatin1(frm).contains(QLatin1String("jpg"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpeg"), Qt::CaseInsensitive))
        {
            jpeg = true;
            continue;
        }

#ifdef HAVE_JASPER
        if (QString::fromLatin1(frm).contains(QLatin1String("jp2"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("j2k"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpx"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("jpc"),  Qt::CaseInsensitive) ||
            QString::fromLatin1(frm).contains(QLatin1String("pgx"),  Qt::CaseInsensitive))
        {
            jp2k = true;
            continue;
        }
#endif // HAVE_JASPER

        formats.append(i18n("%1 Image (%2)", QString::fromLatin1(frm).toUpper(), QLatin1String("*.") + QLatin1String(frm)));
        allTypes.append(QString::fromLatin1("*.%1 ").arg(QLatin1String(frm)));
    }

    if (tiff)
    {
        formats.append(i18n("TIFF Image (*.tiff *.tif)"));
        allTypes.append(QLatin1String("*.tiff *.tif "));
    }

    if (jpeg)
    {
        formats.append(i18n("JPEG Image (*.jpg *.jpeg *.jpe)"));
        allTypes.append(QLatin1String("*.jpg *.jpeg *.jpe "));
    }

#ifdef HAVE_JASPER
    if (jp2k)
    {
        formats.append(i18n("JPEG2000 Image (*.jp2 *.j2k *.jpx *.pgx)"));
        allTypes.append(QLatin1String("*.jp2 *.j2k *.jpx *.pgx "));
    }
#endif // HAVE_JASPER

    formats << i18n("Progressive Graphics file (*.pgf)");
    allTypes.append(QLatin1String("*.pgf "));

    if (mode != QIODevice::WriteOnly)
    {
        formats << i18n("Raw Images (%1)", QLatin1String(DRawDecoder::rawFiles()));
        allTypes.append(QLatin1String(DRawDecoder::rawFiles()));
        formats << i18n("All supported files (%1)", allTypes);
    }

    return formats;
}

void showRawCameraList()
{
    RawCameraDlg* const dlg = new RawCameraDlg(qApp->activeWindow());
    dlg->show();
}

QProcessEnvironment adjustedEnvironmentForAppImage()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // If we are running into AppImage bundle, switch env var to the right values.
    if (env.contains(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")) &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH"))  &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS"))   &&
        env.contains(QLatin1String("APPIMAGE_ORIGINAL_PATH")))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Adjusting environment variables for AppImage bundle";

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")).isEmpty())
        {
            env.insert(QLatin1String("LD_LIBRARY_PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_LD_LIBRARY_PATH")));
        }
        else
        {
            env.remove(QLatin1String("LD_LIBRARY_PATH"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH")).isEmpty())
        {
            env.insert(QLatin1String("QT_PLUGIN_PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_QT_PLUGIN_PATH")));
        }
        else
        {
            env.remove(QLatin1String("QT_PLUGIN_PATH"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS")).isEmpty())
        {
            env.insert(QLatin1String("XDG_DATA_DIRS"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_XDG_DATA_DIRS")));
        }
        else
        {
            env.remove(QLatin1String("XDG_DATA_DIRS"));
        }

        if (!env.value(QLatin1String("APPIMAGE_ORIGINAL_PATH")).isEmpty())
        {
            env.insert(QLatin1String("PATH"),
                       env.value(QLatin1String("APPIMAGE_ORIGINAL_PATH")));
        }
        else
        {
            env.remove(QLatin1String("PATH"));
        }
    }

    return env;
}


void tryInitDrMingw()
{
#ifdef HAVE_DRMINGW

    qCDebug(DIGIKAM_GENERAL_LOG) << "Loading DrMinGw run-time...";

    wchar_t path[MAX_PATH];
    QString pathStr = QCoreApplication::applicationDirPath().replace(L'/', L'\\') + QLatin1String("\\exchndl.dll");

    if (pathStr.size() > MAX_PATH - 1)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DrMinGw: cannot find crash handler dll.";
        return;
    }

    int pathLen   = pathStr.toWCharArray(path);
    path[pathLen] = L'\0'; // toWCharArray doesn't add NULL terminator
    HMODULE hMod  = LoadLibraryW(path);

    if (!hMod)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DrMinGw: cannot init crash handler dll.";
        return;
    }

    // No need to call ExcHndlInit since the crash handler is installed on DllMain
    auto myExcHndlSetLogFileNameA = reinterpret_cast<BOOL (APIENTRY*)(const char*)>(GetProcAddress(hMod, "ExcHndlSetLogFileNameA"));

    if (!myExcHndlSetLogFileNameA)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DrMinGw: cannot init customized crash file.";
        return;
    }

    // Set the log file path to %LocalAppData%\kritacrash.log
    QString logFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).replace(L'/', L'\\') + QLatin1String("\\digikam_crash.log");
    myExcHndlSetLogFileNameA(logFile.toLocal8Bit().data());

    qCDebug(DIGIKAM_GENERAL_LOG) << "DrMinGw run-time loaded.";
    qCDebug(DIGIKAM_GENERAL_LOG) << "DrMinGw crash-file will be located at: " << logFile;

#endif // HAVE_DRMINGW
}

QString toolButtonStyleSheet()
{
    return QLatin1String("QToolButton { padding: 1px; background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(100, 100, 100, 50%), "
                         "  stop: 1 rgba(170, 170, 170, 50%)); "
                         "border: 1px solid rgba(170, 170, 170, 10%); } "

                         "QToolButton:hover { border-color: white; } "

                         "QToolButton:pressed { background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(40, 40, 40, 50%), "
                         "  stop: 1 rgba(90, 90, 90, 50%)); "
                         "border-color: white; } "

                         "QToolButton:checked { background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(40, 40, 40, 50%), "
                         "  stop: 1 rgba(90, 90, 90, 50%)); } "

                         "QToolButton:disabled { background-color: "
                         "  qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
                         "  stop: 0 rgba(40, 40, 40, 50%), "
                         "  stop: 1 rgba(50, 50, 50, 50%)); }");
}

} // namespace Digikam
