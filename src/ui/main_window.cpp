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
#include <QtGlobal>
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
#include <QScrollBar>
#include <QPainter>
#include <QTextBlock>
#include <fstream>
#include <stdexcept>
#include "markdown_highlighter.h"
#include "cpp_highlighter.h"
#include "python_highlighter.h"
#include "go_highlighter.h"

// Enhanced FoldingTextEdit with line numbers
class EnhancedFoldingTextEdit : public FoldingTextEdit {
    Q_OBJECT

public:
    explicit EnhancedFoldingTextEdit(QWidget* parent = nullptr) : FoldingTextEdit(parent) {
        lineNumberArea_ = new LineNumberArea(this);
        
        connect(this->document(), &QTextDocument::blockCountChanged, this, &EnhancedFoldingTextEdit::updateLineNumberAreaWidth);
        connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) { updateLineNumberArea(); });
        connect(this, &QPlainTextEdit::textChanged, this, [this]() { updateLineNumberArea(); });
        connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this]() { updateLineNumberArea(); });
        
        updateLineNumberAreaWidth(0);
    }

    void lineNumberAreaPaintEvent(QPaintEvent* event) {
        QPainter painter(lineNumberArea_);
        painter.fillRect(event->rect(), QColor(30, 30, 30));

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(QColor(106, 106, 106));
                painter.setFont(QFont("Cascadia Code", 13));
                painter.drawText(0, top, lineNumberArea_->width(), fontMetrics().height(),
                               Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }

    int lineNumberAreaWidth() {
        int digits = 1;
        int max = qMax(1, blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }

        int space = 13 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
        return space;
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QPlainTextEdit::resizeEvent(event);
        QRect cr = contentsRect();
        lineNumberArea_->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }

private slots:
    void updateLineNumberAreaWidth(int newBlockCount) {
        Q_UNUSED(newBlockCount)
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }

    void updateLineNumberArea() {
        lineNumberArea_->update();
    }

private:
    class LineNumberArea : public QWidget {
    public:
        LineNumberArea(EnhancedFoldingTextEdit* editor) : QWidget(editor), codeEditor_(editor) {}

        QSize sizeHint() const override {
            return QSize(codeEditor_->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent* event) override {
            codeEditor_->lineNumberAreaPaintEvent(event);
        }

    private:
        EnhancedFoldingTextEdit* codeEditor_;
    };

    LineNumberArea* lineNumberArea_;
};

#include "main_window.moc"

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
    isDarkTheme_ = true; // Default to dark theme
    
    setWindowTitle("Nexus - Multi-Purpose Code Editor & Visualizer");
    
    // Set window icon
    QIcon windowIcon("icon/log.svg");
    setWindowIcon(windowIcon);
    
    resize(1400, 900);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    // Create main splitter with improved flexibility
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    mainSplitter_->setChildrenCollapsible(false);  // 防止面板完全折叠
    mainSplitter_->setHandleWidth(8);  // 增加分割条宽度，便于拖拽
    mainSplitter_->setStyleSheet(
        "QSplitter::handle {"
        "    background-color: #2D2D30;"
        "    border: none;"
        "    margin: 1px;"
        "    width: 1px;"
        "}"
        "QSplitter::handle:hover {"
        "    background-color: #007ACC;"
        "    width: 3px;"
        "}"
        "QSplitter::handle:pressed {"
        "    background-color: #005A9E;"
        "    width: 3px;"
        "}"
    );
    setCentralWidget(mainSplitter_);
    
    // Left panel - File browser and controls
    leftPanel_ = new QWidget();
    leftPanel_->setObjectName("leftPanel");
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel_);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(8);
    
    // 简化的顶部控制区域 - 参考VSCode设计
    QHBoxLayout* topInfoLayout = new QHBoxLayout();
    topInfoLayout->setContentsMargins(0, 0, 0, 0);
    topInfoLayout->setSpacing(8);
    
    // 文件状态指示器 - 更简约
    QLabel* statusIcon = new QLabel();
    statusIcon->setPixmap(style()->standardIcon(QStyle::SP_FileIcon).pixmap(14, 14));
    statusIcon->setToolTip("File Status");
    statusIcon->setStyleSheet("QLabel { margin: 2px; }");
    
    topInfoLayout->addWidget(statusIcon);
    topInfoLayout->addStretch();
    
    leftLayout->addLayout(topInfoLayout);
    
    // File info - 更简约的设计
    fileLabel_ = new QLabel("No file selected");
    fileLabel_->setStyleSheet("color: #CCCCCC; font-weight: 500; font-size: 13px; padding: 6px; background-color: #2D2D30; border-radius: 4px;");
    fileLabel_->setWordWrap(true);
    fileLabel_->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(fileLabel_);
    
    // Tree widget for file structure/analysis results
    treeWidget_ = new QTreeWidget();
    treeWidget_->setHeaderLabel("File Structure");
    treeWidget_->setAlternatingRowColors(true);
    treeWidget_->setRootIsDecorated(true);
    leftLayout->addWidget(treeWidget_);
    
    // Right panel - Code editor
    centerPanel_ = new QWidget();
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel_);
    centerLayout->setContentsMargins(8, 8, 8, 8);
    centerLayout->setSpacing(8);
    
    // Editor with line numbers - 参考VSCode设计
    xmlEditor_ = new EnhancedFoldingTextEdit();
    xmlEditor_->setReadOnly(true);
    xmlEditor_->setFont(QFont("Cascadia Code", 14));
    xmlEditor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    xmlEditor_->setStyleSheet("QPlainTextEdit { border: none; background-color: #1E1E1E; }");
    
    // Set proper margins and tab settings
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    xmlEditor_->setTabStopDistance(4 * xmlEditor_->fontMetrics().horizontalAdvance(' '));
#else
    xmlEditor_->setTabStopWidth(4 * xmlEditor_->fontMetrics().horizontalAdvance(' '));
