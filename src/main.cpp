#include <QApplication>
#include <QWebEngineProfile>
#include "MainWindow.h"
#include "ThemeManager.h"

int main(int argc, char *argv[]) {
    // Se tiver problemas com GPU, descomente a linha abaixo:
    // QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("MiniBrowser");
    QCoreApplication::setOrganizationName("MiniBrowser");

    // Set default profile
    QWebEngineProfile::defaultProfile()->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

    MainWindow window;
    window.show();

    return app.exec();
}