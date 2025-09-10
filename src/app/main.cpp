#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QIcon>
#include <QTimer>
#include <QMessageBox>
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
            // Delay file loading to ensure window is fully initialized
            QTimer::singleShot(100, [&window, filePath]() {
                window.loadFileFromPath(filePath);
            });
        } else {
            QMessageBox::warning(nullptr, "File Not Found", 
                QString("The specified file does not exist: %1").arg(filePath));
        }
    }
    
    return app.exec();
} 