#include "librarymanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QDateTime>

// ── paths ─────────────────────────────────────────────────────────────────────
QString LibraryManager::globalWorkRoot()
{
    return QDir::homePath() + "/.questa_libs/work";
}

QString LibraryManager::registryPath()
{
    return QDir::homePath() + "/.questa_libs/registry.json";
}

QString LibraryManager::workDbPath(const QString &name) const
{
    return globalWorkRoot() + "/" + name;
}

// ── LibEntry serialisation ────────────────────────────────────────────────────
QJsonObject LibEntry::toJson() const
{
    QJsonObject o;
    o["sourcePath"]  = sourcePath;
    o["compiled"]    = compiled;
    o["compiledAt"]  = compiledAt.toString(Qt::ISODate);
    return o;
}

LibEntry LibEntry::fromJson(const QString &n, const QJsonObject &obj)
{
    LibEntry e;
    e.name        = n;
    e.sourcePath  = obj["sourcePath"].toString();
    e.compiled    = obj["compiled"].toBool(false);
    e.compiledAt  = QDateTime::fromString(obj["compiledAt"].toString(), Qt::ISODate);
    return e;
}

// ── constructor ───────────────────────────────────────────────────────────────
LibraryManager::LibraryManager()
{
    // Ensure directories exist
    QDir().mkpath(globalWorkRoot());
    load();
}

// ── persistence ───────────────────────────────────────────────────────────────
void LibraryManager::load()
{
    m_libs.clear();
    QFile f(registryPath());
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it)
        m_libs << LibEntry::fromJson(it.key(), it.value().toObject());
}

void LibraryManager::save() const
{
    QJsonObject root;
    for (const LibEntry &e : m_libs)
        root[e.name] = e.toJson();

    QFile f(registryPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

// ── queries ───────────────────────────────────────────────────────────────────
bool LibraryManager::hasLibrary(const QString &name) const
{
    for (const LibEntry &e : m_libs)
        if (e.name == name) return true;
    return false;
}

LibEntry LibraryManager::library(const QString &name) const
{
    for (const LibEntry &e : m_libs)
        if (e.name == name) return e;
    return {};
}

// ── add ───────────────────────────────────────────────────────────────────────
QString LibraryManager::addLibrary(const QString &name, const QString &sourcePath)
{
    if (name.trimmed().isEmpty())
        return "Library name cannot be empty.";
    if (name.contains(' ') || name.contains('/') || name.contains('\\'))
        return "Library name must be a single word with no spaces or slashes.";
    if (hasLibrary(name))
        return QString("A library named \"%1\" already exists.").arg(name);
    if (!QDir(sourcePath).exists())
        return QString("Source path does not exist:\n%1").arg(sourcePath);

    QStringList svFiles = QDir(sourcePath).entryList({"*.sv"}, QDir::Files);
    if (svFiles.isEmpty())
        return QString("No .sv files found in:\n%1").arg(sourcePath);

    LibEntry e;
    e.name       = name;
    e.sourcePath = sourcePath;
    e.compiled   = false;
    m_libs << e;
    save();
    return {};
}

// ── remove ────────────────────────────────────────────────────────────────────
QString LibraryManager::removeLibrary(const QString &name)
{
    for (int i = 0; i < m_libs.size(); ++i) {
        if (m_libs[i].name == name) {
            m_libs.removeAt(i);
            save();
            // Also nuke the compiled work db if it exists
            QString db = workDbPath(name);
            if (QDir(db).exists())
                QDir(db).removeRecursively();
            return {};
        }
    }
    return QString("Library \"%1\" not found.").arg(name);
}

// ── compile ───────────────────────────────────────────────────────────────────
QString LibraryManager::compileLibrary(const QString &name, const QString &vsimBinaryPath)
{
    if (!hasLibrary(name))
        return QString("Library \"%1\" not registered.").arg(name);

    LibEntry &e = m_libs[0];   // find proper entry below
    for (auto &entry : m_libs)
        if (entry.name == name) { e = entry; break; }

    QFileInfo vsimInfo(vsimBinaryPath);
    QString binDir = vsimInfo.absolutePath();
    QString vlib   = binDir + "/vlib";
    QString vlog   = binDir + "/vlog";

    if (!QFile::exists(vlog))
        return QString("vlog not found at:\n%1\nCheck the vsim path in Configuration.").arg(vlog);

    // Collect .sv source files
    QStringList svFiles = QDir(e.sourcePath).entryList({"*.sv"}, QDir::Files);
    if (svFiles.isEmpty())
        return QString("No .sv files found in:\n%1").arg(e.sourcePath);

    QStringList svPaths;
    for (const QString &f : svFiles)
        svPaths << (e.sourcePath + "/" + f);

    QString dbPath = workDbPath(name);

    // Step 1 — vlib
    if (!QDir(dbPath).exists()) {
        QProcess vlibProc;
        vlibProc.setProgram(vlib);
        vlibProc.setArguments({dbPath});
        vlibProc.start();
        vlibProc.waitForFinished(10000);
        if (vlibProc.exitCode() != 0)
            return QString("vlib failed:\n%1")
                   .arg(QString::fromLocal8Bit(vlibProc.readAllStandardError()));
    }

    // Step 2 — vlog
    QStringList vlogArgs = {"-work", dbPath};
    vlogArgs << svPaths;

    QProcess vlogProc;
    vlogProc.setProgram(vlog);
    vlogProc.setArguments(vlogArgs);
    vlogProc.start();
    vlogProc.waitForFinished(60000);

    if (vlogProc.exitCode() != 0)
        return QString("vlog compilation failed:\n%1")
               .arg(QString::fromLocal8Bit(vlogProc.readAllStandardError()));

    // Mark compiled
    for (auto &entry : m_libs) {
        if (entry.name == name) {
            entry.compiled   = true;
            entry.compiledAt = QDateTime::currentDateTime();
            break;
        }
    }
    save();
    return {};
}

// ── per-project deps ──────────────────────────────────────────────────────────
static QString depsFile(const QString &projectDir)
{
    return projectDir + "/questa_libs.json";
}

QStringList LibraryManager::projectDeps(const QString &projectDir) const
{
    QFile f(depsFile(projectDir));
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return {};

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return {};

    QJsonArray arr = doc.object()["dependencies"].toArray();
    QStringList deps;
    for (const QJsonValue &v : arr)
        deps << v.toString();
    return deps;
}

QString LibraryManager::setProjectDeps(const QString &projectDir,
                                        const QStringList &deps)
{
    QJsonObject root;
    QJsonArray arr;
    for (const QString &d : deps) arr << d;
    root["dependencies"] = arr;

    QFile f(depsFile(projectDir));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return QString("Cannot write %1").arg(depsFile(projectDir));
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return {};
}

// ── link flags ────────────────────────────────────────────────────────────────
QStringList LibraryManager::linkFlags(const QStringList &deps) const
{
    QStringList flags;
    for (const QString &dep : deps) {
        if (!hasLibrary(dep)) continue;
        flags << "-L" << workDbPath(dep);
    }
    return flags;
}
