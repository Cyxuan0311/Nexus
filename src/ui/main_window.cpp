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
#include <QIcon>
#include <QProgressBar>
#include <fstream>
#include <stdexcept>
#include "markdown_highlighter.h"
#include "cpp_highlighter.h"
#include "python_highlighter.h"
#include "go_highlighter.h"

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
    isMarkdownMode_ = false;
    isCppMode_ = false;
    isPythonMode_ = false;
    isGoMode_ = false;
    currentHighlighter_ = nullptr;
    isDarkTheme_ = true; // 默认使用深色主题
    
    setWindowTitle("Nexus - 多功能代码编辑器与可视化工具");
    
    // Set window icon
    QIcon windowIcon("icon/log.svg");
    setWindowIcon(windowIcon);
    
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
    openButton_ = new QPushButton("Open File");
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
    
    // Parse buttons
    parseButton_ = new QPushButton("Parse XML");
    parseButton_->setEnabled(false);
    leftLayout->addWidget(parseButton_);
    
    cppParseButton_ = new QPushButton("Parse C++");
    cppParseButton_->setEnabled(false);
    leftLayout->addWidget(cppParseButton_);
    
    pythonParseButton_ = new QPushButton("Parse Python");
    pythonParseButton_->setEnabled(false);
    leftLayout->addWidget(pythonParseButton_);
    
    goParseButton_ = new QPushButton("Parse Go");
    goParseButton_->setEnabled(false);
    leftLayout->addWidget(goParseButton_);
    
    graphButton_ = new QPushButton("生成函数关系图");
    graphButton_->setEnabled(false);
    leftLayout->addWidget(graphButton_);
    
    // Right panel (Tabs)
    rightPanel_ = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel_);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    rightTabs_ = new QTabWidget(rightPanel_);
    
    QWidget* detailsTab = new QWidget();
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsTab);
    QLabel* detailsLabel = new QLabel("Node Details");
    detailsLabel->setStyleSheet("color: #4EC9B0; font-weight: bold; font-size: 14px;");
    detailsTextEdit_ = new QTextEdit();
    detailsTextEdit_->setReadOnly(true);
    detailsLayout->addWidget(detailsLabel);
    detailsLayout->addWidget(detailsTextEdit_);
    rightTabs_->addTab(detailsTab, "Details");
    
    QWidget* previewTab = new QWidget();
    QVBoxLayout* previewLayout = new QVBoxLayout(previewTab);
    QLabel* previewLabel = new QLabel("Markdown Preview");
    previewLabel->setStyleSheet("color: #4EC9B0; font-weight: bold; font-size: 14px;");
    markdownPreview_ = new QTextBrowser();
    markdownPreview_->setOpenExternalLinks(true);
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(markdownPreview_);
    rightTabs_->addTab(previewTab, "Preview");
    
    // Function Graph Tab
    functionGraphView_ = new FunctionGraphView();
    rightTabs_->addTab(functionGraphView_, "函数关系图");
    
    rightLayout->addWidget(rightTabs_);
    
    // Center panel for editor
    centerPanel_ = new QWidget();
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel_);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    // Editor controls
    QHBoxLayout* editorControlsLayout = new QHBoxLayout();
    editButton_ = new QPushButton("Edit");
    saveButton_ = new QPushButton("Save Changes");
    saveButton_->setEnabled(false);
    editorControlsLayout->addWidget(editButton_);
    editorControlsLayout->addWidget(saveButton_);
    editorControlsLayout->addStretch();
    
    centerLayout->addLayout(editorControlsLayout);
    
    // Editor
    xmlEditor_ = new FoldingTextEdit();
    xmlEditor_->setReadOnly(true);
    xmlEditor_->setFont(QFont("Consolas", 12));
    xmlEditor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    
    // Default XML highlighter
    currentHighlighter_ = new XmlHighlighter(xmlEditor_->document());
    
    centerLayout->addWidget(xmlEditor_);
    
    // Add panels to splitter
    mainSplitter_->addWidget(leftPanel_);
    mainSplitter_->addWidget(centerPanel_);
    mainSplitter_->addWidget(rightPanel_);
    mainSplitter_->setSizes({400, 600, 400});
    
    // Connect signals
    connect(openButton_, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(parseButton_, &QPushButton::clicked, this, &MainWindow::parseXml);
    connect(cppParseButton_, &QPushButton::clicked, this, &MainWindow::parseCpp);
    connect(pythonParseButton_, &QPushButton::clicked, this, &MainWindow::parsePython);
    connect(goParseButton_, &QPushButton::clicked, this, &MainWindow::parseGo);
    connect(graphButton_, &QPushButton::clicked, this, &MainWindow::generateFunctionGraph);
    connect(editButton_, &QPushButton::clicked, this, &MainWindow::toggleEditMode);
    connect(saveButton_, &QPushButton::clicked, this, &MainWindow::saveXmlContent);
    connect(treeWidget_, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
    connect(xmlEditor_, &QPlainTextEdit::textChanged, this, &MainWindow::renderMarkdownPreview);
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
    
    editMenu->addSeparator();
    
    // 解析快捷键
    parseCppAction_ = editMenu->addAction("Parse &C++");
    parseCppAction_->setShortcut(QKeySequence("Ctrl+Shift+C"));
    connect(parseCppAction_, &QAction::triggered, this, &MainWindow::parseCpp);
    
    parsePythonAction_ = editMenu->addAction("Parse &Python");
    parsePythonAction_->setShortcut(QKeySequence("Ctrl+Shift+P"));
    connect(parsePythonAction_, &QAction::triggered, this, &MainWindow::parsePython);
    
    parseGoAction_ = editMenu->addAction("Parse &Go");
    parseGoAction_->setShortcut(QKeySequence("Ctrl+Shift+G"));
    connect(parseGoAction_, &QAction::triggered, this, &MainWindow::parseGo);
    
    editMenu->addSeparator();
    
    // 生成函数图快捷键
    generateGraphAction_ = editMenu->addAction("Generate &Function Graph");
    generateGraphAction_->setShortcut(QKeySequence("Ctrl+Shift+F"));
    connect(generateGraphAction_, &QAction::triggered, this, &MainWindow::generateFunctionGraph);
    
    // View menu
    QMenu* viewMenu = menuBar->addMenu("&View");
    toggleThemeAction_ = viewMenu->addAction("Toggle &Theme");
    toggleThemeAction_->setShortcut(QKeySequence("Ctrl+T"));
    connect(toggleThemeAction_, &QAction::triggered, this, &MainWindow::toggleTheme);
    
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
    
    // 添加进度条
    progressBar_ = new QProgressBar();
    progressBar_->setVisible(false);
    progressBar_->setMaximumWidth(200);
    statusBar()->addPermanentWidget(progressBar_);
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
    QString filters = "Go Files (*.go);;Python Files (*.py *.pyw);;C++ Files (*.cpp *.cc *.cxx *.h *.hpp *.hxx);;XML Files (*.xml);;Markdown Files (*.md *.markdown);;All Files (*)";
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open File", "", filters);
    
    if (!fileName.isEmpty()) {
        currentFilePath_ = fileName.toStdString();
        fileLabel_->setText(QFileInfo(fileName).fileName());
        parseButton_->setEnabled(true);
        cppParseButton_->setEnabled(true);
        pythonParseButton_->setEnabled(true);
        goParseButton_->setEnabled(true);
        
        // Load content
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            xmlEditor_->setPlainText(content);
            originalXmlContent_ = content;
            isEditing_ = false;
            editButton_->setEnabled(true);
            saveButton_->setEnabled(false);
            xmlEditor_->setReadOnly(true);
        } else {
            QMessageBox::critical(this, "Error", "Failed to open file: " + fileName);
            return;
        }
        
        // Mode + highlighter
        isMarkdownMode_ = isCurrentFileMarkdown();
        isCppMode_ = isCurrentFileCpp();
        isPythonMode_ = isCurrentFilePython();
        isGoMode_ = isCurrentFileGo();
        applyHighlighterForCurrentFile();
        renderMarkdownPreview();
        
        statusBar()->showMessage("File loaded: " + fileName);
    }
}

