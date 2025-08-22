#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QTranslator>
#include <QApplication>
#include <QDir>

class Translator : public QObject {
    Q_OBJECT

public:
    static Translator& instance();
    
    // Load language
    bool loadLanguage(const QString& languageCode);
    
    // Get available languages
    QStringList getAvailableLanguages() const;
    
    // Get current language
    QString getCurrentLanguage() const;
    
    // Get language name
    QString getLanguageName(const QString& languageCode) const;
    
    // Install translator
    void installTranslator();
    
    // Remove translator
    void removeTranslator();

private:
    explicit Translator(QObject* parent = nullptr);
    ~Translator() = default;
    
    QTranslator translator_;
    QString currentLanguage_;
    QStringList availableLanguages_;
    
    void scanTranslations();
    QString getTranslationPath() const;
};

#endif // TRANSLATOR_H 