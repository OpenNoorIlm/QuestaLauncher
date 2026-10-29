#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>

// ── Single registered library entry ──────────────────────────────────────────
struct LibEntry {
    QString  name;           // unique identifier, e.g. "uart_lib"
    QString  sourcePath;     // folder whose .sv files are compiled
    bool     compiled = false;
    QDateTime compiledAt;

    QJsonObject toJson()    const;
    static LibEntry fromJson(const QString &name, const QJsonObject &obj);
};

// ── Manager ───────────────────────────────────────────────────────────────────
// Singleton-style class (just construct one in MainWindow and pass around).
// All persistence is automatic on every mutating call.
class LibraryManager
{
public:
    explicit LibraryManager();

    // Global registry
    QList<LibEntry> libraries()              const { return m_libs; }
    bool            hasLibrary(const QString &name) const;
    LibEntry        library(const QString &name)    const;

    // Returns error string, empty = success
    QString addLibrary(const QString &name, const QString &sourcePath);
    QString removeLibrary(const QString &name);

    // Compile a library's source into its work db using vsim/vlog
    // vsimDir = directory containing the vlog binary
    QString compileLibrary(const QString &name, const QString &vsimBinaryPath);

    // Per-project questa_libs.json
    QStringList projectDeps(const QString &projectDir)   const;
    QString     setProjectDeps(const QString &projectDir,
                               const QStringList &deps);

    // Returns -L flags for vlog/vopt/vsim for the given dep list
    // e.g. { "-L", "uart_lib", "-L", "spi_lib" }
    QStringList linkFlags(const QStringList &deps) const;

    // Path where this library's vlib work db lives
    QString workDbPath(const QString &name) const;

    // Root dir for all compiled work dbs  (~/.questa_libs/work/)
    static QString globalWorkRoot();
    // Path to the registry JSON  (~/.questa_libs/registry.json)
    static QString registryPath();

private:
    QList<LibEntry> m_libs;

    void load();
    void save() const;
};
