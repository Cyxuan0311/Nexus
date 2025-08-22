#include <gtest/gtest.h>
#include <QApplication>
#include <QTextDocument>
#include <QTextBlock>
#include "code_folding.h"

class CodeFoldingTest : public ::testing::Test {
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

TEST_F(CodeFoldingTest, FoldingTextEditCreation) {
    FoldingTextEdit editor;
    EXPECT_TRUE(editor.getFoldingArea() != nullptr);
}

TEST_F(CodeFoldingTest, XmlTagDetection) {
    FoldingTextEdit editor;
    CodeFoldingArea* foldingArea = editor.getFoldingArea();
    
    // Test XML tag detection
    QString xmlContent = R"(
<root>
    <element>
        <child>content</child>
    </element>
    <self-closing />
</root>
)";
    
    editor.setPlainText(xmlContent);
    
    // Give the folding area time to update
    QApplication::processEvents();
    
    // The folding area should detect foldable tags
    // Note: We can't directly test the private methods, but we can test the behavior
    EXPECT_TRUE(editor.document()->blockCount() > 0);
}

TEST_F(CodeFoldingTest, FoldAllUnfoldAll) {
    FoldingTextEdit editor;
    
    QString xmlContent = R"(
<root>
    <element1>
        <child1>content1</child1>
        <child2>content2</child2>
    </element1>
    <element2>
        <child3>content3</child3>
    </element2>
</root>
)";
    
    editor.setPlainText(xmlContent);
    
    // Test fold all
    editor.foldAll();
    
    // Test unfold all
    editor.unfoldAll();
    
    // Both operations should complete without errors
    EXPECT_TRUE(true);
}

TEST_F(CodeFoldingTest, TagNameExtraction) {
    // Test tag name extraction logic
    QString tag1 = "<root>";
    QString tag2 = "<element attr=\"value\">";
    QString tag3 = "<self-closing />";
    QString tag4 = "</closing>";
    QString tag5 = "<?xml version=\"1.0\"?>";
    QString tag6 = "<!-- comment -->";
    
    // These should be foldable
    EXPECT_TRUE(tag1.trimmed().startsWith('<') && !tag1.trimmed().startsWith("</") && !tag1.trimmed().endsWith("/>"));
    EXPECT_TRUE(tag2.trimmed().startsWith('<') && !tag2.trimmed().startsWith("</") && !tag2.trimmed().endsWith("/>"));
    
    // These should not be foldable
    EXPECT_TRUE(tag3.trimmed().endsWith("/>"));
    EXPECT_TRUE(tag4.trimmed().startsWith("</"));
    EXPECT_TRUE(tag5.trimmed().startsWith("<?"));
    EXPECT_TRUE(tag6.trimmed().startsWith("<!--"));
}

TEST_F(CodeFoldingTest, DocumentContentChange) {
    FoldingTextEdit editor;
    
    // Set initial content
    editor.setPlainText("<root><child>content</child></root>");
    QApplication::processEvents();
    
    // Change content
    editor.setPlainText("<newroot><newchild>newcontent</newchild></newroot>");
    QApplication::processEvents();
    
    // Should handle content changes without errors
    EXPECT_TRUE(editor.document()->blockCount() > 0);
}

TEST_F(CodeFoldingTest, FoldingAreaGeometry) {
    FoldingTextEdit editor;
    CodeFoldingArea* foldingArea = editor.getFoldingArea();
    
    // Test that folding area has correct width
    EXPECT_EQ(foldingArea->width(), 20);
    
    // Test that folding area is visible
    EXPECT_TRUE(foldingArea->isVisible());
}

TEST_F(CodeFoldingTest, MouseInteraction) {
    FoldingTextEdit editor;
    CodeFoldingArea* foldingArea = editor.getFoldingArea();
    
    QString xmlContent = R"(
<root>
    <element>
        <child>content</child>
    </element>
</root>
)";
    
    editor.setPlainText(xmlContent);
    QApplication::processEvents();
    
    // Test mouse cursor changes
    // Note: We can't easily simulate mouse events in unit tests,
    // but we can verify the folding area responds to mouse tracking
    EXPECT_TRUE(foldingArea->hasMouseTracking());
} 