#endif
    
    // Default XML highlighter
    currentHighlighter_ = new XmlHighlighter(xmlEditor_->document());
    
    centerLayout->addWidget(xmlEditor_);
    
    // Add panels to splitter - only two panels now
    mainSplitter_->addWidget(leftPanel_);
    mainSplitter_->addWidget(centerPanel_);
    
    // Set initial sizes with better proportions
    mainSplitter_->setSizes({300, 700});
    mainSplitter_->setStretchFactor(0, 0);  // 左侧面板不拉伸
    mainSplitter_->setStretchFactor(1, 1);  // 右侧面板可拉伸
    
    // Initialize hidden panels for analysis results
    rightPanel_ = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel_);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    rightTabs_ = new QTabWidget(rightPanel_);
    
    QWidget* detailsTab = new QWidget();
    QVBoxLayout* detailsLayout = new QVBoxLayout(detailsTab);
    detailsLayout->setContentsMargins(8, 8, 8, 8);
    QLabel* detailsLabel = new QLabel("Analysis Results");
    detailsLabel->setStyleSheet("color: #CCCCCC; font-weight: 600; font-size: 13px; padding: 4px;");
    detailsTextEdit_ = new QTextEdit();
    detailsTextEdit_->setReadOnly(true);
    detailsTextEdit_->setStyleSheet("QTextEdit { background-color: #1E1E1E; border: none; color: #D4D4D4; font-family: 'Cascadia Code', monospace; }");
    detailsLayout->addWidget(detailsLabel);
    detailsLayout->addWidget(detailsTextEdit_);
    rightTabs_->addTab(detailsTab, "Details");
    
    QWidget* previewTab = new QWidget();
    QVBoxLayout* previewLayout = new QVBoxLayout(previewTab);
    previewLayout->setContentsMargins(8, 8, 8, 8);
    QLabel* previewLabel = new QLabel("Preview");
    previewLabel->setStyleSheet("color: #CCCCCC; font-weight: 600; font-size: 13px; padding: 4px;");
    markdownPreview_ = new QTextBrowser();
    markdownPreview_->setOpenExternalLinks(true);
    markdownPreview_->setStyleSheet("QTextBrowser { background-color: #1E1E1E; border: none; color: #D4D4D4; }");
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(markdownPreview_);
    rightTabs_->addTab(previewTab, "Preview");
    
    // Function Graph Tab
    functionGraphView_ = new FunctionGraphView();
    rightTabs_->addTab(functionGraphView_, "Function Graph");
    
    rightLayout->addWidget(rightTabs_);
    
    // Connect signals
    connect(treeWidget_, &QTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
    connect(xmlEditor_, &QPlainTextEdit::textChanged, this, &MainWindow::renderMarkdownPreview);
    connect(xmlEditor_, &QPlainTextEdit::textChanged, this, &MainWindow::updateLineCount);
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
    
    // Parse shortcuts
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
    
    // Generate function graph shortcut
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
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);  // 只显示图标，更简约
    toolBar->setIconSize(QSize(20, 20));  // 设置图标大小
    
    // 文件操作组 - 核心功能
    toolBar->addAction(openAction_);
    toolBar->addAction(saveAction_);
    toolBar->addSeparator();
    
    // 项目操作组
    openProjectAction_ = new QAction(style()->standardIcon(QStyle::SP_DirIcon), "Open Project", this);
    connect(openProjectAction_, &QAction::triggered, this, &MainWindow::openProject);
    toolBar->addAction(openProjectAction_);
    toolBar->addSeparator();
    
    // 编辑操作组
    editAction_ = new QAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Edit", this);
    connect(editAction_, &QAction::triggered, this, &MainWindow::toggleEditMode);
    toolBar->addAction(editAction_);
    toolBar->addSeparator();
    
    // 解析操作组 - 简化设计
    QMenu* parseMenu = new QMenu();
    parseMenu->addAction("Parse XML", this, &MainWindow::parseXml);
    parseMenu->addAction("Parse C++", this, &MainWindow::parseCpp);
    parseMenu->addAction("Parse Python", this, &MainWindow::parsePython);
    parseMenu->addAction("Parse Go", this, &MainWindow::parseGo);
    parseMenu->addSeparator();
    parseMenu->addAction("Generate Function Graph", this, &MainWindow::generateFunctionGraph);
    
    parseButton_ = new QToolButton();
    parseButton_->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    parseButton_->setMenu(parseMenu);
    parseButton_->setPopupMode(QToolButton::InstantPopup);
    parseButton_->setEnabled(false);
    parseButton_->setToolTip("Parse Code");
    toolBar->addWidget(parseButton_);
    toolBar->addSeparator();
    
    // 搜索功能
    toolBar->addAction(searchAction_);
    toolBar->addSeparator();
    
    // 主题切换
    toolBar->addAction(toggleThemeAction_);
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("Ready");
    
    // Add progress bar - 更简约的设计
    progressBar_ = new QProgressBar();
    progressBar_->setVisible(false);
    progressBar_->setMaximumWidth(150);
    progressBar_->setStyleSheet("QProgressBar { height: 16px; }");
    statusBar()->addPermanentWidget(progressBar_);
    
    // Add line count label - 参考VSCode设计
    lineCountLabel_ = new QLabel("Lines: 0");
    lineCountLabel_->setStyleSheet("color: #CCCCCC; font-weight: 500; font-size: 12px; padding: 2px 8px;");
    statusBar()->addPermanentWidget(lineCountLabel_);
    
    // Add character count label
    charCountLabel_ = new QLabel("Chars: 0");
    charCountLabel_->setStyleSheet("color: #CCCCCC; font-weight: 500; font-size: 12px; padding: 2px 8px;");
    statusBar()->addPermanentWidget(charCountLabel_);
}

