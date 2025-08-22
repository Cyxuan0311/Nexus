#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <QObject>
#include <QPluginLoader>
#include <QDir>
#include <QMap>
#include <memory>

class XmlPlugin {
public:
    virtual ~XmlPlugin() = default;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString description() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

Q_DECLARE_INTERFACE(XmlPlugin, "com.cxml.plugin")

class PluginManager : public QObject {
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();
    
    // Load plugins from directory
    bool loadPlugins(const QString& pluginDir);
    
    // Get loaded plugins
    QList<XmlPlugin*> getPlugins() const;
    
    // Get plugin by name
    XmlPlugin* getPlugin(const QString& name) const;
    
    // Enable/disable plugin
    bool enablePlugin(const QString& name);
    bool disablePlugin(const QString& name);
    
    // Plugin information
    QStringList getPluginNames() const;
    bool isPluginEnabled(const QString& name) const;

private:
    QMap<QString, QPluginLoader*> loaders_;
    QMap<QString, XmlPlugin*> plugins_;
    QMap<QString, bool> enabledPlugins_;
    
    void loadPlugin(const QString& filePath);
    void unloadPlugin(const QString& name);
};

#endif // PLUGIN_MANAGER_H 