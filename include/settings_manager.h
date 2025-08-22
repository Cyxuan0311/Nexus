#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QColor>
#include <QFont>

class SettingsManager : public QObject {
    Q_OBJECT

public:
    static SettingsManager& instance();
    
    // Editor settings
    void setEditorFont(const QFont& font);
    QFont getEditorFont() const;
    
    void setEditorTabSize(int size);
    int getEditorTabSize() const;
    
    void setEditorShowLineNumbers(bool show);
    bool getEditorShowLineNumbers() const;
    
    void setEditorShowWhitespace(bool show);
    bool getEditorShowWhitespace() const;
    
    // Theme settings
    void setTheme(const QString& theme);
    QString getTheme() const;
    
    void setSyntaxHighlighting(bool enabled);
    bool getSyntaxHighlighting() const;
    
    // Window settings
    void setWindowGeometry(const QByteArray& geometry);
    QByteArray getWindowGeometry() const;
    
    void setSplitterSizes(const QList<int>& sizes);
    QList<int> getSplitterSizes() const;
    
    // Recent files
    void addRecentFile(const QString& filePath);
    QStringList getRecentFiles() const;
    void clearRecentFiles();
    
    // Export settings
    void setDefaultExportFormat(const QString& format);
    QString getDefaultExportFormat() const;
    
    void setExportIndentSize(int size);
    int getExportIndentSize() const;
    
    // Validation settings
    void setAutoValidate(bool enabled);
    bool getAutoValidate() const;
    
    void setShowValidationWarnings(bool show);
    bool getShowValidationWarnings() const;

private:
    explicit SettingsManager(QObject* parent = nullptr);
    ~SettingsManager() = default;
    
    QSettings settings_;
    
    // Default values
    void setDefaults();
};

#endif // SETTINGS_MANAGER_H 