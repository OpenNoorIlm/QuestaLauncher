#include "librarydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QFrame>
#include <QStyle>
#include <QApplication>

static const QString LIB_STYLE = R"(
    QDialog, QWidget { background-color: #0d0d1a; color: #e0e0e0; }

    QListWidget {
        background-color: #080812; color: #e0e0e0;
        border: 1px solid #1a1a3a; border-radius: 6px;
        font-family: 'Courier New', monospace; font-size: 11px;
        padding: 4px;
    }
    QListWidget::item { padding: 5px 6px; border-radius: 3px; }
    QListWidget::item:hover    { background-color: #1a1a3a; }
    QListWidget::item:selected { background-color: #1a2a3a; color: #00e5ff; }

    QGroupBox {
        border: 1px solid #1a2a3a; border-radius: 6px;
        margin-top: 8px; padding-top: 6px;
        font-family: 'Courier New', monospace; font-size: 10px; color: #446688;
    }
    QGroupBox#projectBox { border-color: #1a3a1a; }
    QGroupBox::title {
        subcontrol-origin: margin; subcontrol-position: top left;
        padding: 0 4px; left: 10px; color: #446688;
        font-family: 'Courier New', monospace; font-size: 10px; letter-spacing: 1px;
    }

    QLabel#title {
        color: #00e5ff; font-size: 14px; font-weight: bold;
        font-family: 'Courier New', monospace; letter-spacing: 2px;
    }
    QLabel#sectionHint {
        color: #334455; font-size: 10px;
        font-family: 'Courier New', monospace;
    }
    QLabel#statusOk  { color: #00ff88; font-family: 'Courier New', monospace; font-size: 11px; padding: 2px 6px; }
    QLabel#statusErr { color: #ff4444; font-family: 'Courier New', monospace; font-size: 11px; padding: 2px 6px; }

    QPushButton#addBtn {
        background-color: transparent; color: #00ff88;
        border: 1px solid #00ff88; border-radius: 5px;
        font-family: 'Courier New', monospace; font-size: 11px; padding: 5px 12px;
    }
    QPushButton#addBtn:hover { background-color: #00ff8818; }

    QPushButton#removeBtn {
        background-color: transparent; color: #ff6666;
        border: 1px solid #ff4444; border-radius: 5px;
        font-family: 'Courier New', monospace; font-size: 11px; padding: 5px 12px;
    }
    QPushButton#removeBtn:hover { background-color: #ff444418; }

    QPushButton#compileBtn {
        background-color: transparent; color: #ffaa00;
        border: 1px solid #ffaa00; border-radius: 5px;
        font-family: 'Courier New', monospace; font-size: 11px; padding: 5px 12px;
    }
    QPushButton#compileBtn:hover { background-color: #ffaa0018; }

    QPushButton#closeBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0057a8,stop:1 #00b4d8);
        color: white; border: none; border-radius: 7px;
        font-size: 13px; font-weight: bold; padding: 8px 0;
        font-family: 'Courier New', monospace;
    }
    QPushButton#closeBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0070cc,stop:1 #00d4f0);
    }

    QCheckBox {
        color: #00e5ff; font-family: 'Courier New', monospace; font-size: 11px;
    }
    QCheckBox::indicator {
        width: 13px; height: 13px;
        border: 1px solid #00e5ff; border-radius: 3px;
        background-color: #0d0d1a;
    }
    QCheckBox::indicator:checked { background-color: #0057a8; }
)";

// ── helpers ───────────────────────────────────────────────────────────────────
static QWidget* makeSep()
{
    auto *f = new QFrame();
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color:#1a2a3a;");
    return f;
}