bool MainWindow::isCurrentFileMarkdown() const {
    QString qpath = QString::fromStdString(currentFilePath_).toLower();
    return qpath.endsWith(".md") || qpath.endsWith(".markdown");
}

bool MainWindow::isCurrentFileCpp() const {
    QString qpath = QString::fromStdString(currentFilePath_).toLower();
    return qpath.endsWith(".cpp") || qpath.endsWith(".cc") || qpath.endsWith(".cxx") ||
           qpath.endsWith(".h") || qpath.endsWith(".hpp") || qpath.endsWith(".hxx");
}

bool MainWindow::isCurrentFilePython() const {
    QString qpath = QString::fromStdString(currentFilePath_).toLower();
    return qpath.endsWith(".py") || qpath.endsWith(".pyw");
}

bool MainWindow::isCurrentFileGo() const {
    QString qpath = QString::fromStdString(currentFilePath_).toLower();
    return qpath.endsWith(".go");
}

void MainWindow::applyHighlighterForCurrentFile() {
    if (currentHighlighter_) {
        delete currentHighlighter_;
        currentHighlighter_ = nullptr;
    }
    if (isMarkdownMode_) {
        currentHighlighter_ = new MarkdownHighlighter(xmlEditor_->document());
        rightTabs_->setTabEnabled(0, false); // Details tab off for Markdown
        rightTabs_->setTabEnabled(2, false); // Function graph tab off for Markdown
        rightTabs_->setCurrentIndex(1);
        if (parseButton_) parseButton_->setEnabled(false);
        if (cppParseButton_) cppParseButton_->setEnabled(false);
        if (pythonParseButton_) pythonParseButton_->setEnabled(false);
        if (goParseButton_) goParseButton_->setEnabled(false);
        if (graphButton_) graphButton_->setEnabled(false);
    } else if (isCppMode_) {
        currentHighlighter_ = new CppHighlighter(xmlEditor_->document());
        rightTabs_->setTabEnabled(0, false); // Details tab off for C++
        rightTabs_->setTabEnabled(1, false); // Preview tab off for C++
        rightTabs_->setTabEnabled(2, true);  // Function graph tab on for C++
        rightTabs_->setCurrentIndex(2);
        if (parseButton_) parseButton_->setEnabled(false);
        if (cppParseButton_) cppParseButton_->setEnabled(true);
        if (pythonParseButton_) pythonParseButton_->setEnabled(false);
        if (goParseButton_) goParseButton_->setEnabled(false);
        if (graphButton_) graphButton_->setEnabled(false);
    } else if (isPythonMode_) {
        currentHighlighter_ = new PythonHighlighter(xmlEditor_->document());
        rightTabs_->setTabEnabled(0, false); // Details tab off for Python
        rightTabs_->setTabEnabled(1, false); // Preview tab off for Python
        rightTabs_->setTabEnabled(2, true);  // Function graph tab on for Python
        rightTabs_->setCurrentIndex(2);
        if (parseButton_) parseButton_->setEnabled(false);
        if (cppParseButton_) cppParseButton_->setEnabled(false);
        if (pythonParseButton_) pythonParseButton_->setEnabled(true);
        if (goParseButton_) goParseButton_->setEnabled(false);
        if (graphButton_) graphButton_->setEnabled(false);
    } else if (isGoMode_) {
        currentHighlighter_ = new GoHighlighter(xmlEditor_->document());
        rightTabs_->setTabEnabled(0, false); // Details tab off for Go
        rightTabs_->setTabEnabled(1, false); // Preview tab off for Go
        rightTabs_->setTabEnabled(2, true);  // Function graph tab on for Go
        rightTabs_->setCurrentIndex(2);
        if (parseButton_) parseButton_->setEnabled(false);
        if (cppParseButton_) cppParseButton_->setEnabled(false);
        if (pythonParseButton_) pythonParseButton_->setEnabled(false);
        if (goParseButton_) goParseButton_->setEnabled(true);
        if (graphButton_) graphButton_->setEnabled(false);
    } else {
        currentHighlighter_ = new XmlHighlighter(xmlEditor_->document());
        rightTabs_->setTabEnabled(0, true);
        rightTabs_->setTabEnabled(1, false);
        rightTabs_->setTabEnabled(2, false);
        rightTabs_->setCurrentIndex(0);
        if (parseButton_) parseButton_->setEnabled(true);
        if (cppParseButton_) cppParseButton_->setEnabled(false);
        if (pythonParseButton_) pythonParseButton_->setEnabled(false);
        if (goParseButton_) goParseButton_->setEnabled(false);
        if (graphButton_) graphButton_->setEnabled(false);
    }
}

