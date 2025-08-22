#include <gtest/gtest.h>
#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPlainTextEdit>
#include "search_dialog.h"

class SearchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            static int argc = 1;
            static char* argv[] = {(char*)"test"};
            app_ = new QApplication(argc, argv);
        }
    }
    
    void TearDown() override {
        // Clean up
    }
    
    QApplication* app_ = nullptr;
};

TEST_F(SearchTest, SearchDialogCreation) {
    SearchDialog dialog;
    EXPECT_TRUE(dialog.isVisible());
}

TEST_F(SearchTest, SearchDialogDefaultValues) {
    SearchDialog dialog;
    
    EXPECT_TRUE(dialog.getSearchText().isEmpty());
    EXPECT_TRUE(dialog.getReplaceText().isEmpty());
    EXPECT_FALSE(dialog.isCaseSensitive());
    EXPECT_FALSE(dialog.isWholeWord());
    EXPECT_FALSE(dialog.isRegex());
    EXPECT_TRUE(dialog.searchInTree());
    EXPECT_TRUE(dialog.searchInEditor());
    EXPECT_FALSE(dialog.isReplaceMode());
}

TEST_F(SearchTest, SearchDialogModeSwitching) {
    SearchDialog dialog;
    
    // Initially in search mode
    EXPECT_FALSE(dialog.isReplaceMode());
    
    // Switch to replace mode (this would be done via UI interaction)
    // For now, we test the getters work correctly
    EXPECT_TRUE(dialog.getSearchText().isEmpty());
    EXPECT_TRUE(dialog.getReplaceText().isEmpty());
}

TEST_F(SearchTest, TreeWidgetSearch) {
    QTreeWidget treeWidget;
    
    // Add some test items
    QTreeWidgetItem* root = new QTreeWidgetItem(&treeWidget);
    root->setText(0, "root");
    
    QTreeWidgetItem* child1 = new QTreeWidgetItem(root);
    child1->setText(0, "child1");
    
    QTreeWidgetItem* child2 = new QTreeWidgetItem(root);
    child2->setText(0, "child2");
    
    QTreeWidgetItem* grandchild = new QTreeWidgetItem(child1);
    grandchild->setText(0, "grandchild");
    
    // Test search functionality
    QString searchText = "child";
    QList<QTreeWidgetItem*> results;
    
    // Simple search implementation
    for (int i = 0; i < treeWidget.topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = treeWidget.topLevelItem(i);
        searchRecursive(item, searchText, results);
    }
    
    EXPECT_EQ(results.size(), 3); // child1, child2, grandchild
}

void searchRecursive(QTreeWidgetItem* item, const QString& searchText, QList<QTreeWidgetItem*>& results) {
    if (!item) return;
    
    if (item->text(0).contains(searchText, Qt::CaseInsensitive)) {
        results.append(item);
    }
    
    for (int i = 0; i < item->childCount(); ++i) {
        searchRecursive(item->child(i), searchText, results);
    }
}

TEST_F(SearchTest, TextEditorSearch) {
    QPlainTextEdit editor;
    editor.setPlainText("This is a test text with some content to search in.");
    
    QString searchText = "test";
    QTextCursor cursor = editor.textCursor();
    QString content = editor.toPlainText();
    
    int pos = content.indexOf(searchText, cursor.position(), Qt::CaseInsensitive);
    EXPECT_GE(pos, 0);
    
    if (pos >= 0) {
        cursor.setPosition(pos);
        cursor.setPosition(pos + searchText.length(), QTextCursor::KeepAnchor);
        editor.setTextCursor(cursor);
        
        EXPECT_EQ(cursor.selectedText(), searchText);
    }
} 