// ── constructor ───────────────────────────────────────────────────────────────
LibraryDialog::LibraryDialog(LibraryManager *mgr,
                             const QString  &projectDir,
                             const QString  &vsimBinaryPath,
                             QWidget        *parent)
    : QDialog(parent), m_mgr(mgr), m_projectDir(projectDir), m_vsimBin(vsimBinaryPath)
{
    setWindowTitle("Library Manager");
    setMinimumSize(620, 520);
    setStyleSheet(LIB_STYLE);
    setModal(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(18, 14, 18, 14);

    // Title
    auto *title = new QLabel("📦   LIBRARY MANAGER");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);
    root->addWidget(makeSep());

    // ── Global registry ───────────────────────────────────────────────────────
    QGroupBox *globalBox = new QGroupBox("GLOBAL REGISTRY   (~/.questa_libs/)");
    QVBoxLayout *gl = new QVBoxLayout(globalBox);
    gl->setContentsMargins(10, 8, 10, 10);
    gl->setSpacing(6);

    auto *hint1 = new QLabel("Libraries compiled here are available to any project.");
    hint1->setObjectName("sectionHint");
    gl->addWidget(hint1);

    m_globalList = new QListWidget();
    m_globalList->setMinimumHeight(150);
    gl->addWidget(m_globalList);

    QHBoxLayout *globalBtns = new QHBoxLayout();
    globalBtns->setSpacing(8);
    m_addBtn     = new QPushButton("＋  ADD LIBRARY");   m_addBtn->setObjectName("addBtn");
    m_removeBtn  = new QPushButton("✕  REMOVE");         m_removeBtn->setObjectName("removeBtn");
    m_compileBtn = new QPushButton("⚡  COMPILE");        m_compileBtn->setObjectName("compileBtn");
    m_addBtn->setFixedHeight(30);
    m_removeBtn->setFixedHeight(30);
    m_compileBtn->setFixedHeight(30);
    globalBtns->addWidget(m_addBtn);
    globalBtns->addStretch();
    globalBtns->addWidget(m_compileBtn);
    globalBtns->addWidget(m_removeBtn);
    gl->addLayout(globalBtns);
    root->addWidget(globalBox);

    // ── Per-project panel ─────────────────────────────────────────────────────
    QGroupBox *projectBox = new QGroupBox("THIS PROJECT'S DEPENDENCIES   (questa_libs.json)");
    projectBox->setObjectName("projectBox");
    QVBoxLayout *pl = new QVBoxLayout(projectBox);
    pl->setContentsMargins(10, 8, 10, 10);
    pl->setSpacing(6);

    if (m_projectDir.isEmpty()) {
        m_projectNote = new QLabel("No project folder selected — open a project first.");
        m_projectNote->setObjectName("sectionHint");
        pl->addWidget(m_projectNote);
        m_projectList = new QListWidget();   // hidden but created to avoid null checks
        m_projectList->hide();
    } else {
        auto *hint2 = new QLabel(
            "Tick a library to add it to this project.\n"
            "Checked libraries are compiled before your project and linked with -L flags.");
        hint2->setObjectName("sectionHint");
        hint2->setWordWrap(true);
        pl->addWidget(hint2);

        m_projectList = new QListWidget();
        m_projectList->setMinimumHeight(110);
        pl->addWidget(m_projectList);

        m_projectNote = new QLabel(
            QString("Saved to: %1/questa_libs.json").arg(m_projectDir));
        m_projectNote->setObjectName("sectionHint");
        pl->addWidget(m_projectNote);
    }
    root->addWidget(projectBox);

    // ── Status bar ────────────────────────────────────────────────────────────
    m_statusLabel = new QLabel(" ");
    m_statusLabel->setObjectName("statusOk");
    root->addWidget(m_statusLabel);

    // ── Close ─────────────────────────────────────────────────────────────────
    auto *closeBtn = new QPushButton("✓   DONE");
    closeBtn->setObjectName("closeBtn");
    closeBtn->setFixedHeight(38);
    root->addWidget(closeBtn);

    // Connections
    connect(m_addBtn,     &QPushButton::clicked, this, &LibraryDialog::addLibrary);
    connect(m_removeBtn,  &QPushButton::clicked, this, &LibraryDialog::removeLibrary);
    connect(m_compileBtn, &QPushButton::clicked, this, &LibraryDialog::compileLibrary);
    connect(closeBtn,     &QPushButton::clicked, this, &QDialog::accept);

    if (!m_projectDir.isEmpty())
        connect(m_projectList, &QListWidget::itemChanged,
                this, &LibraryDialog::toggleProjectDep);

    refreshGlobalList();
    if (!m_projectDir.isEmpty()) refreshProjectList();
}

// ── status helper ─────────────────────────────────────────────────────────────
void LibraryDialog::status(const QString &msg, bool error)
{
    m_statusLabel->setObjectName(error ? "statusErr" : "statusOk");
    m_statusLabel->setText(msg);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
}

// ── refresh global list ───────────────────────────────────────────────────────
void LibraryDialog::refreshGlobalList()
{
    m_globalList->blockSignals(true);
    m_globalList->clear();
    for (const LibEntry &e : m_mgr->libraries()) {
        QString badge   = e.compiled ? "✓" : "○";
        QString when    = e.compiled
                          ? e.compiledAt.toString("yyyy-MM-dd hh:mm")
                          : "not compiled";
        QString display = QString("%1  %2    %3    [%4]")
                          .arg(badge).arg(e.name, -20).arg(e.sourcePath).arg(when);
        auto *item = new QListWidgetItem(display, m_globalList);
        item->setData(Qt::UserRole, e.name);
        item->setForeground(e.compiled ? QColor(0x00, 0xff, 0x88) : QColor(0x88, 0x88, 0x88));
    }
    if (m_mgr->libraries().isEmpty()) {
        auto *placeholder = new QListWidgetItem("  No libraries registered yet. Click ＋ ADD LIBRARY.");
        placeholder->setFlags(Qt::NoItemFlags);
        placeholder->setForeground(QColor(0x33, 0x44, 0x55));
        m_globalList->addItem(placeholder);
    }
    m_globalList->blockSignals(false);
}