void MainWindow::renderMarkdownPreview() {
    if (!isMarkdownMode_ || !markdownPreview_) return;
    // 简化：使用 QTextBrowser 的 setMarkdown（Qt >= 5.14）或手动降级
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    markdownPreview_->setMarkdown(xmlEditor_->toPlainText());
#else
    // Fallback: 基于简单替换（粗体/斜体/标题/代码）转为 HTML
    QString md = xmlEditor_->toPlainText();
    QString html = md;
    html.replace(QRegExp("^######\\s+(.*)$"), "<h6>\\1</h6>");
    html.replace(QRegExp("^#####\\s+(.*)$"), "<h5>\\1</h5>");
    html.replace(QRegExp("^####\\s+(.*)$"), "<h4>\\1</h4>");
    html.replace(QRegExp("^###\\s+(.*)$"), "<h3>\\1</h3>");
    html.replace(QRegExp("^##\\s+(.*)$"), "<h2>\\1</h2>");
    html.replace(QRegExp("^#\\s+(.*)$"), "<h1>\\1</h1>");
    html.replace(QRegExp("\\*\\*([^*]+)\\*\\*"), "<b>\\1</b>");
    html.replace(QRegExp("_([^_]+)_"), "<i>\\1</i>");
    html.replace(QRegExp("`([^`]+)`"), "<code>\\1</code>");
    markdownPreview_->setHtml(html);
#endif
}

