#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
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
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QFont>
#include <QRegExp>
#include "xml_parser.h"
#include "xml_serializer.h"
#include "xml_highlighter.h"
#include "search_dialog.h"
#include "code_folding.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void parseXml();
    void saveFile();
    void exportToJson();
    void exportToYaml();
    void exportToCsv();
    void importFromJson();
    void importFromYaml();
    void toggleEditMode();
    void saveXmlContent();
    void showSearchDialog();
    void performSearch();
    void performReplace();
    void foldAllXml();
    void unfoldAllXml();
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
    QWidget* centerPanel_;
    QTreeWidget* treeWidget_;
    QTextEdit* detailsTextEdit_;
    FoldingTextEdit* xmlEditor_;
    QPushButton* parseButton_;
    QPushButton* openButton_;
    QPushButton* editButton_;
    QPushButton* saveButton_;
    QLabel* fileLabel_;
    
    // Data
    XmlParser parser_;
    XmlSerializer serializer_;
    std::shared_ptr<XmlNode> rootNode_;
    std::string currentFilePath_;
    bool isEditing_;
    QString originalXmlContent_;
    SearchDialog* searchDialog_;
    QList<QTreeWidgetItem*> searchResults_;
    int currentSearchIndex_;
    
    // Actions
    QAction* openAction_;
    QAction* saveAction_;
    QAction* parseAction_;
    QAction* exportJsonAction_;
    QAction* exportYamlAction_;
    QAction* exportCsvAction_;
    QAction* importJsonAction_;
    QAction* importYamlAction_;
    QAction* searchAction_;
    QAction* foldAllAction_;
    QAction* unfoldAllAction_;
    QAction* exitAction_;
    QAction* aboutAction_;
};

#endif // MAIN_WINDOW_H 