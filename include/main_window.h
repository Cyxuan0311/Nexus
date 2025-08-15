#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include "xml_parser.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void parseXml();
    void saveFile();
    void about();
    void onTreeItemClicked(QTreeWidgetItem* item, int column);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupStyle();
    void populateTreeWidget(const std::shared_ptr<XmlNode>& node, QTreeWidgetItem* parentItem = nullptr);
    void displayNodeDetails(const std::shared_ptr<XmlNode>& node);
    void clearDisplay();
    
    // UI Components
    QSplitter* mainSplitter_;
    QWidget* leftPanel_;
    QWidget* rightPanel_;
    QTreeWidget* treeWidget_;
    QTextEdit* detailsTextEdit_;
    QPushButton* parseButton_;
    QPushButton* openButton_;
    QLabel* fileLabel_;
    
    // Data
    XmlParser parser_;
    std::shared_ptr<XmlNode> rootNode_;
    std::string currentFilePath_;
    
    // Actions
    QAction* openAction_;
    QAction* saveAction_;
    QAction* parseAction_;
    QAction* exitAction_;
    QAction* aboutAction_;
};

#endif // MAIN_WINDOW_H 