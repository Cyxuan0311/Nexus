#include "search_dialog.h"
#include <QApplication>
#include <QStyle>

SearchDialog::SearchDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi();
    setWindowTitle("Search and Replace");
    setModal(true);
    resize(400, 300);
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