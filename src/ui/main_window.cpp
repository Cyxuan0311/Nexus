#include "main_window.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QTextStream>
#include <QFont>
#include <QPalette>
#include <QColor>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QKeySequence>
#include <QVariant>
#include <QStyle>
#include <QToolButton>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QTextCursor>
#include <QMessageBox>
#include <fstream>
#include <stdexcept>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupStyle();
    
    // Initialize data
    isEditing_ = false;
    originalXmlContent_ = "";
    searchDialog_ = nullptr;
    currentSearchIndex_ = -1;
    
    setWindowTitle("Cxml - XML Visualizer");
    resize(1400, 900);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    // Create main splitter
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter_);
    
    // Left panel
    leftPanel_ = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel_);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // File controls
    QHBoxLayout* fileLayout = new QHBoxLayout();
    openButton_ = new QPushButton("Open XML File");
    fileLabel_ = new QLabel("No file selected");
    fileLabel_->setStyleSheet("color: #4EC9B0; font-weight: bold;");
    fileLayout->addWidget(openButton_);
    fileLayout->addWidget(fileLabel_);
    fileLayout->addStretch();
    
    leftLayout->addLayout(fileLayout);
    
    // Tree widget
    treeWidget_ = new QTreeWidget();
    treeWidget_->setHeaderLabel("XML Structure");
    treeWidget_->setAlternatingRowColors(true);
    leftLayout->addWidget(treeWidget_);
    
    // Parse button
    parseButton_ = new QPushButton("Parse XML");
    parseButton_->setEnabled(false);
    leftLayout->addWidget(parseButton_);
    
    // Right panel
    rightPanel_ = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel_);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* detailsLabel = new QLabel("Node Details");
    detailsLabel->setStyleSheet("color: #4EC9B0; font-weight: bold; font-size: 14px;");
    rightLayout->addWidget(detailsLabel);
    
    detailsTextEdit_ = new QTextEdit();
    detailsTextEdit_->setReadOnly(true);
    rightLayout->addWidget(detailsTextEdit_);
    
    // Center panel for XML editor
    centerPanel_ = new QWidget();
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel_);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    // XML Editor controls
    QHBoxLayout* editorControlsLayout = new QHBoxLayout();
    editButton_ = new QPushButton("Edit XML");
    saveButton_ = new QPushButton("Save Changes");
    saveButton_->setEnabled(false);
    editorControlsLayout->addWidget(editButton_);
    editorControlsLayout->addWidget(saveButton_);
    editorControlsLayout->addStretch();
    
    centerLayout->addLayout(editorControlsLayout);
    
    // XML Editor
    xmlEditor_ = new QPlainTextEdit();
    xmlEditor_->setReadOnly(true);
    xmlEditor_->setFont(QFont("Consolas", 12));
    xmlEditor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    
    // Apply syntax highlighting
    new XmlHighlighter(xmlEditor_->document());
    
    centerLayout->addWidget(xmlEditor_);
    
    // Add panels to splitter
    mainSplitter_->addWidget(leftPanel_);
    mainSplitter_->addWidget(centerPanel_);
    mainSplitter_->addWidget(rightPanel_);
    mainSplitter_->setSizes({400, 600, 400});
    
    // Connect signals
    connect(openButton_, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(parseButton_, &QPushButton::clicked, this, &MainWindow::parseXml);
    connect(editButton_, &QPushButton::clicked, this, &MainWindow::toggleEditMode);
    connect(saveButton_, &QPushButton::clicked, this, &MainWindow::saveXmlContent);
    connect(treeWidget_, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    openAction_ = fileMenu->addAction("&Open...");
    openAction_->setShortcut(QKeySequence::Open);
    connect(openAction_, &QAction::triggered, this, &MainWindow::openFile);
    
    saveAction_ = fileMenu->addAction("&Save As...");
    saveAction_->setShortcut(QKeySequence::Save);
    connect(saveAction_, &QAction::triggered, this, &MainWindow::saveFile);
    
    fileMenu->addSeparator();
    
    // Export submenu
    QMenu* exportMenu = fileMenu->addMenu("&Export");
    exportJsonAction_ = exportMenu->addAction("To &JSON...");
    connect(exportJsonAction_, &QAction::triggered, this, &MainWindow::exportToJson);
    
    exportYamlAction_ = exportMenu->addAction("To &YAML...");
    connect(exportYamlAction_, &QAction::triggered, this, &MainWindow::exportToYaml);
    
    exportCsvAction_ = exportMenu->addAction("To &CSV...");
    connect(exportCsvAction_, &QAction::triggered, this, &MainWindow::exportToCsv);
    
    // Import submenu
    QMenu* importMenu = fileMenu->addMenu("&Import");
    importJsonAction_ = importMenu->addAction("From &JSON...");
    connect(importJsonAction_, &QAction::triggered, this, &MainWindow::importFromJson);
    
    importYamlAction_ = importMenu->addAction("From &YAML...");
    connect(importYamlAction_, &QAction::triggered, this, &MainWindow::importFromYaml);
    
    fileMenu->addSeparator();
    
    exitAction_ = fileMenu->addAction("E&xit");
    exitAction_->setShortcut(QKeySequence::Quit);
    connect(exitAction_, &QAction::triggered, this, &QApplication::quit);
    
    // Edit menu
    QMenu* editMenu = menuBar->addMenu("&Edit");
    searchAction_ = editMenu->addAction("&Find/Replace...");
    searchAction_->setShortcut(QKeySequence::Find);
    connect(searchAction_, &QAction::triggered, this, &MainWindow::showSearchDialog);
    
    editMenu->addSeparator();
    foldAllAction_ = editMenu->addAction("Fold &All");
    foldAllAction_->setShortcut(QKeySequence("Ctrl+Shift+["));
    connect(foldAllAction_, &QAction::triggered, this, &MainWindow::foldAllXml);
    
    unfoldAllAction_ = editMenu->addAction("&Unfold All");
    unfoldAllAction_->setShortcut(QKeySequence("Ctrl+Shift+]"));
    connect(unfoldAllAction_, &QAction::triggered, this, &MainWindow::unfoldAllXml);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    aboutAction_ = helpMenu->addAction("&About");
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::setupToolBar() {
    QToolBar* toolBar = addToolBar("Main Toolbar");
    toolBar->addAction(openAction_);
    toolBar->addAction(saveAction_);
    toolBar->addSeparator();
    toolBar->addWidget(parseButton_);
    toolBar->addSeparator();
    
    // Add export actions to toolbar
    QMenu* exportMenu = new QMenu();
    exportMenu->addAction(exportJsonAction_);
    exportMenu->addAction(exportYamlAction_);
    exportMenu->addAction(exportCsvAction_);
    
    QToolButton* exportButton = new QToolButton();
    exportButton->setText("Export");
    exportButton->setMenu(exportMenu);
    exportButton->setPopupMode(QToolButton::InstantPopup);
    toolBar->addWidget(exportButton);
    toolBar->addSeparator();
    toolBar->addAction(searchAction_);
    toolBar->addSeparator();
    toolBar->addAction(foldAllAction_);
    toolBar->addAction(unfoldAllAction_);
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Ready");
}

void MainWindow::setupStyle() {
    // Set application style
    QApplication::setStyle("Fusion");
    
    // Create dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    QApplication::setPalette(darkPalette);
    
    // Set stylesheet for custom styling
    QString styleSheet = R"(
        QMainWindow {
            background-color: #1E1E1E;
        }
        
        QTreeWidget {
            background-color: #252526;
            border: 1px solid #3E3E42;
            color: #CCCCCC;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
        }
        
        QTreeWidget::item {
            padding: 4px;
            border: none;
        }
        
        QTreeWidget::item:selected {
            background-color: #094771;
        }
        
        QTreeWidget::item:hover {
            background-color: #2A2D2E;
        }
        
        QTreeWidget::item:alternate {
            background-color: #2D2D30;
        }
        
        QTextEdit {
            background-color: #1E1E1E;
            border: 1px solid #3E3E42;
            color: #CCCCCC;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
        }
        
        QPlainTextEdit {
            background-color: #1E1E1E;
            border: 1px solid #3E3E42;
            color: #CCCCCC;
            font-family: 'Consolas', 'Monaco', monospace;
            font-size: 12px;
            selection-background-color: #094771;
        }
        
        QPushButton {
            background-color: #0E639C;
            border: 1px solid #007ACC;
            color: white;
            padding: 8px 16px;
            font-weight: bold;
            border-radius: 4px;
        }
        
        QPushButton:hover {
            background-color: #1177BB;
        }
        
        QPushButton:pressed {
            background-color: #005A9E;
        }
        
        QPushButton:disabled {
            background-color: #3E3E42;
            border: 1px solid #5A5A5A;
            color: #6A6A6A;
        }
        
        QLabel {
            color: #CCCCCC;
        }
        
        QMenuBar {
            background-color: #3C3C3C;
            color: #CCCCCC;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 4px 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #094771;
        }
        
        QMenu {
            background-color: #3C3C3C;
            border: 1px solid #5A5A5A;
            color: #CCCCCC;
        }
        
        QMenu::item {
            padding: 6px 20px;
        }
        
        QMenu::item:selected {
            background-color: #094771;
        }
        
        QToolBar {
            background-color: #3C3C3C;
            border: none;
            spacing: 4px;
        }
        
        QStatusBar {
            background-color: #007ACC;
            color: white;
        }
        
        QSplitter::handle {
            background-color: #3E3E42;
        }
        
        QSplitter::handle:hover {
            background-color: #4EC9B0;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open XML File", "", "XML Files (*.xml);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        currentFilePath_ = fileName.toStdString();
        fileLabel_->setText(QFileInfo(fileName).fileName());
        parseButton_->setEnabled(true);
        
        // Load and display XML content in editor
        loadXmlContent(fileName);
        
        statusBar()->showMessage("File loaded: " + fileName);
    }
}

void MainWindow::loadXmlContent(const QString& fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        xmlEditor_->setPlainText(content);
        originalXmlContent_ = content;
        isEditing_ = false;
        
        // Update button states
        editButton_->setEnabled(true);
        saveButton_->setEnabled(false);
        xmlEditor_->setReadOnly(true);
    } else {
        QMessageBox::critical(this, "Error", "Failed to open file: " + fileName);
    }
}

void MainWindow::parseXml() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "Please select an XML file first.");
        return;
    }
    
    clearDisplay();
    
    rootNode_ = parser_.parseFile(currentFilePath_);
    
    if (parser_.hasError()) {
        QMessageBox::critical(this, "Error", 
            QString("Failed to parse XML: %1").arg(QString::fromStdString(parser_.getErrorMessage())));
        return;
    }
    
    if (rootNode_) {
        populateTreeWidget(rootNode_);
        statusBar()->showMessage("XML parsed successfully");
    } else {
        QMessageBox::warning(this, "Warning", "No valid XML content found.");
    }
}