void MainWindow::parseXml() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "Please select a file first.");
        return;
    }
    
    if (isMarkdownMode_) {
        QMessageBox::information(this, "Info", "Markdown files do not support XML parsing.");
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
    if (isMarkdownMode_) {
        QString fileName = QFileDialog::getSaveFileName(this,
            "Save Markdown File", "", "Markdown Files (*.md *.markdown);;All Files (*)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << xmlEditor_->toPlainText();
                file.close();
                statusBar()->showMessage("File saved: " + fileName);
            } else {
                QMessageBox::critical(this, "Error", "Failed to save file.");
            }
        }
        return;
    }
    
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
        "<p>A Qt-based XML and Markdown editor with preview and XML visualizer.</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>Real-time XML structure visualization</li>"
        "<li>Markdown editing with syntax highlighting and preview</li>"
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
            // 实现正则表达式替换
            QRegExp regex(searchText);
            if (regex.isValid()) {
                Qt::CaseSensitivity caseSensitivity = searchDialog_->isCaseSensitive() ? 
                    Qt::CaseSensitive : Qt::CaseInsensitive;
                regex.setCaseSensitivity(caseSensitivity);
                
                int count = 0;
                content.replace(regex, replaceText, &count);
                if (count > 0) {
                    xmlEditor_->setPlainText(content);
                    statusBar()->showMessage(QString("Replaced %1 occurrences using regex").arg(count));
                } else {
                    statusBar()->showMessage("No matches found for regex pattern");
                }
            } else {
                QMessageBox::warning(this, "Invalid Regex", "The regular expression pattern is invalid.");
            }
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
        statusBar()->showMessage(isMarkdownMode_ ? "Edit mode enabled - you can now modify the Markdown" : "Edit mode enabled - you can now modify the XML");
    } else {
        // Cancel edit mode
        isEditing_ = false;
        xmlEditor_->setPlainText(originalXmlContent_);
        xmlEditor_->setReadOnly(true);
        editButton_->setText("Edit");
        saveButton_->setEnabled(false);
        statusBar()->showMessage("Edit cancelled - changes discarded");
    }
}

void MainWindow::saveXmlContent() {
    if (!isEditing_) {
        return;
    }
    
    QString newContent = xmlEditor_->toPlainText();

    if (isMarkdownMode_) {
        // Save Markdown without XML validation
        QFile file(QString::fromStdString(currentFilePath_));
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << newContent;
            file.close();
            
            originalXmlContent_ = newContent;
            isEditing_ = false;
            xmlEditor_->setReadOnly(true);
            editButton_->setText("Edit");
            saveButton_->setEnabled(false);
            statusBar()->showMessage("Markdown content saved successfully");
        } else {
            QMessageBox::critical(this, "Error", "Failed to save Markdown content");
        }
        return;
    }
    
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
        editButton_->setText("Edit");
        saveButton_->setEnabled(false);
        
        statusBar()->showMessage("XML content saved successfully");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save XML content");
    }
}