void MainWindow::setupStyle() {
    // Set application style
    QApplication::setStyle("Fusion");
    
    // Create modern dark palette - 参考VSCode配色
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::WindowText, QColor(212, 212, 212));
    darkPalette.setColor(QPalette::Base, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::AlternateBase, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ToolTipText, QColor(212, 212, 212));
    darkPalette.setColor(QPalette::Text, QColor(212, 212, 212));
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::ButtonText, QColor(212, 212, 212));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Link, QColor(0, 122, 204));
    darkPalette.setColor(QPalette::Highlight, QColor(0, 122, 204));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    
    QApplication::setPalette(darkPalette);
    
    // Set stylesheet for custom styling - 参考VSCode等现代编辑器设计
    QString styleSheet = R"(
        QMainWindow {
            background-color: #1E1E1E;
            color: #D4D4D4;
        }
        
        /* 侧边栏样式 - 参考VSCode设计 */
        QWidget#leftPanel {
            background-color: #252526;
            border-right: 1px solid #2D2D30;
            min-width: 200px;
            max-width: 300px;
        }
        
        QTreeWidget {
            background-color: #252526;
            border: none;
            color: #CCCCCC;
            font-family: 'Segoe UI', 'Cascadia Code', 'Consolas', monospace;
            font-size: 13px;
            outline: none;
            selection-background-color: #094771;
        }
        
        QTreeWidget::item {
            padding: 4px 8px;
            border: none;
            min-height: 22px;
            margin: 1px 0px;
        }
        
        QTreeWidget::item:selected {
            background-color: #094771;
            color: white;
            border-radius: 3px;
        }
        
        QTreeWidget::item:hover {
            background-color: #2A2D2E;
            border-radius: 3px;
        }
        
        QTreeWidget::item:alternate {
            background-color: transparent;
        }
        
        QTreeWidget::branch {
            background-color: transparent;
            width: 16px;
        }
        
        QTreeWidget::branch:has-children:!has-siblings:closed,
        QTreeWidget::branch:closed:has-children:has-siblings {
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTYgNEwxMCA4TDYgMTJWNloiIGZpbGw9IiM4ODg4ODgiLz4KPC9zdmc+);
        }
        
        QTreeWidget::branch:open:has-children:!has-siblings,
        QTreeWidget::branch:open:has-children:has-siblings {
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQgNkw4IDEwTDQgMTRWNloiIGZpbGw9IiM4ODg4ODgiLz4KPC9zdmc+);
        }
        
        /* 编辑器样式 - 参考VSCode */
        QTextEdit, QPlainTextEdit {
            background-color: #1E1E1E;
            border: none;
            color: #D4D4D4;
            font-family: 'Cascadia Code', 'JetBrains Mono', 'Consolas', 'Monaco', monospace;
            font-size: 14px;
            line-height: 1.5;
            selection-background-color: #264F78;
            padding: 0px;
        }
        
        QPlainTextEdit {
            selection-background-color: #264F78;
        }
        
        /* 按钮样式 - 现代化简约设计 */
        QPushButton {
            background-color: #0E639C;
            border: none;
            color: white;
            padding: 8px 16px;
            font-weight: 500;
            border-radius: 6px;
            min-height: 28px;
            min-width: 80px;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
        }
        
        QPushButton:hover {
            background-color: #1177BB;
            transform: translateY(-1px);
        }
        
        QPushButton:pressed {
            background-color: #005A9E;
            transform: translateY(0px);
        }
        
        QPushButton:disabled {
            background-color: #3E3E42;
            color: #6A6A6A;
        }
        
        /* 工具栏样式 - 更简约 */
        QToolBar {
            background-color: #2D2D30;
            border: none;
            border-bottom: 1px solid #3E3E42;
            spacing: 6px;
            padding: 6px 12px;
        }
        
        QToolButton {
            background-color: transparent;
            border: none;
            color: #CCCCCC;
            padding: 6px 10px;
            border-radius: 4px;
            font-size: 13px;
            min-width: 32px;
            min-height: 32px;
        }
        
        QToolButton:hover {
            background-color: #2A2D2E;
        }
        
        QToolButton:pressed {
            background-color: #094771;
        }
        
        QToolButton:disabled {
            color: #6A6A6A;
        }
        
        /* 菜单栏样式 - 更现代 */
        QMenuBar {
            background-color: #2D2D30;
            color: #CCCCCC;
            border: none;
            border-bottom: 1px solid #3E3E42;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
        }
        
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
            border-radius: 4px;
            margin: 2px;
        }
        
        QMenuBar::item:selected {
            background-color: #094771;
        }
        
        QMenu {
            background-color: #2D2D30;
            border: 1px solid #3E3E42;
            color: #CCCCCC;
            font-size: 13px;
            border-radius: 6px;
            padding: 4px;
        }
        
        QMenu::item {
            padding: 8px 20px;
            border-radius: 4px;
            margin: 1px;
        }
        
        QMenu::item:selected {
            background-color: #094771;
        }
        
        QMenu::separator {
            height: 1px;
            background-color: #3E3E42;
            margin: 6px 12px;
        }
        
        /* 状态栏样式 - 更简约 */
        QStatusBar {
            background-color: #007ACC;
            color: white;
            border: none;
            font-size: 12px;
            font-family: 'Segoe UI', sans-serif;
            padding: 4px 12px;
        }
        
        /* 标签页样式 - 参考VSCode */
        QTabWidget::pane {
            border: none;
            background-color: #1E1E1E;
        }
        
        QTabBar::tab {
            background-color: #2D2D30;
            color: #CCCCCC;
            padding: 10px 20px;
            margin-right: 1px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            font-size: 13px;
            min-width: 120px;
        }
        
        QTabBar::tab:selected {
            background-color: #1E1E1E;
            color: white;
            border-bottom: 2px solid #007ACC;
        }
        
        QTabBar::tab:hover {
            background-color: #3E3E42;
        }
        
        QTabBar::close-button {
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iMTIiIHZpZXdCb3g9IjAgMCAxMiAxMiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTkgM0wzIDlNMyAzTDkgOSIgc3Ryb2tlPSIjQ0NDQ0NDIiBzdHJva2Utd2lkdGg9IjEuNSIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIi8+Cjwvc3ZnPg==);
            width: 16px;
            height: 16px;
        }
        
        QTabBar::close-button:hover {
            background-color: #E81123;
            border-radius: 8px;
        }
        
        /* 标签样式 */
        QLabel {
            color: #CCCCCC;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
        }
        
        /* 进度条样式 - 更现代 */
        QProgressBar {
            border: none;
            border-radius: 6px;
            background-color: #2D2D30;
            text-align: center;
            color: white;
            height: 8px;
        }
        
        QProgressBar::chunk {
            background-color: #007ACC;
            border-radius: 6px;
        }
        
        /* 滚动条样式 - 参考VSCode */
        QScrollBar:vertical {
            background-color: #1E1E1E;
            width: 12px;
            border: none;
        }
        
        QScrollBar::handle:vertical {
            background-color: #424242;
            border-radius: 6px;
            min-height: 20px;
            margin: 2px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #4F4F4F;
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        
        QScrollBar:horizontal {
            background-color: #1E1E1E;
            height: 12px;
            border: none;
        }
        
        QScrollBar::handle:horizontal {
            background-color: #424242;
            border-radius: 6px;
            min-width: 20px;
            margin: 2px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background-color: #4F4F4F;
        }
        
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
        
        /* 分割器样式 - 更现代 */
        QSplitter::handle {
            background-color: #2D2D30;
            border: none;
            margin: 1px;
        }
        
        QSplitter::handle:hover {
            background-color: #007ACC;
        }
        
        QSplitter::handle:pressed {
            background-color: #005A9E;
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
        
        // Load content
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            xmlEditor_->setPlainText(content);
            originalXmlContent_ = content;
            isEditing_ = false;
            editAction_->setEnabled(true);
            saveAction_->setEnabled(false);
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

void MainWindow::openProject() {
    QString projectPath = QFileDialog::getExistingDirectory(this,
        "Open Project Folder", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!projectPath.isEmpty()) {
        // Clear current display
        clearDisplay();
        
        // Set project path
        currentProjectPath_ = projectPath.toStdString();
        
        // Update file label to show project name
        QFileInfo projectInfo(projectPath);
        fileLabel_->setText("Project: " + projectInfo.fileName());
        
        // Populate tree widget with project structure
        populateProjectTree(projectPath);
        
        // Enable parse button for project analysis
        parseButton_->setEnabled(true);
        
        statusBar()->showMessage("Project opened: " + projectPath);
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
        // Hide analysis panel for markdown files
        if (mainSplitter_->count() == 3) {
            rightPanel_->hide();
            mainSplitter_->setSizes({350, 650});
        }
        parseButton_->setEnabled(false);
    } else if (isCppMode_) {
        currentHighlighter_ = new CppHighlighter(xmlEditor_->document());
        parseButton_->setEnabled(true);
    } else if (isPythonMode_) {
        currentHighlighter_ = new PythonHighlighter(xmlEditor_->document());
        parseButton_->setEnabled(true);
    } else if (isGoMode_) {
        currentHighlighter_ = new GoHighlighter(xmlEditor_->document());
        parseButton_->setEnabled(true);
    } else {
        currentHighlighter_ = new XmlHighlighter(xmlEditor_->document());
        parseButton_->setEnabled(true);
    }
}

void MainWindow::renderMarkdownPreview() {
    if (!isMarkdownMode_ || !markdownPreview_) return;
    // Simplified: Use QTextBrowser's setMarkdown (Qt >= 5.14) or manual fallback
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    markdownPreview_->setMarkdown(xmlEditor_->toPlainText());
#else
    // Fallback: Convert to HTML based on simple replacements (bold/italic/headers/code)
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
    showAnalysisPanel();
    
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
    Q_UNUSED(column)
    if (!item) return;
    
    // Get the data from the item
    QVariant itemData = item->data(0, Qt::UserRole);
    if (itemData.isValid()) {
        QString filePath = itemData.toString();
        QFileInfo fileInfo(filePath);
        
        // If it's a file, load it in the editor
        if (fileInfo.isFile()) {
            loadFileFromPath(filePath);
        }
        // If it's a directory, expand/collapse it
        else if (fileInfo.isDir()) {
            item->setExpanded(!item->isExpanded());
        }
    }
    
    // Check if it's an XML node (for XML parsing results)
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

void MainWindow::showAnalysisPanel() {
    // Show the analysis panel if it's hidden
    if (rightPanel_->isHidden()) {
        rightPanel_->show();
        mainSplitter_->setSizes({300, 500, 300});
    } else if (mainSplitter_->count() == 2) {
        // Add the analysis panel to the splitter if not already added
        mainSplitter_->addWidget(rightPanel_);
        mainSplitter_->setSizes({300, 500, 300});
    }
}

void MainWindow::updateLineCount() {
    if (!xmlEditor_) return;
    
    int lineCount = xmlEditor_->document()->blockCount();
    int charCount = xmlEditor_->toPlainText().length();
    
    lineCountLabel_->setText(QString("Lines: %1").arg(lineCount));
    charCountLabel_->setText(QString("Chars: %1").arg(charCount));
}


void MainWindow::populateProjectTree(const QString& projectPath) {
    treeWidget_->clear();
    treeWidget_->setHeaderLabel("Project Structure");
    
    QDir projectDir(projectPath);
    if (!projectDir.exists()) {
        return;
    }
    
    // Create root item
    QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget_);
    rootItem->setText(0, projectDir.dirName());
    rootItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    rootItem->setData(0, Qt::UserRole, projectPath);
    
    // Populate with project files
    populateProjectTreeRecursive(projectDir, rootItem);
    
    // Expand root item
    rootItem->setExpanded(true);
}

void MainWindow::populateProjectTreeRecursive(const QDir& dir, QTreeWidgetItem* parentItem) {
    QStringList filters;
    filters << "*.cpp" << "*.c" << "*.h" << "*.hpp" << "*.py" << "*.go" << "*.xml" << "*.md" << "*.txt" << "*.json" << "*.yaml" << "*.yml";
    
    // Add directories first
    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo& dirInfo : dirs) {
        // Skip common build/cache directories
        if (dirInfo.fileName().startsWith('.') || 
            dirInfo.fileName() == "build" || 
            dirInfo.fileName() == "bin" ||
            dirInfo.fileName() == "obj" ||
            dirInfo.fileName() == "node_modules") {
            continue;
        }
        
        QTreeWidgetItem* dirItem = new QTreeWidgetItem(parentItem);
        dirItem->setText(0, dirInfo.fileName());
        dirItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        dirItem->setData(0, Qt::UserRole, dirInfo.absoluteFilePath());
        
        // Recursively populate subdirectories
        QDir subDir(dirInfo.absoluteFilePath());
        populateProjectTreeRecursive(subDir, dirItem);
    }
    
    // Add files
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo& fileInfo : files) {
        QTreeWidgetItem* fileItem = new QTreeWidgetItem(parentItem);
        fileItem->setText(0, fileInfo.fileName());
        
        // Set appropriate icon based on file type
        QString suffix = fileInfo.suffix().toLower();
        if (suffix == "cpp" || suffix == "c" || suffix == "h" || suffix == "hpp") {
            fileItem->setIcon(0, style()->standardIcon(QStyle::SP_ComputerIcon));
        } else if (suffix == "py") {
            fileItem->setIcon(0, style()->standardIcon(QStyle::SP_ComputerIcon));
        } else if (suffix == "go") {
            fileItem->setIcon(0, style()->standardIcon(QStyle::SP_ComputerIcon));
        } else if (suffix == "xml") {
            fileItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        } else if (suffix == "md") {
            fileItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        } else {
            fileItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        }
        
        fileItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
    }
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
            // Implement regex replacement
            QRegExp regex(searchText);
            if (regex.isValid()) {
                Qt::CaseSensitivity caseSensitivity = searchDialog_->isCaseSensitive() ? 
                    Qt::CaseSensitive : Qt::CaseInsensitive;
                regex.setCaseSensitivity(caseSensitivity);
                
                QString originalContent = content;
                content.replace(regex, replaceText);
                int count = (originalContent.length() - content.length()) / (searchText.length() - replaceText.length());
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
        editAction_->setText("Cancel Edit");
        saveAction_->setEnabled(true);
        statusBar()->showMessage(isMarkdownMode_ ? "Edit mode enabled - you can now modify the Markdown" : "Edit mode enabled - you can now modify the XML");
    } else {
        // Cancel edit mode
        isEditing_ = false;
        xmlEditor_->setPlainText(originalXmlContent_);
        xmlEditor_->setReadOnly(true);
        editAction_->setText("Edit");
        saveAction_->setEnabled(false);
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
            editAction_->setText("Edit");
            saveAction_->setEnabled(false);
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
        editAction_->setText("Edit");
        saveAction_->setEnabled(false);
        
        statusBar()->showMessage("XML content saved successfully");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save XML content");
    }
}

