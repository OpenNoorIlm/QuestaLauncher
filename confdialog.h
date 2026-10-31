#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// confdialog.h — conf.svconf editor (Feature A)
// Owner: writer-a.  Ports the MultiHD Launcher VS-extension conf.svconf grammar
// (src/core/project.ts: parseConf / defaultConf / saveConf / defaultConfTemplate)
// into the Qt app.  Grammar + ordering rules are mirrored exactly.
// ─────────────────────────────────────────────────────────────────────────────
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QStringList>

// ─── conf.svconf parsed model (mirrors extension SvConf) ─────────────────────
struct SvConf
{
    QString     compileMode = "all";   // "all" | "list" | "N"
    QStringList priorityList;          // @priority list ...  (RTL order, parsed in order)
    QString     tbName;                // @tb (file.sv)  explicit testbench
    QStringList tbPriorityList;        // @tb priority list ...  (TB order)
    QString     topModule;             // @topmodule:  overrides detected top
    QString     vsimArgs;              // @vsimargs:   injected after `vsim`
    QString     vlogArgs;              // @vlogargs:   injected after `-work <lib>`
    QString     workLib = "work";      // @work:       replaces default `work` lib name
};

// ─── free helper functions (extension parity) ────────────────────────────────
inline QString confPathForProject(const QString &projectDir)
{
    return projectDir + "/conf.svconf";
}

// Parse `conf.svconf` (mirror extension parseConf grammar exactly).
SvConf parseConf(const QString &confPath);

// Serialise `conf.svconf` (mirror extension saveConf writer).
void saveConf(const QString &confPath, const SvConf &conf);

// Fresh-project conf.svconf template (mirror extension defaultConfTemplate).
QString defaultConfTemplate(const QString &openedBase, const QStringList &allBases);

// Qt naming rule: remove *every* ".sv" substring.
inline QString qtModuleName(const QString &fileName)
{
    QString s = fileName;
    s.replace(".sv", "");
    return s;
}

inline bool isTbFile(const QString &fileName)
{
    return fileName.endsWith("_tb.sv");
}

// ─── the editor dialog ───────────────────────────────────────────────────────
class ConfDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfDialog(const QString &projectDir, QWidget *parent = nullptr);

private slots:
    void createTemplate();      // overwrite file with defaultConfTemplate
    void saveConfFile();        // write file from current fields then accept

private:
    QString m_projectDir;

    // ── widgets ──
    QComboBox     *m_compileMode;   // all / list / N
    QLineEdit     *m_nCompile;      // the N when compile @(N)
    QLineEdit     *m_tbName;
    QTextEdit     *m_priorityList;  // one filename per line
    QTextEdit     *m_tbPriorityList;
    QLineEdit     *m_topModule;
    QLineEdit     *m_vsimArgs;
    QLineEdit     *m_vlogArgs;
    QLineEdit     *m_work;
    QLabel        *m_pathLabel;
    QPushButton   *m_templateBtn;
    QPushButton   *m_saveBtn;

    void loadFromFile();
    SvConf collectFromWidgets() const;
    void populateFromConf(const SvConf &conf);
};