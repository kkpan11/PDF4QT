//    Copyright (C) 2021-2024 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#include "pdfeditormainwindow.h"
#include "pdfconstants.h"
#include "pdfsecurityhandler.h"
#include "pdfwidgetutils.h"
#include "pdfviewersettings.h"

#include <QSettings>
#include <QApplication>
#include <QCommandLineParser>

#include "pdfdbgheap.h"

int main(int argc, char *argv[])
{
#if defined(PDF4QT_USE_DBG_HEAP)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    QApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents, true);
    QApplication application(argc, argv);

    QCoreApplication::setOrganizationName("MelkaJ");
    QCoreApplication::setApplicationName("PDF4QT Editor");
    QCoreApplication::setApplicationVersion(pdf::PDF_LIBRARY_VERSION);
    QApplication::setApplicationDisplayName(QApplication::translate("Application", "PDF4QT Editor"));

    QCommandLineOption noDrm("no-drm", "Disable DRM settings of documents.");
    QCommandLineOption lightGui("theme-light", "Use a light theme for the GUI.");
    QCommandLineOption darkGui("theme-dark", "Use a dark theme for the GUI.");

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addOption(noDrm);
    parser.addOption(lightGui);
    parser.addOption(darkGui);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The PDF file to open.");
    parser.process(application);

    if (parser.isSet(noDrm))
    {
        pdf::PDFSecurityHandler::setNoDRMMode();
    }

    bool isLightGui = false;
    bool isDarkGui = false;
    const pdfviewer::PDFViewerSettings::ColorScheme colorScheme = pdfviewer::PDFViewerSettings::getColorSchemeStatic();
    switch (colorScheme)
    {
        case pdfviewer::PDFViewerSettings::AutoScheme:
            isLightGui = parser.isSet(lightGui);
            isDarkGui = parser.isSet(darkGui);
            break;

        case pdfviewer::PDFViewerSettings::LightScheme:
            isLightGui = true;
            break;

        case pdfviewer::PDFViewerSettings::DarkScheme:
            isDarkGui = true;
            break;

        default:
            Q_ASSERT(false);
            break;
    }

    pdf::PDFWidgetUtils::setDarkTheme(isLightGui, isDarkGui);

    QIcon appIcon(":/app-icon.svg");
    QApplication::setWindowIcon(appIcon);

    pdfviewer::PDFEditorMainWindow mainWindow;
    mainWindow.show();

    QStringList arguments = parser.positionalArguments();
    if (!arguments.isEmpty())
    {
        mainWindow.getProgramController()->openDocument(arguments.front());
    }

    return application.exec();
}
