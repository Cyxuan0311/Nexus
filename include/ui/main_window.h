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
#include <QTabWidget>
#include <QTextBrowser>
#include <QProgressBar>
#include "xml_parser.h"
#include "xml_serializer.h"
#include "xml_highlighter.h"
#include "cpp_highlighter.h"
#include "python_highlighter.h"
#include "go_highlighter.h"
#include "cpp_parser.h"
#include "python_parser.h"
#include "go_parser.h"
#include "function_graph_view.h"
#include "search_dialog.h"
#include "code_folding.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	
	// Public methods
	void loadFileFromPath(const QString& filePath);

private slots:
	void openFile();
	void openProject();
	void parseXml();
	void parseCpp();
	void parsePython();
	void parseGo();
	void generateFunctionGraph();
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
	void populateProjectTree(const QString& projectPath);
	void populateProjectTreeRecursive(const QDir& dir, QTreeWidgetItem* parentItem);
	void displayNodeDetails(const std::shared_ptr<XmlNode>& node);
	void clearDisplay();
	void showAnalysisPanel();
	void updateLineCount();
	void renderMarkdownPreview();
	void applyHighlighterForCurrentFile();
	bool isCurrentFileMarkdown() const;
	bool isCurrentFileCpp() const;
	bool isCurrentFilePython() const;
	bool isCurrentFileGo() const;
	void adaptPythonToCppParser(CppParser& cppParser);
	void adaptGoToCppParser(CppParser& cppParser);
	void toggleTheme();
	
	// Search methods
	void searchInTreeWidget(const QString& searchText);
	void searchInTreeWidgetRecursive(QTreeWidgetItem* item, const QString& searchText);
	void searchInEditor(const QString& searchText);
	void highlightNextResult();
	
	// UI Components
	QSplitter* mainSplitter_;
	QWidget* leftPanel_;
	QWidget* rightPanel_;
	QWidget* centerPanel_;
	QTreeWidget* treeWidget_;
	QTextEdit* detailsTextEdit_;
	FoldingTextEdit* xmlEditor_;
	// 工具栏按钮引用
	QToolButton* parseButton_;
	QAction* editAction_;
	QAction* openProjectAction_;
	
	QLabel* fileLabel_;
	QTabWidget* rightTabs_;
	QTextBrowser* markdownPreview_;
	FunctionGraphView* functionGraphView_;
	QProgressBar* progressBar_;
	QLabel* lineCountLabel_;
	QLabel* charCountLabel_;
	
	// Data
	XmlParser parser_;
	XmlSerializer serializer_;
	CppParser cppParser_;
	PythonParser pythonParser_;
	GoParser goParser_;
	std::shared_ptr<XmlNode> rootNode_;
	std::string currentFilePath_;
	std::string currentProjectPath_;
	bool isEditing_;
	QString originalXmlContent_;
	SearchDialog* searchDialog_;
	QList<QTreeWidgetItem*> searchResults_;
	int currentSearchIndex_;
	bool isMarkdownMode_;
	bool isCppMode_;
	bool isPythonMode_;
	bool isGoMode_;
	QSyntaxHighlighter* currentHighlighter_;
	bool isDarkTheme_;
	
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
	QAction* parseCppAction_;
	QAction* parsePythonAction_;
	QAction* parseGoAction_;
	QAction* generateGraphAction_;
	QAction* toggleThemeAction_;
	QAction* exitAction_;
	QAction* aboutAction_;
};

#endif // MAIN_WINDOW_H 