void MainWindow::saveFile() {
    if (!rootNode_) {
        QMessageBox::warning(this, "Warning", "No XML data to save.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save XML File", "", "XML Files (*.xml);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        try {
            std::string xmlContent = serializer_.serializeToXml(rootNode_);
            std::ofstream file(fileName.toStdString());
            if (file.is_open()) {
                file << xmlContent;
                file.close();
                statusBar()->showMessage("File saved: " + fileName);
            } else {
                QMessageBox::critical(this, "Error", "Failed to save file.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to save file: %1").arg(e.what()));
        }
    }
}

void MainWindow::about() {
    QMessageBox::about(this, "About Cxml",
        "<h3>Cxml - XML Visualizer</h3>"
        "<p>A Qt-based XML file parser and visualizer with a VSCode-like interface.</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>Real-time XML structure visualization</li>"
        "<li>Hierarchical tree view</li>"
        "<li>Node details display</li>"
        "<li>Dark theme with green accents</li>"
        "</ul>"
        "<p>Built with Qt5, C++17, and CMake</p>");
}

void MainWindow::onTreeItemClicked(QTreeWidgetItem* item, int column) {
    if (!item) return;
    
    // Get the node data from the item
    QVariant nodeData = item->data(0, Qt::UserRole);
    if (nodeData.isValid()) {
        std::shared_ptr<XmlNode> node = nodeData.value<std::shared_ptr<XmlNode>>();
        if (node) {
            displayNodeDetails(node);
        }
    }
}

