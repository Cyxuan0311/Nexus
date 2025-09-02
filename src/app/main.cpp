#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QIcon>
#include "main_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Nexus");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Nexus Project");
    
    // Set application icon
    QIcon appIcon("icon/log.svg");
    app.setWindowIcon(appIcon);
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    // Handle command line arguments
    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        if (QFile::exists(filePath)) {
            // TODO: Load the XML file directly
            // window.loadFile(filePath);
        }
    }
    
    return app.exec();
} 