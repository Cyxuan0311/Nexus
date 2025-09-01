#include "code_folding.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTextBlock>
#include <QTextDocument>
#include <QScrollBar>
#include <QApplication>
#include <QStyle>

CodeFoldingArea::CodeFoldingArea(FoldingTextEdit* editor)
    : QWidget(editor), editor_(editor), foldableLines_(), foldedBlocks_() {
    setFixedWidth(20);
    setMouseTracking(true);
    
    // Connect to editor signals
    connect(editor_->document(), &QTextDocument::contentsChanged, this, &CodeFoldingArea::updateFoldableLines);
    connect(editor_->verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeFoldingArea::update);
    
    updateFoldableLines();
}

void CodeFoldingArea::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    if (!editor_) return;
    
    QTextDocument* document = editor_->document();
    QTextCursor cursor = editor_->textCursor();
    
    // Get visible blocks
    QTextBlock block = document->firstBlock();
    int blockNumber = 0;
    
    while (block.isValid()) {
        QRectF blockRect = editor_->blockBoundingGeometry(block);
        QRectF blockRectTranslated = editor_->viewport()->mapToParent(blockRect.toRect());
        
        // Check if block is visible
        if (blockRectTranslated.bottom() >= 0 && blockRectTranslated.top() <= height()) {
            int y = blockRectTranslated.top() + blockRect.height() / 2 - 4;
            
            // Draw fold marker if this is a foldable line
            if (foldableLines_.contains(blockNumber)) {
                bool isFolded = foldedBlocks_.contains(blockNumber);
                
                // Draw fold indicator
                QRect foldRect(2, y, 12, 8);
                
                if (isFolded) {
                    // Draw folded state (plus sign)
                    painter.setPen(QColor(100, 200, 100));
                    painter.setBrush(QColor(50, 100, 50));
                    painter.drawRect(foldRect);
                    
                    painter.setPen(QColor(200, 200, 200));
                    painter.drawLine(foldRect.left() + 3, foldRect.center().y(), 
                                   foldRect.right() - 3, foldRect.center().y());
                    painter.drawLine(foldRect.center().x(), foldRect.top() + 3, 
                                   foldRect.center().x(), foldRect.bottom() - 3);
                } else {
                    // Draw unfolded state (minus sign)
                    painter.setPen(QColor(100, 200, 100));
                    painter.setBrush(QColor(50, 100, 50));
                    painter.drawRect(foldRect);
                    
                    painter.setPen(QColor(200, 200, 200));
                    painter.drawLine(foldRect.left() + 3, foldRect.center().y(), 
                                   foldRect.right() - 3, foldRect.center().y());
                }
            }
            
            // Draw fold line if this block is part of a fold
            if (isBlockInFold(blockNumber)) {
                painter.setPen(QColor(80, 80, 80));
                painter.drawLine(width() - 1, y + 4, width() - 1, y + 8);
            }
        }
        
        block = block.next();
        blockNumber++;
    }
}

void CodeFoldingArea::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int blockNumber = getBlockNumberAtY(event->y());
        
        if (foldableLines_.contains(blockNumber)) {
            if (foldedBlocks_.contains(blockNumber)) {
                unfoldBlock(blockNumber);
            } else {
                foldBlock(blockNumber);
            }
            update();
        }
    }
}