void MainWindow::populateTreeWidget(const std::shared_ptr<XmlNode>& node, QTreeWidgetItem* parentItem) {
    if (!node) return;
    
    QTreeWidgetItem* item;
    if (parentItem) {
        item = new QTreeWidgetItem(parentItem);
    } else {
        item = new QTreeWidgetItem(treeWidget_);
    }
    
    // Set item text based on node type
    QString displayText;
    switch (node->getType()) {
        case XmlNode::NodeType::Element:
            displayText = QString::fromStdString(node->getName());
            if (!node->getAttributes().empty()) {
                displayText += " [";
                bool first = true;
                for (const auto& attr : node->getAttributes()) {
                    if (!first) displayText += ", ";
                    displayText += QString::fromStdString(attr.first);
                    first = false;
                }
                displayText += "]";
            }
            break;
        case XmlNode::NodeType::Text:
            displayText = QString::fromStdString(node->getValue());
            if (displayText.length() > 50) {
                displayText = displayText.left(50) + "...";
            }
            displayText = "\"" + displayText + "\"";
            break;
        case XmlNode::NodeType::Comment:
            displayText = "<!-- " + QString::fromStdString(node->getValue()) + " -->";
            break;
        default:
            displayText = QString::fromStdString(node->getName());
            break;
    }
    
    item->setText(0, displayText);
    item->setData(0, Qt::UserRole, QVariant::fromValue(node));
    
    // Set icon based on node type
    if (node->getType() == XmlNode::NodeType::Element) {
        if (node->isLeaf()) {
            item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        } else {
            item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        }
    } else if (node->getType() == XmlNode::NodeType::Text) {
        item->setIcon(0, style()->standardIcon(QStyle::SP_MessageBoxInformation));
    }
    
    // Recursively add children
    for (const auto& child : node->getChildren()) {
        populateTreeWidget(child, item);
    }
    
    // Expand the item if it has children
    if (!node->isLeaf()) {
        item->setExpanded(true);
    }
}