void MainWindow::parseCpp() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "请先打开一个C++文件");
        return;
    }
    
    // 显示进度条
    progressBar_->setVisible(true);
    progressBar_->setRange(0, 0); // 不确定进度
    statusBar()->showMessage("正在解析C++文件...");
    QApplication::processEvents(); // 更新UI
    
    QString content = xmlEditor_->toPlainText();
    std::string cppContent = content.toStdString();
    
    // 检查文件大小，大文件给出提示
    if (cppContent.length() > 100000) { // 100KB
        progressBar_->setRange(0, 100);
        progressBar_->setValue(50);
        statusBar()->showMessage("正在解析大型C++文件，请稍候...");
        QApplication::processEvents();
    }
    
    if (cppParser_.parseFile(cppContent)) {
        progressBar_->setValue(100);
        statusBar()->showMessage("C++ 文件解析成功");
        graphButton_->setEnabled(true);
        
        // 显示解析统计信息
        const auto& functions = cppParser_.getFunctions();
        const auto& classes = cppParser_.getClasses();
        QString info = QString("解析完成：发现 %1 个函数，%2 个类")
                      .arg(functions.size())
                      .arg(classes.size());
        QMessageBox::information(this, "解析结果", info);
    } else {
        QMessageBox::critical(this, "Error", "C++ 文件解析失败");
        graphButton_->setEnabled(false);
    }
    
    // 隐藏进度条
    progressBar_->setVisible(false);
}

void MainWindow::parsePython() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "请先打开一个Python文件");
        return;
    }
    
    QString content = xmlEditor_->toPlainText();
    std::string pythonContent = content.toStdString();
    
    if (pythonParser_.parseFile(pythonContent)) {
        statusBar()->showMessage("Python 文件解析成功");
        graphButton_->setEnabled(true);
        
        // 显示解析统计信息
        const auto& functions = pythonParser_.getFunctions();
        const auto& classes = pythonParser_.getClasses();
        QString info = QString("解析完成：发现 %1 个函数，%2 个类")
                      .arg(functions.size())
                      .arg(classes.size());
        QMessageBox::information(this, "解析结果", info);
    } else {
        QMessageBox::critical(this, "Error", "Python 文件解析失败");
        graphButton_->setEnabled(false);
    }
}

void MainWindow::generateFunctionGraph() {
    bool hasFunctions = false;
    
    if (isCppMode_ && !cppParser_.getFunctions().empty()) {
        // 设置C++解析数据到图形视图
        functionGraphView_->setParserData(cppParser_);
        hasFunctions = true;
    } else if (isPythonMode_ && !pythonParser_.getFunctions().empty()) {
        // 需要创建一个转换函数将Python数据转换为C++数据格式
        // 或者修改FunctionGraphView以支持多种解析器类型
        // 为了简化，我们暂时使用适配器模式
        CppParser adaptedParser;
        adaptPythonToCppParser(adaptedParser);
        functionGraphView_->setParserData(adaptedParser);
        hasFunctions = true;
    } else if (isGoMode_ && !goParser_.getFunctions().empty()) {
        // 将Go解析数据转换为C++数据格式
        CppParser adaptedParser;
        adaptGoToCppParser(adaptedParser);
        functionGraphView_->setParserData(adaptedParser);
        hasFunctions = true;
    }
    
    if (!hasFunctions) {
        QMessageBox::warning(this, "Warning", "请先解析代码文件");
        return;
    }
    
    // 生成函数关系图
    functionGraphView_->generateGraph();
    
    // 切换到函数关系图标签页
    rightTabs_->setCurrentIndex(2);
    
    statusBar()->showMessage("函数关系图生成完成");
}

