/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-20
 * Description : a widget to display a welcome page
 *               on root album.
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#ifndef DIGIKAM_WELCOME_PAGE_VIEW_H
#define DIGIKAM_WELCOME_PAGE_VIEW_H

#include "digikam_config.h"

// Qt includes

#include <QByteArray>
#include <QString>
#include <QUrl>

#ifdef HAVE_QWEBENGINE
#   include <QWebEngineView>
#   include <QWebEnginePage>
#   include <QWebEngineSettings>
#else
#   include <qwebview.h>
#endif

// Local includes

#include "digikam_export.h"

namespace Digikam
{
#ifdef HAVE_QWEBENGINE
class WelcomePageViewPage : public QWebEnginePage
{
    Q_OBJECT

public:

    explicit WelcomePageViewPage(QObject* const parent = 0);
    virtual ~WelcomePageViewPage();

    bool acceptNavigationRequest(const QUrl&, QWebEnginePage::NavigationType, bool);

Q_SIGNALS:

    void linkClicked(const QUrl&);

};

// -------------------------------------------------------------------

class WelcomePageView : public QWebEngineView
#else
class WelcomePageView : public QWebView
#endif
{
    Q_OBJECT

public:

    explicit WelcomePageView(QWidget* const parent);
    ~WelcomePageView();

private:

    QByteArray  fileToString(const QString& aFileName) const;
    QStringList featuresTabContent() const;
    QStringList aboutTabContent() const;

private Q_SLOTS:

    void slotUrlOpen(const QUrl&);
    void slotThemeChanged();
};

} // namespace Digikam

#endif // DIGIKAM_WELCOME_PAGE_VIEW_H
