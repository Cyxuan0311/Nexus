#include "search_dialog.h"
#include <QApplication>
#include <QStyle>

SearchDialog::SearchDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi();
    setupStyle();
    setWindowTitle("Search and Replace");
    setModal(true);
    resize(450, 350);
}

void SearchDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Mode selection
    QGroupBox* modeGroup = new QGroupBox("Mode");
    QHBoxLayout* modeLayout = new QHBoxLayout(modeGroup);
    
    searchModeRadio_ = new QRadioButton("Search");
    replaceModeRadio_ = new QRadioButton("Replace");
    searchModeRadio_->setChecked(true);
    
    modeLayout->addWidget(searchModeRadio_);
    modeLayout->addWidget(replaceModeRadio_);
    modeLayout->addStretch();
    
    mainLayout->addWidget(modeGroup);
    
    // Search text
    QGroupBox* searchGroup = new QGroupBox("Search");
    QVBoxLayout* searchLayout = new QVBoxLayout(searchGroup);
    
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("Enter search text...");
    searchLayout->addWidget(searchEdit_);
    
    // Search options
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    caseSensitiveCheck_ = new QCheckBox("Case sensitive");
    wholeWordCheck_ = new QCheckBox("Whole word");
    regexCheck_ = new QCheckBox("Regular expression");
    
    optionsLayout->addWidget(caseSensitiveCheck_);
    optionsLayout->addWidget(wholeWordCheck_);
    optionsLayout->addWidget(regexCheck_);
    optionsLayout->addStretch();
    
    searchLayout->addLayout(optionsLayout);
    mainLayout->addWidget(searchGroup);
    
    // Replace text (initially hidden)
    replaceGroup_ = new QGroupBox("Replace");
    QVBoxLayout* replaceLayout = new QVBoxLayout(replaceGroup_);
    
    replaceEdit_ = new QLineEdit();
    replaceEdit_->setPlaceholderText("Enter replacement text...");
    replaceLayout->addWidget(replaceEdit_);
    
    mainLayout->addWidget(replaceGroup_);
    replaceGroup_->setVisible(false);
    
    // Search scope
    QGroupBox* scopeGroup = new QGroupBox("Search in");
    QHBoxLayout* scopeLayout = new QHBoxLayout(scopeGroup);
    
    searchTreeCheck_ = new QCheckBox("Tree view");
    searchEditorCheck_ = new QCheckBox("Text editor");
    searchTreeCheck_->setChecked(true);
    searchEditorCheck_->setChecked(true);
    
    scopeLayout->addWidget(searchTreeCheck_);
    scopeLayout->addWidget(searchEditorCheck_);
    scopeLayout->addStretch();
    
    mainLayout->addWidget(scopeGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    findButton_ = new QPushButton("Find");
    replaceButton_ = new QPushButton("Replace");
    replaceAllButton_ = new QPushButton("Replace All");
    QPushButton* closeButton = new QPushButton("Close");
    
    findButton_->setDefault(true);
    replaceButton_->setEnabled(false);
    replaceAllButton_->setEnabled(false);
    
    buttonLayout->addWidget(findButton_);
    buttonLayout->addWidget(replaceButton_);
    buttonLayout->addWidget(replaceAllButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(searchModeRadio_, &QRadioButton::toggled, this, &SearchDialog::onSearchModeChanged);
    connect(replaceModeRadio_, &QRadioButton::toggled, this, &SearchDialog::onReplaceModeChanged);
    connect(searchEdit_, &QLineEdit::textChanged, this, &SearchDialog::onSearchTextChanged);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    
    connect(findButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(replaceButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(replaceAllButton_, &QPushButton::clicked, this, &QDialog::accept);
}

QString SearchDialog::getSearchText() const {
    return searchEdit_->text();
}

QString SearchDialog::getReplaceText() const {
    return replaceEdit_->text();
}

bool SearchDialog::isCaseSensitive() const {
    return caseSensitiveCheck_->isChecked();
}

bool SearchDialog::isWholeWord() const {
    return wholeWordCheck_->isChecked();
}

bool SearchDialog::isRegex() const {
    return regexCheck_->isChecked();
}

bool SearchDialog::searchInTree() const {
    return searchTreeCheck_->isChecked();
}

bool SearchDialog::searchInEditor() const {
    return searchEditorCheck_->isChecked();
}

bool SearchDialog::isReplaceMode() const {
    return replaceModeRadio_->isChecked();
}

void SearchDialog::onSearchModeChanged() {
    replaceGroup_->setVisible(false);
    findButton_->setText("Find");
    replaceButton_->setEnabled(false);
    replaceAllButton_->setEnabled(false);
}

void SearchDialog::onReplaceModeChanged() {
    replaceGroup_->setVisible(true);
    findButton_->setText("Find Next");
    replaceButton_->setEnabled(true);
    replaceAllButton_->setEnabled(true);
}

void SearchDialog::onSearchTextChanged() {
    bool hasText = !searchEdit_->text().isEmpty();
    findButton_->setEnabled(hasText);
    replaceButton_->setEnabled(hasText && isReplaceMode());
    replaceAllButton_->setEnabled(hasText && isReplaceMode());
}

void SearchDialog::setupStyle() {
    setStyleSheet(R"(
        QDialog {
            background-color: #1E1E1E;
            color: #D4D4D4;
        }
        
        QGroupBox {
            font-weight: 600;
            font-size: 13px;
            color: #CCCCCC;
            border: 1px solid #3E3E42;
            border-radius: 6px;
            margin-top: 8px;
            padding-top: 8px;
            font-family: 'Segoe UI', sans-serif;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px 0 8px;
            background-color: #1E1E1E;
        }
        
        QLineEdit {
            background-color: #2D2D30;
            border: 1px solid #3E3E42;
            color: #D4D4D4;
            padding: 8px 12px;
            border-radius: 6px;
            font-size: 13px;
            font-family: 'Cascadia Code', 'Consolas', monospace;
        }
        
        QLineEdit:focus {
            border-color: #007ACC;
            background-color: #2D2D30;
        }
        
        QLineEdit::placeholder {
            color: #6A6A6A;
        }
        
        QRadioButton {
            color: #CCCCCC;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
            spacing: 8px;
        }
        
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
        }
        
        QRadioButton::indicator::unchecked {
            border: 2px solid #3E3E42;
            border-radius: 8px;
            background-color: transparent;
        }
        
        QRadioButton::indicator::checked {
            border: 2px solid #007ACC;
            border-radius: 8px;
            background-color: #007ACC;
        }
        
        QCheckBox {
            color: #CCCCCC;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        
        QCheckBox::indicator::unchecked {
            border: 1px solid #3E3E42;
            border-radius: 3px;
            background-color: transparent;
        }
        
        QCheckBox::indicator::checked {
            border: 1px solid #007ACC;
            border-radius: 3px;
            background-color: #007ACC;
        }
        
        QPushButton {
            background-color: #0E639C;
            border: none;
            color: white;
            padding: 8px 16px;
            font-weight: 500;
            border-radius: 6px;
            font-size: 13px;
            font-family: 'Segoe UI', sans-serif;
            min-height: 28px;
            min-width: 80px;
        }
        
        QPushButton:hover {
            background-color: #1177BB;
        }
        
        QPushButton:pressed {
            background-color: #005A9E;
        }
        
        QPushButton:disabled {
            background-color: #3E3E42;
            color: #6A6A6A;
        }
        
        QPushButton[text="Close"] {
            background-color: #3E3E42;
        }
        
        QPushButton[text="Close"]:hover {
            background-color: #4F4F4F;
        }
    )");
} 