void MainWindow::parseCpp() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "Please open a C++ file first");
        return;
    }
    
    showAnalysisPanel();
    
    // Show progress bar
    progressBar_->setVisible(true);
    progressBar_->setRange(0, 0); // Indeterminate progress
    statusBar()->showMessage("Parsing C++ file...");
    QApplication::processEvents(); // Update UI
    
    QString content = xmlEditor_->toPlainText();
    std::string cppContent = content.toStdString();
    
    // Check file size, show prompt for large files
    if (cppContent.length() > 100000) { // 100KB
        progressBar_->setRange(0, 100);
        progressBar_->setValue(50);
        statusBar()->showMessage("Parsing large C++ file, please wait...");
        QApplication::processEvents();
    }
    
    if (cppParser_.parseFile(cppContent)) {
        progressBar_->setValue(100);
        statusBar()->showMessage("C++ file parsed successfully");
        
        // Show parsing statistics in analysis panel
        const auto& functions = cppParser_.getFunctions();
        const auto& classes = cppParser_.getClasses();
        QString info = QString("<h3>C++ Analysis Results</h3>"
                              "<p><b>Functions found:</b> %1</p>"
                              "<p><b>Classes found:</b> %2</p>"
                              "<p>Click 'Generate Function Graph' to visualize the code structure.</p>")
                      .arg(functions.size())
                      .arg(classes.size());
        detailsTextEdit_->setHtml(info);
        rightTabs_->setCurrentIndex(0); // Show details tab
        
        // Enable function graph generation
        parseButton_->setEnabled(true);
    } else {
        QMessageBox::critical(this, "Error", "C++ file parsing failed");
    }
    
    // Hide progress bar
    progressBar_->setVisible(false);
}

