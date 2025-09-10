#ifndef SEARCH_DIALOG_H
#define SEARCH_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

class SearchDialog : public QDialog {
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    
    QString getSearchText() const;
    QString getReplaceText() const;
    bool isCaseSensitive() const;
    bool isWholeWord() const;
    bool isRegex() const;
    bool searchInTree() const;
    bool searchInEditor() const;
    bool isReplaceMode() const;

private slots:
    void onSearchModeChanged();
    void onReplaceModeChanged();
    void onSearchTextChanged();

private:
    void setupUi();
    
    QLineEdit* searchEdit_;
    QLineEdit* replaceEdit_;
    QPushButton* findButton_;
    QPushButton* replaceButton_;
    QPushButton* replaceAllButton_;
    QCheckBox* caseSensitiveCheck_;
    QCheckBox* wholeWordCheck_;
    QCheckBox* regexCheck_;
    QCheckBox* searchTreeCheck_;
    QCheckBox* searchEditorCheck_;
    QRadioButton* searchModeRadio_;
    QRadioButton* replaceModeRadio_;
    QGroupBox* replaceGroup_;
};

#endif // SEARCH_DIALOG_H 