void MainWindow::displayNodeDetails(const std::shared_ptr<XmlNode>& node) {
    if (!node) return;
    
    QString details;
    QTextStream stream(&details);
    
    stream << "<h3>Node Details</h3>";
    stream << "<table border='1' cellpadding='5' cellspacing='0' style='border-collapse: collapse;'>";
    
    stream << "<tr><td><b>Type:</b></td><td>";
    switch (node->getType()) {
        case XmlNode::NodeType::Element:
            stream << "Element";
            break;
        case XmlNode::NodeType::Text:
            stream << "Text";
            break;
        case XmlNode::NodeType::Comment:
            stream << "Comment";
            break;
        case XmlNode::NodeType::ProcessingInstruction:
            stream << "Processing Instruction";
            break;
        case XmlNode::NodeType::Document:
            stream << "Document";
            break;
    }
    stream << "</td></tr>";
    
    if (!node->getName().empty()) {
        stream << "<tr><td><b>Name:</b></td><td>" << QString::fromStdString(node->getName()) << "</td></tr>";
    }
    
    if (!node->getValue().empty()) {
        stream << "<tr><td><b>Value:</b></td><td><pre>" << QString::fromStdString(node->getValue()) << "</pre></td></tr>";
    }
    
    stream << "<tr><td><b>Depth:</b></td><td>" << node->getDepth() << "</td></tr>";
    stream << "<tr><td><b>Path:</b></td><td>" << QString::fromStdString(node->getPath()) << "</td></tr>";
    stream << "<tr><td><b>Children:</b></td><td>" << node->getChildren().size() << "</td></tr>";
    
    if (!node->getAttributes().empty()) {
        stream << "<tr><td><b>Attributes:</b></td><td><table border='1' cellpadding='3' cellspacing='0'>";
        stream << "<tr><th>Name</th><th>Value</th></tr>";
        for (const auto& attr : node->getAttributes()) {
            stream << "<tr><td>" << QString::fromStdString(attr.first) << "</td><td>" 
                   << QString::fromStdString(attr.second) << "</td></tr>";
        }
        stream << "</table></td></tr>";
    }
    
    stream << "</table>";
    
    detailsTextEdit_->setHtml(details);
}

void MainWindow::clearDisplay() {
    treeWidget_->clear();
    detailsTextEdit_->clear();
    rootNode_.reset();
}

void MainWindow::exportToJson() {
    if (!rootNode_) {
        QMessageBox::warning(this, "Warning", "No XML data to export.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export to JSON", "", "JSON Files (*.json);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        try {
            std::string jsonContent = serializer_.serializeToJson(rootNode_);
            std::ofstream file(fileName.toStdString());
            if (file.is_open()) {
                file << jsonContent;
                file.close();
                statusBar()->showMessage("Exported to JSON: " + fileName);
            } else {
                QMessageBox::critical(this, "Error", "Failed to save JSON file.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to export to JSON: %1").arg(e.what()));
        }
    }
}