void MainWindow::parsePython() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "Please open a Python file first");
        return;
    }
    
    showAnalysisPanel();
    
    QString content = xmlEditor_->toPlainText();
    std::string pythonContent = content.toStdString();
    
    if (pythonParser_.parseFile(pythonContent)) {
        statusBar()->showMessage("Python file parsed successfully");
        
        // Show parsing statistics in analysis panel
        const auto& functions = pythonParser_.getFunctions();
        const auto& classes = pythonParser_.getClasses();
        QString info = QString("<h3>Python Analysis Results</h3>"
                              "<p><b>Functions found:</b> %1</p>"
                              "<p><b>Classes found:</b> %2</p>"
                              "<p>Click 'Generate Function Graph' to visualize the code structure.</p>")
                      .arg(functions.size())
                      .arg(classes.size());
        detailsTextEdit_->setHtml(info);
        rightTabs_->setCurrentIndex(0); // Show details tab
        
        // Enable function graph generation
        parseButton_->setEnabled(true);
    } else {
        QMessageBox::critical(this, "Error", "Python file parsing failed");
    }
}

void MainWindow::generateFunctionGraph() {
    bool hasFunctions = false;
    
    if (isCppMode_ && !cppParser_.getFunctions().empty()) {
        // Set C++ parsing data to graph view
        functionGraphView_->setParserData(cppParser_);
        hasFunctions = true;
    } else if (isPythonMode_ && !pythonParser_.getFunctions().empty()) {
        // Need to create a conversion function to convert Python data to C++ data format
        // Or modify FunctionGraphView to support multiple parser types
        // For simplicity, we temporarily use adapter pattern
        CppParser adaptedParser;
        adaptPythonToCppParser(adaptedParser);
        functionGraphView_->setParserData(adaptedParser);
        hasFunctions = true;
    } else if (isGoMode_ && !goParser_.getFunctions().empty()) {
        // Convert Go parsing data to C++ data format
        CppParser adaptedParser;
        adaptGoToCppParser(adaptedParser);
        functionGraphView_->setParserData(adaptedParser);
        hasFunctions = true;
    }
    
    if (!hasFunctions) {
        QMessageBox::warning(this, "Warning", "Please parse code file first");
        return;
    }
    
    // Show analysis panel if not already shown
    showAnalysisPanel();
    
    // Generate function relationship graph
    functionGraphView_->generateGraph();
    
    // Switch to function graph tab
    rightTabs_->setCurrentIndex(2);
    
    statusBar()->showMessage("Function graph generation completed");
}