// ── refresh project list (checkboxes) ────────────────────────────────────────
void LibraryDialog::refreshProjectList()
{
    m_projectList->blockSignals(true);
    m_projectList->clear();
    QStringList current = m_mgr->projectDeps(m_projectDir);

    for (const LibEntry &e : m_mgr->libraries()) {
        QString badge   = e.compiled ? "✓" : "⚠ not compiled";
        QString display = QString("%1  %2    %3").arg(badge).arg(e.name, -20).arg(e.sourcePath);
        auto *item = new QListWidgetItem(display, m_projectList);
        item->setData(Qt::UserRole, e.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(current.contains(e.name) ? Qt::Checked : Qt::Unchecked);
        item->setForeground(e.compiled ? QColor(0xe0, 0xe0, 0xe0) : QColor(0xff, 0xaa, 0x00));
    }

    if (m_mgr->libraries().isEmpty()) {
        auto *ph = new QListWidgetItem("  No global libraries yet — add some above.");
        ph->setFlags(Qt::NoItemFlags);
        ph->setForeground(QColor(0x33, 0x44, 0x55));
        m_projectList->addItem(ph);
    }
    m_projectList->blockSignals(false);
}

// ── add library ───────────────────────────────────────────────────────────────
void LibraryDialog::addLibrary()
{
    // Pick folder first
    QString path = QFileDialog::getExistingDirectory(
        this, "Select Library Source Folder", QDir::homePath());
    if (path.isEmpty()) return;

    // Then ask for a name (default = folder name)
    QString defaultName = QDir(path).dirName().toLower().replace(' ', '_');
    bool ok;
    QString name = QInputDialog::getText(
        this, "Library Name",
        "Enter a unique name for this library\n(single word, no spaces):",
        QLineEdit::Normal, defaultName, &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    QString err = m_mgr->addLibrary(name.trimmed(), path);
    if (!err.isEmpty()) {
        status("✕  " + err, true);
        return;
    }

    status(QString("✓  Library \"%1\" registered. Compile it to use it.").arg(name));
    refreshGlobalList();
    if (!m_projectDir.isEmpty()) refreshProjectList();
}

// ── remove library ────────────────────────────────────────────────────────────
void LibraryDialog::removeLibrary()
{
    auto *item = m_globalList->currentItem();
    if (!item || !item->data(Qt::UserRole).isValid()) {
        status("✕  Select a library to remove.", true);
        return;
    }
    QString name = item->data(Qt::UserRole).toString();

    int r = QMessageBox::question(this, "Remove Library",
        QString("Remove library \"%1\"?\n\nThis will delete its compiled work database.").arg(name),
        QMessageBox::Yes | QMessageBox::Cancel);
    if (r != QMessageBox::Yes) return;

    QString err = m_mgr->removeLibrary(name);
    if (!err.isEmpty()) { status("✕  " + err, true); return; }

    status(QString("✓  Library \"%1\" removed.").arg(name));
    refreshGlobalList();
    if (!m_projectDir.isEmpty()) refreshProjectList();
}

// ── compile library ───────────────────────────────────────────────────────────
void LibraryDialog::compileLibrary()
{
    auto *item = m_globalList->currentItem();
    if (!item || !item->data(Qt::UserRole).isValid()) {
        status("✕  Select a library to compile.", true);
        return;
    }
    QString name = item->data(Qt::UserRole).toString();

    if (m_vsimBin.isEmpty() || !QFile::exists(m_vsimBin)) {
        status("✕  vsim path not configured. Open ⚙ CONFIG first.", true);
        return;
    }

    status(QString("⚡  Compiling \"%1\" …").arg(name));
    QApplication::processEvents();   // flush UI before blocking call

    QString err = m_mgr->compileLibrary(name, m_vsimBin);
    if (!err.isEmpty()) { status("✕  " + err, true); return; }

    status(QString("✓  \"%1\" compiled successfully.").arg(name));
    refreshGlobalList();
    if (!m_projectDir.isEmpty()) refreshProjectList();
}

// ── toggle project dep (checkbox changed) ────────────────────────────────────
void LibraryDialog::toggleProjectDep(QListWidgetItem *item)
{
    if (!item->data(Qt::UserRole).isValid()) return;

    QStringList deps;
    for (int i = 0; i < m_projectList->count(); ++i) {
        auto *it = m_projectList->item(i);
        if (it->data(Qt::UserRole).isValid() && it->checkState() == Qt::Checked)
            deps << it->data(Qt::UserRole).toString();
    }

    QString err = m_mgr->setProjectDeps(m_projectDir, deps);
    if (!err.isEmpty()) { status("✕  " + err, true); return; }

    QString name = item->data(Qt::UserRole).toString();
    bool checked = (item->checkState() == Qt::Checked);

    // Warn if the library isn't compiled yet
    if (checked && m_mgr->hasLibrary(name) && !m_mgr->library(name).compiled) {
        status(QString("⚠  \"%1\" added but not compiled — compile it before launching.").arg(name));
    } else {
        status(checked
               ? QString("✓  \"%1\" added to project deps.").arg(name)
               : QString("✓  \"%1\" removed from project deps.").arg(name));
    }
}