void MainWindow::exportToYaml() {
    if (!rootNode_) {
        QMessageBox::warning(this, "Warning", "No XML data to export.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export to YAML", "", "YAML Files (*.yaml *.yml);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        try {
            std::string yamlContent = serializer_.serializeToYaml(rootNode_);
            std::ofstream file(fileName.toStdString());
            if (file.is_open()) {
                file << yamlContent;
                file.close();
                statusBar()->showMessage("Exported to YAML: " + fileName);
            } else {
                QMessageBox::critical(this, "Error", "Failed to save YAML file.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to export to YAML: %1").arg(e.what()));
        }
    }
}

void MainWindow::exportToCsv() {
    if (!rootNode_) {
        QMessageBox::warning(this, "Warning", "No XML data to export.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export to CSV", "", "CSV Files (*.csv);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        try {
            std::string csvContent = serializer_.serializeToCsv(rootNode_);
            std::ofstream file(fileName.toStdString());
            if (file.is_open()) {
                file << csvContent;
                file.close();
                statusBar()->showMessage("Exported to CSV: " + fileName);
            } else {
                QMessageBox::critical(this, "Error", "Failed to save CSV file.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to export to CSV: %1").arg(e.what()));
        }
    }
}

void MainWindow::importFromJson() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Import from JSON", "", "JSON Files (*.json);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        try {
            std::ifstream file(fileName.toStdString());
            if (file.is_open()) {
                std::string jsonContent((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                file.close();
                
                auto importedNode = serializer_.deserializeFromJson(jsonContent);
                if (importedNode) {
                    clearDisplay();
                    rootNode_ = importedNode;
                    populateTreeWidget(rootNode_);
                    currentFilePath_ = fileName.toStdString();
                    fileLabel_->setText(QFileInfo(fileName).fileName());
                    statusBar()->showMessage("Imported from JSON: " + fileName);
                } else {
                    QMessageBox::warning(this, "Warning", "Failed to parse JSON file.");
                }
            } else {
                QMessageBox::critical(this, "Error", "Failed to open JSON file.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to import from JSON: %1").arg(e.what()));
        }
    }
}

void MainWindow::importFromYaml() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Import from YAML", "", "YAML Files (*.yaml *.yml);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        try {
            std::ifstream file(fileName.toStdString());
            if (file.is_open()) {
                std::string yamlContent((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                file.close();
                
                auto importedNode = serializer_.deserializeFromYaml(yamlContent);
                if (importedNode) {
                    clearDisplay();
                    rootNode_ = importedNode;
                    populateTreeWidget(rootNode_);
                    currentFilePath_ = fileName.toStdString();
                    fileLabel_->setText(QFileInfo(fileName).fileName());
                    statusBar()->showMessage("Imported from YAML: " + fileName);
                } else {
                    QMessageBox::warning(this, "Warning", "Failed to parse YAML file.");
                }
            } else {
                QMessageBox::critical(this, "Error", "Failed to open YAML file.");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", 
                QString("Failed to import from YAML: %1").arg(e.what()));
        }
    }
}

void MainWindow::showSearchDialog() {
    if (!searchDialog_) {
        searchDialog_ = new SearchDialog(this);
    }
    
    if (searchDialog_->exec() == QDialog::Accepted) {
        if (searchDialog_->isReplaceMode()) {
            performReplace();
        } else {
            performSearch();
        }
    }
}

void MainWindow::performSearch() {
    QString searchText = searchDialog_->getSearchText();
    if (searchText.isEmpty()) return;
    
    searchResults_.clear();
    currentSearchIndex_ = -1;
    
    // Search in tree view
    if (searchDialog_->searchInTree()) {
        searchInTreeWidget(searchText);
    }
    
    // Search in editor
    if (searchDialog_->searchInEditor()) {
        searchInEditor(searchText);
    }
    
    // Show results
    if (!searchResults_.isEmpty()) {
        statusBar()->showMessage(QString("Found %1 results").arg(searchResults_.size()));
        highlightNextResult();
    } else {
        statusBar()->showMessage("No results found");
    }
}