void MainWindow::parseGo() {
    if (currentFilePath_.empty()) {
        QMessageBox::warning(this, "Warning", "Please open a Go file first");
        return;
    }
    
    showAnalysisPanel();
    
    QString content = xmlEditor_->toPlainText();
    std::string goContent = content.toStdString();
    
    if (goParser_.parseFile(goContent)) {
        statusBar()->showMessage("Go file parsed successfully");
        
        // Show parsing statistics in analysis panel
        const auto& functions = goParser_.getFunctions();
        const auto& structs = goParser_.getStructs();
        const auto& interfaces = goParser_.getInterfaces();
        QString info = QString("<h3>Go Analysis Results</h3>"
                              "<p><b>Functions found:</b> %1</p>"
                              "<p><b>Structs found:</b> %2</p>"
                              "<p><b>Interfaces found:</b> %3</p>"
                              "<p>Click 'Generate Function Graph' to visualize the code structure.</p>")
                      .arg(functions.size())
                      .arg(structs.size())
                      .arg(interfaces.size());
        detailsTextEdit_->setHtml(info);
        rightTabs_->setCurrentIndex(0); // Show details tab
        
        // Enable function graph generation
        parseButton_->setEnabled(true);
    } else {
        QMessageBox::critical(this, "Error", "Go file parsing failed");
    }
}