void MainWindow::parseGo() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "请先打开一个Go文件");
        return;
    }
    
    QString content = xmlEditor_->toPlainText();
    std::string goContent = content.toStdString();
    
    if (goParser_.parseFile(goContent)) {
        statusBar()->showMessage("Go 文件解析成功");
        graphButton_->setEnabled(true);
        
        // 显示解析统计信息
        const auto& functions = goParser_.getFunctions();
        const auto& structs = goParser_.getStructs();
        const auto& interfaces = goParser_.getInterfaces();
        QString info = QString("解析完成：发现 %1 个函数，%2 个结构体，%3 个接口")
                      .arg(functions.size())
                      .arg(structs.size())
                      .arg(interfaces.size());
        QMessageBox::information(this, "解析结果", info);
    } else {
        QMessageBox::critical(this, "Error", "Go 文件解析失败");
        graphButton_->setEnabled(false);
    }
}

void MainWindow::adaptPythonToCppParser(CppParser& cppParser) {
    // 清空目标解析器
    cppParser.clear();
    
    // 将Python函数转换为C++函数格式
    const auto& pythonFunctions = pythonParser_.getFunctions();
    const auto& pythonClasses = pythonParser_.getClasses();
    const auto& pythonCalls = pythonParser_.getFunctionCalls();
    
    // 这是一个简化的适配器实现
    // 在实际应用中，您可能需要修改FunctionGraphView以支持泛型解析器接口
    
    // 由于CppParser的内部数据是私有的，我们需要通过解析来填充数据
    // 这里我们构造一个临时的C++代码字符串来模拟转换
    std::string adaptedCppCode;
    
    // 转换类定义
    for (const auto& pythonClass : pythonClasses) {
        adaptedCppCode += "class " + pythonClass.name;
        if (!pythonClass.baseClasses.empty()) {
            adaptedCppCode += " : public " + pythonClass.baseClasses[0];
        }
        adaptedCppCode += " {\npublic:\n";
        
        // 转换类的方法
        for (const auto& method : pythonClass.methods) {
            adaptedCppCode += "    void " + method.name + "(";
            for (size_t i = 0; i < method.parameters.size(); ++i) {
                if (i > 0) adaptedCppCode += ", ";
                adaptedCppCode += "int " + method.parameters[i].name;
            }
            adaptedCppCode += ") {\n";
            
            // 添加函数调用
            auto it = pythonCalls.find(method.name);
            if (it != pythonCalls.end()) {
                for (const std::string& calledFunc : it->second) {
                    adaptedCppCode += "        " + calledFunc + "();\n";
                }
            }
            
            adaptedCppCode += "    }\n";
        }
        adaptedCppCode += "};\n\n";
    }
    
    // 转换独立函数
    for (const auto& pythonFunc : pythonFunctions) {
        if (pythonFunc.className.empty()) { // 只处理独立函数
            adaptedCppCode += "void " + pythonFunc.name + "(";
            for (size_t i = 0; i < pythonFunc.parameters.size(); ++i) {
                if (i > 0) adaptedCppCode += ", ";
                adaptedCppCode += "int " + pythonFunc.parameters[i].name;
            }
            adaptedCppCode += ") {\n";
            
            // 添加函数调用
            auto it = pythonCalls.find(pythonFunc.name);
            if (it != pythonCalls.end()) {
                for (const std::string& calledFunc : it->second) {
                    adaptedCppCode += "    " + calledFunc + "();\n";
                }
            }
            
            adaptedCppCode += "}\n\n";
        }
    }
    
    // 解析适配后的代码
    cppParser.parseFile(adaptedCppCode);
}