void CodeFoldingArea::mouseMoveEvent(QMouseEvent* event) {
    int blockNumber = getBlockNumberAtY(event->y());
    
    if (foldableLines_.contains(blockNumber)) {
        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

int CodeFoldingArea::getBlockNumberAtY(int y) const {
    if (!editor_) return -1;
    
    QTextDocument* document = editor_->document();
    QTextBlock block = document->firstBlock();
    int blockNumber = 0;
    
    while (block.isValid()) {
        QRectF blockRect = editor_->blockBoundingGeometry(block);
        QRectF blockRectTranslated = editor_->viewport()->mapToParent(blockRect.toRect());
        
        if (y >= blockRectTranslated.top() && y <= blockRectTranslated.bottom()) {
            return blockNumber;
        }
        
        block = block.next();
        blockNumber++;
    }
    
    return -1;
}

void CodeFoldingArea::updateFoldableLines() {
    foldableLines_.clear();
    
    if (!editor_) return;
    
    QTextDocument* document = editor_->document();
    QTextBlock block = document->firstBlock();
    int blockNumber = 0;
    
    while (block.isValid()) {
        QString text = block.text();
        
        // Check if this is an opening XML tag that can be folded
        if (isFoldableXmlTag(text)) {
            foldableLines_.insert(blockNumber);
        }
        
        block = block.next();
        blockNumber++;
    }
}

bool CodeFoldingArea::isFoldableXmlTag(const QString& text) const {
    // Simple XML tag detection
    QString trimmed = text.trimmed();
    
    // Check for opening tags like <tag>, <tag attr="value">, but not self-closing tags
    if (trimmed.startsWith('<') && !trimmed.startsWith("</") && !trimmed.endsWith("/>")) {
        // Extract tag name
        int spacePos = trimmed.indexOf(' ');
        int closePos = trimmed.indexOf('>');
        
        if (closePos > 0) {
            QString tagName = trimmed.mid(1, (spacePos > 0 && spacePos < closePos) ? 
                                        spacePos - 1 : closePos - 1);
            
            // Skip processing instructions, comments, etc.
            if (!tagName.startsWith('?') && !tagName.startsWith('!')) {
                return true;
            }
        }
    }
    
    return false;
}

void CodeFoldingArea::foldBlock(int blockNumber) {
    if (!editor_ || !foldableLines_.contains(blockNumber)) return;
    
    QTextDocument* document = editor_->document();
    QTextBlock startBlock = document->findBlockByNumber(blockNumber);
    
    if (!startBlock.isValid()) return;
    
    QString startText = startBlock.text().trimmed();
    QString tagName = extractTagName(startText);
    
    if (tagName.isEmpty()) return;
    
    // Find the closing tag
    QTextBlock endBlock = findClosingTag(startBlock, tagName);
    
    if (endBlock.isValid()) {
        // Hide the blocks between start and end
        QTextBlock block = startBlock.next();
        while (block.isValid() && block.blockNumber() < endBlock.blockNumber()) {
            block.setVisible(false);
            block = block.next();
        }
        
        foldedBlocks_.insert(blockNumber);
        editor_->document()->markContentsDirty(startBlock.position(), 
                                              endBlock.position() - startBlock.position());
    }
}

void CodeFoldingArea::unfoldBlock(int blockNumber) {
    if (!editor_ || !foldedBlocks_.contains(blockNumber)) return;
    
    QTextDocument* document = editor_->document();
    QTextBlock block = document->firstBlock();
    
    // Make all blocks visible
    while (block.isValid()) {
        block.setVisible(true);
        block = block.next();
    }
    
    foldedBlocks_.remove(blockNumber);
    editor_->document()->markContentsDirty(0, document->characterCount());
}

bool CodeFoldingArea::isBlockInFold(int blockNumber) const {
    for (int foldedBlock : foldedBlocks_) {
        if (blockNumber > foldedBlock) {
            // Check if this block is within the fold range
            QTextDocument* document = editor_->document();
            QTextBlock startBlock = document->findBlockByNumber(foldedBlock);
            QString tagName = extractTagName(startBlock.text().trimmed());
            QTextBlock endBlock = findClosingTag(startBlock, tagName);
            
            if (endBlock.isValid() && blockNumber < endBlock.blockNumber()) {
                return true;
            }
        }
    }
    return false;
}

QString CodeFoldingArea::extractTagName(const QString& text) const {
    if (!text.startsWith('<') || text.startsWith("</")) return QString();
    
    int spacePos = text.indexOf(' ');
    int closePos = text.indexOf('>');
    
    if (closePos > 0) {
        return text.mid(1, (spacePos > 0 && spacePos < closePos) ? 
                         spacePos - 1 : closePos - 1);
    }
    
    return QString();
}

QTextBlock CodeFoldingArea::findClosingTag(const QTextBlock& startBlock, const QString& tagName) const {
    if (!editor_) return QTextBlock();
    
    QTextDocument* document = editor_->document();
    QTextBlock block = startBlock.next();
    int depth = 1;
    
    while (block.isValid()) {
        QString text = block.text().trimmed();
        
        if (text.startsWith('<') && !text.startsWith("</")) {
            QString currentTagName = extractTagName(text);
            if (currentTagName == tagName) {
                depth++;
            }
        } else if (text.startsWith("</" + tagName + ">")) {
            depth--;
            if (depth == 0) {
                return block;
            }
        }
        
        block = block.next();
    }
    
    return QTextBlock();
}

void CodeFoldingArea::foldAll() {
    for (int blockNumber : foldableLines_) {
        if (!foldedBlocks_.contains(blockNumber)) {
            foldBlock(blockNumber);
        }
    }
    update();
}

void CodeFoldingArea::unfoldAll() {
    if (!editor_) return;
    
    QTextDocument* document = editor_->document();
    QTextBlock block = document->firstBlock();
    
    while (block.isValid()) {
        block.setVisible(true);
        block = block.next();
    }
    
    foldedBlocks_.clear();
    editor_->document()->markContentsDirty(0, document->characterCount());
    update();
}

// FoldingTextEdit implementation
FoldingTextEdit::FoldingTextEdit(QWidget* parent)
    : QPlainTextEdit(parent), foldingArea_(nullptr) {
    foldingArea_ = new CodeFoldingArea(this);
    
    // Connect scroll signals
    connect(verticalScrollBar(), &QScrollBar::valueChanged, 
            foldingArea_, &CodeFoldingArea::update);
    
    // Update folding area when content changes
    connect(document(), &QTextDocument::contentsChanged, 
            foldingArea_, &CodeFoldingArea::updateFoldableLines);
}

void FoldingTextEdit::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    
    // Update folding area geometry
    QRect cr = contentsRect();
    foldingArea_->setGeometry(QRect(cr.left(), cr.top(), 
                                   foldingArea_->width(), cr.height()));
}

void FoldingTextEdit::foldAll() {
    if (foldingArea_) {
        foldingArea_->foldAll();
    }
}

void FoldingTextEdit::unfoldAll() {
    if (foldingArea_) {
        foldingArea_->unfoldAll();
    }
}

CodeFoldingArea* FoldingTextEdit::getFoldingArea() const {
    return foldingArea_;
} 