void MainWindow::adaptPythonToCppParser(CppParser& cppParser) {
    // Clear target parser
    cppParser.clear();
    
    // Convert Python functions to C++ function format
    const auto& pythonFunctions = pythonParser_.getFunctions();
    const auto& pythonClasses = pythonParser_.getClasses();
    const auto& pythonCalls = pythonParser_.getFunctionCalls();
    
    // This is a simplified adapter implementation
    // In actual applications, you may need to modify FunctionGraphView to support generic parser interface
    
    // Since CppParser's internal data is private, we need to populate data through parsing
    // Here we construct a temporary C++ code string to simulate conversion
    std::string adaptedCppCode;
    
    // Convert class definitions
    for (const auto& pythonClass : pythonClasses) {
        adaptedCppCode += "class " + pythonClass.name;
        if (!pythonClass.baseClasses.empty()) {
            adaptedCppCode += " : public " + pythonClass.baseClasses[0];
        }
        adaptedCppCode += " {\npublic:\n";
        
        // Convert class methods
        for (const auto& method : pythonClass.methods) {
            adaptedCppCode += "    void " + method.name + "(";
            for (size_t i = 0; i < method.parameters.size(); ++i) {
                if (i > 0) adaptedCppCode += ", ";
                adaptedCppCode += "int " + method.parameters[i].name;
            }
            adaptedCppCode += ") {\n";
            
            // Add function calls
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
    
    // Convert independent functions
    for (const auto& pythonFunc : pythonFunctions) {
        if (pythonFunc.className.empty()) { // Only handle independent functions
            adaptedCppCode += "void " + pythonFunc.name + "(";
            for (size_t i = 0; i < pythonFunc.parameters.size(); ++i) {
                if (i > 0) adaptedCppCode += ", ";
                adaptedCppCode += "int " + pythonFunc.parameters[i].name;
            }
            adaptedCppCode += ") {\n";
            
            // Add function calls
            auto it = pythonCalls.find(pythonFunc.name);
            if (it != pythonCalls.end()) {
                for (const std::string& calledFunc : it->second) {
                    adaptedCppCode += "    " + calledFunc + "();\n";
                }
            }
            
            adaptedCppCode += "}\n\n";
        }
    }
    
    // Parse adapted code
    cppParser.parseFile(adaptedCppCode);
}

void MainWindow::adaptGoToCppParser(CppParser& cppParser) {
    // Clear target parser
    cppParser.clear();
    
    // Convert Go functions to C++ function format
    const auto& goFunctions = goParser_.getFunctions();
    const auto& goStructs = goParser_.getStructs();
    const auto& goCalls = goParser_.getFunctionCalls();
    
    // Construct a temporary C++ code string to simulate conversion
    std::string adaptedCppCode;
    
    // Convert struct definitions
    for (const auto& goStruct : goStructs) {
        adaptedCppCode += "class " + goStruct.name + " {\npublic:\n";
        
        // Convert struct fields
        for (const auto& field : goStruct.fields) {
            adaptedCppCode += "    int " + field + ";\n";
        }
        
        adaptedCppCode += "};\n\n";
    }
    
    // Convert function definitions
    for (const auto& goFunc : goFunctions) {
        // Handle return types (Go can have multiple return values)
        std::string returnType = "void";
        if (!goFunc.returnTypes.empty()) {
            returnType = goFunc.returnTypes[0]; // Simplified handling, only take first return type
            // Map Go types to C++ types
            if (returnType == "string") returnType = "std::string";
            else if (returnType == "int") returnType = "int";
            else if (returnType == "bool") returnType = "bool";
            else if (returnType == "float64") returnType = "double";
            else returnType = "int"; // Default type
        }
        
        // Handle methods (with receiver)
        if (goFunc.isMethod) {
            adaptedCppCode += "class " + goFunc.receiverType + " {\npublic:\n";
            adaptedCppCode += "    " + returnType + " " + goFunc.name + "(";
        } else {
            adaptedCppCode += returnType + " " + goFunc.name + "(";
        }
        
        // Convert parameters
        for (size_t i = 0; i < goFunc.parameters.size(); ++i) {
            if (i > 0) adaptedCppCode += ", ";
            
            std::string paramType = goFunc.parameters[i].type;
            // Simple type mapping
            if (paramType == "string") paramType = "std::string";
            else if (paramType == "int") paramType = "int";
            else if (paramType == "bool") paramType = "bool";
            else if (paramType == "float64") paramType = "double";
            else paramType = "int"; // Default type
            
            adaptedCppCode += paramType + " " + goFunc.parameters[i].name;
        }
        
        adaptedCppCode += ") {\n";
        
        // Add function calls
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
    
    // Parse adapted code
    cppParser.parseFile(adaptedCppCode);
}

void MainWindow::loadFileFromPath(const QString& filePath) {
    // Set current file path
    currentFilePath_ = filePath.toStdString();
    
    // Update file label
    QFileInfo fileInfo(filePath);
    fileLabel_->setText(fileInfo.fileName());
    
    // Read file content
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", 
            QString("Cannot open file: %1").arg(filePath));
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Set editor content
    xmlEditor_->setPlainText(content);
    
    // Set mode based on file extension
    QString extension = fileInfo.suffix().toLower();
    isMarkdownMode_ = (extension == "md" || extension == "markdown");
    isCppMode_ = (extension == "cpp" || extension == "cxx" || extension == "cc" || extension == "c");
    isPythonMode_ = (extension == "py");
    isGoMode_ = (extension == "go");
    
    // Apply syntax highlighting
    applyHighlighterForCurrentFile();
    
    // Enable parse button for supported file types
    if (!isMarkdownMode_) {
        parseButton_->setEnabled(true);
    }
    
    // If it's a Markdown file, render preview
    if (isMarkdownMode_) {
        renderMarkdownPreview();
    }
    
    statusBar()->showMessage(QString("Loaded: %1").arg(filePath));
}

void MainWindow::toggleTheme() {
    isDarkTheme_ = !isDarkTheme_;
    
    if (isDarkTheme_) {
        // Apply dark theme
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
        statusBar()->showMessage("Switched to dark theme");
    } else {
        // Apply light theme
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
        statusBar()->showMessage("Switched to light theme");
    }
    
    // Reapply syntax highlighting to adapt to new theme
    applyHighlighterForCurrentFile();
} 