void MainWindow::adaptGoToCppParser(CppParser& cppParser) {
    // 清空目标解析器
    cppParser.clear();
    
    // 将Go函数转换为C++函数格式
    const auto& goFunctions = goParser_.getFunctions();
    const auto& goStructs = goParser_.getStructs();
    const auto& goCalls = goParser_.getFunctionCalls();
    
    // 构造一个临时的C++代码字符串来模拟转换
    std::string adaptedCppCode;
    
    // 转换结构体定义
    for (const auto& goStruct : goStructs) {
        adaptedCppCode += "class " + goStruct.name + " {\npublic:\n";
        
        // 转换结构体字段
        for (const auto& field : goStruct.fields) {
            adaptedCppCode += "    int " + field + ";\n";
        }
        
        adaptedCppCode += "};\n\n";
    }
    
    // 转换函数定义
    for (const auto& goFunc : goFunctions) {
        // 处理返回类型（Go可以有多个返回值）
        std::string returnType = "void";
        if (!goFunc.returnTypes.empty()) {
            returnType = goFunc.returnTypes[0]; // 简化处理，只取第一个返回类型
            // 将Go类型映射到C++类型
            if (returnType == "string") returnType = "std::string";
            else if (returnType == "int") returnType = "int";
            else if (returnType == "bool") returnType = "bool";
            else if (returnType == "float64") returnType = "double";
            else returnType = "int"; // 默认类型
        }
        
        // 处理方法（有接收者）
        if (goFunc.isMethod) {
            adaptedCppCode += "class " + goFunc.receiverType + " {\npublic:\n";
            adaptedCppCode += "    " + returnType + " " + goFunc.name + "(";
        } else {
            adaptedCppCode += returnType + " " + goFunc.name + "(";
        }
        
        // 转换参数
        for (size_t i = 0; i < goFunc.parameters.size(); ++i) {
            if (i > 0) adaptedCppCode += ", ";
            
            std::string paramType = goFunc.parameters[i].type;
            // 简单的类型映射
            if (paramType == "string") paramType = "std::string";
            else if (paramType == "int") paramType = "int";
            else if (paramType == "bool") paramType = "bool";
            else if (paramType == "float64") paramType = "double";
            else paramType = "int"; // 默认类型
            
            adaptedCppCode += paramType + " " + goFunc.parameters[i].name;
        }
        
        adaptedCppCode += ") {\n";
        
        // 添加函数调用
        auto it = goCalls.find(goFunc.name);
        if (it != goCalls.end()) {
            for (const std::string& calledFunc : it->second) {
                adaptedCppCode += "    " + calledFunc + "();\n";
            }
        }
        
        if (goFunc.isMethod) {
            adaptedCppCode += "    }\n};\n\n";
        } else {
            adaptedCppCode += "}\n\n";
        }
    }
    
    // 解析适配后的代码
    cppParser.parseFile(adaptedCppCode);
}

void MainWindow::loadFileFromPath(const QString& filePath) {
    // 设置当前文件路径
    currentFilePath_ = filePath.toStdString();
    
    // 更新文件标签
    QFileInfo fileInfo(filePath);
    fileLabel_->setText(fileInfo.fileName());
    
    // 读取文件内容
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", 
            QString("Cannot open file: %1").arg(filePath));
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // 设置编辑器内容
    xmlEditor_->setPlainText(content);
    
    // 根据文件扩展名设置模式
    QString extension = fileInfo.suffix().toLower();
    isMarkdownMode_ = (extension == "md" || extension == "markdown");
    isCppMode_ = (extension == "cpp" || extension == "cxx" || extension == "cc" || extension == "c");
    isPythonMode_ = (extension == "py");
    isGoMode_ = (extension == "go");
    
    // 应用语法高亮
    applyHighlighterForCurrentFile();
    
    // 启用相应的解析按钮
    if (isCppMode_) {
        cppParseButton_->setEnabled(true);
    } else if (isPythonMode_) {
        pythonParseButton_->setEnabled(true);
    } else if (isGoMode_) {
        goParseButton_->setEnabled(true);
    } else if (!isMarkdownMode_) {
        parseButton_->setEnabled(true);
    }
    
    // 如果是Markdown文件，渲染预览
    if (isMarkdownMode_) {
        renderMarkdownPreview();
    }
    
    statusBar()->showMessage(QString("Loaded: %1").arg(filePath));
}

void MainWindow::toggleTheme() {
    isDarkTheme_ = !isDarkTheme_;
    
    if (isDarkTheme_) {
        // 应用深色主题
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
        statusBar()->showMessage("已切换到深色主题");
    } else {
        // 应用浅色主题
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
        lightPalette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
        lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
        lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
        lightPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        lightPalette.setColor(QPalette::Link, QColor(0, 102, 204));
        lightPalette.setColor(QPalette::Highlight, QColor(0, 102, 204));
        lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        QApplication::setPalette(lightPalette);
        statusBar()->showMessage("已切换到浅色主题");
    }
    
    // 重新应用语法高亮以适配新主题
    applyHighlighterForCurrentFile();
} 