void MainWindow::performReplace() {
    QString searchText = searchDialog_->getSearchText();
    QString replaceText = searchDialog_->getReplaceText();
    
    if (searchText.isEmpty()) return;
    
    // Replace in editor
    if (searchDialog_->searchInEditor()) {
        QTextCursor cursor = xmlEditor_->textCursor();
        QString content = xmlEditor_->toPlainText();
        
        if (searchDialog_->isRegex()) {
            // TODO: Implement regex replace
            QMessageBox::information(this, "Info", "Regex replace not implemented yet");
        } else {
            Qt::CaseSensitivity caseSensitivity = searchDialog_->isCaseSensitive() ? 
                Qt::CaseSensitive : Qt::CaseInsensitive;
            
            int count = content.count(searchText, caseSensitivity);
            if (count > 0) {
                content.replace(searchText, replaceText, caseSensitivity);
                xmlEditor_->setPlainText(content);
                statusBar()->showMessage(QString("Replaced %1 occurrences").arg(count));
            }
        }
    }
}

void MainWindow::searchInTreeWidget(const QString& searchText) {
    searchInTreeWidgetRecursive(treeWidget_->invisibleRootItem(), searchText);
}

void MainWindow::searchInTreeWidgetRecursive(QTreeWidgetItem* item, const QString& searchText) {
    if (!item) return;
    
    // Check current item
    QString itemText = item->text(0);
    Qt::CaseSensitivity caseSensitivity = searchDialog_->isCaseSensitive() ? 
        Qt::CaseSensitive : Qt::CaseInsensitive;
    
    if (itemText.contains(searchText, caseSensitivity)) {
        searchResults_.append(item);
    }
    
    // Check children
    for (int i = 0; i < item->childCount(); ++i) {
        searchInTreeWidgetRecursive(item->child(i), searchText);
    }
}

void MainWindow::searchInEditor(const QString& searchText) {
    QTextCursor cursor = xmlEditor_->textCursor();
    QString content = xmlEditor_->toPlainText();
    
    Qt::CaseSensitivity caseSensitivity = searchDialog_->isCaseSensitive() ? 
        Qt::CaseSensitive : Qt::CaseInsensitive;
    
    int pos = content.indexOf(searchText, cursor.position(), caseSensitivity);
    if (pos >= 0) {
        cursor.setPosition(pos);
        cursor.setPosition(pos + searchText.length(), QTextCursor::KeepAnchor);
        xmlEditor_->setTextCursor(cursor);
        xmlEditor_->ensureCursorVisible();
    }
}

void MainWindow::highlightNextResult() {
    if (searchResults_.isEmpty()) return;
    
    currentSearchIndex_ = (currentSearchIndex_ + 1) % searchResults_.size();
    QTreeWidgetItem* item = searchResults_[currentSearchIndex_];
    
    if (item) {
        treeWidget_->setCurrentItem(item);
        treeWidget_->scrollToItem(item);
        item->setSelected(true);
    }
}

void MainWindow::foldAllXml() {
    if (xmlEditor_) {
        xmlEditor_->foldAll();
        statusBar()->showMessage("All XML blocks folded");
    }
}

void MainWindow::unfoldAllXml() {
    if (xmlEditor_) {
        xmlEditor_->unfoldAll();
        statusBar()->showMessage("All XML blocks unfolded");
    }
}

void MainWindow::toggleEditMode() {
    if (!isEditing_) {
        // Enter edit mode
        isEditing_ = true;
        xmlEditor_->setReadOnly(false);
        editButton_->setText("Cancel Edit");
        saveButton_->setEnabled(true);
        statusBar()->showMessage("Edit mode enabled - you can now modify the XML");
    } else {
        // Cancel edit mode
        isEditing_ = false;
        xmlEditor_->setPlainText(originalXmlContent_);
        xmlEditor_->setReadOnly(true);
        editButton_->setText("Edit XML");
        saveButton_->setEnabled(false);
        statusBar()->showMessage("Edit cancelled - changes discarded");
    }
}

void MainWindow::saveXmlContent() {
    if (!isEditing_) {
        return;
    }
    
    QString newContent = xmlEditor_->toPlainText();
    
    // Validate XML before saving
    try {
        auto testNode = parser_.parseString(newContent.toStdString());
        if (!testNode) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning", 
                "The XML content appears to be invalid. Save anyway?",
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;
            }
        }
    } catch (...) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning", 
            "The XML content appears to be invalid. Save anyway?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    
    // Save to file
    QFile file(QString::fromStdString(currentFilePath_));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << newContent;
        file.close();
        
        // Update original content
        originalXmlContent_ = newContent;
        isEditing_ = false;
        xmlEditor_->setReadOnly(true);
        editButton_->setText("Edit XML");
        saveButton_->setEnabled(false);
        
        statusBar()->showMessage("XML content saved successfully");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save XML content");
    }
} 