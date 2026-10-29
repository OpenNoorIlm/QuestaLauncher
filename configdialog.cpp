#include "configdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QStyle>

// ── shared palette ──────────────────────────────────────────────────────────
static const QString CFG_STYLE = R"(
    QDialog, QWidget { background-color: #0d0d1a; color: #e0e0e0; }
    QLineEdit {
        background-color: #131326; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 6px;
        padding: 5px 9px; font-family: 'Courier New', monospace; font-size: 11px;
    }
    QLineEdit:focus { border-color: #00ffcc; }
    QLineEdit[valid="false"] { border-color: #ff4444; color: #ff6666; }
    QLabel {
        font-family: 'Courier New', monospace; font-size: 11px; color: #8899bb;
    }
    QLabel#warning {
        color: #ff4444; font-size: 11px; font-weight: bold;
        font-family: 'Courier New', monospace;
        padding: 6px 10px;
        border: 1px solid #ff4444;
        border-radius: 6px;
        background-color: #1a0000;
    }
    QLabel#ok {
        color: #00ff88; font-size: 11px; font-weight: bold;
        font-family: 'Courier New', monospace;
        padding: 6px 10px;
        border: 1px solid #00ff88;
        border-radius: 6px;
        background-color: #001a0a;
    }
    QGroupBox {
        border: 1px solid #1a2a3a; border-radius: 6px;
        margin-top: 8px; padding-top: 6px;
        font-family: 'Courier New', monospace; font-size: 10px;
        color: #446688;
    }
    QGroupBox::title {
        subcontrol-origin: margin; subcontrol-position: top left;
        padding: 0 4px; left: 10px; color: #446688;
        font-family: 'Courier New', monospace; font-size: 10px; letter-spacing: 1px;
    }
    QPushButton#browseSmall {
        background-color: transparent; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 5px;
        padding: 4px 10px; font-family: 'Courier New', monospace; font-size: 11px;
        min-width: 64px;
    }
    QPushButton#browseSmall:hover { background-color: #00e5ff18; }
    QPushButton#saveBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0057a8,stop:1 #00b4d8);
        color: white; border: none; border-radius: 7px;
        font-size: 13px; font-weight: bold; letter-spacing: 1px;
        padding: 8px 0;
    }
    QPushButton#saveBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0070cc,stop:1 #00d4f0);
    }
    QPushButton#saveBtn:pressed { background: #003d7a; }
    QPushButton#cancelBtn {
        background-color: transparent; color: #556688;
        border: 1px solid #223344; border-radius: 7px;
        font-size: 13px; padding: 8px 0;
        font-family: 'Courier New', monospace;
    }
    QPushButton#cancelBtn:hover { color: #8899bb; border-color: #446688; }
)";

// ── helpers ─────────────────────────────────────────────────────────────────
static QHBoxLayout* makeRow(QLineEdit *edit, QPushButton *btn)
{
    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(6);
    row->addWidget(edit);
    row->addWidget(btn);
    return row;
}

// ── constructor ──────────────────────────────────────────────────────────────
ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("MultiHD Launcher — Configuration");
    setFixedSize(580, 560);   // taller to fit extra rows
    setStyleSheet(CFG_STYLE);
    setModal(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(18, 16, 18, 16);

    // ── Title ────────────────────────────────────────────────────────────────
    QLabel *title = new QLabel("⚙   CONFIGURATION");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:14px; font-weight:bold; color:#00e5ff;"
                         "font-family:'Courier New',monospace; letter-spacing:2px;");
    root->addWidget(title);

    QFrame *sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#1a2a3a;");
    root->addWidget(sep);

    // ── License warning banner ───────────────────────────────────────────────
    m_licenseWarning = new QLabel();
    m_licenseWarning->setObjectName("warning");
    m_licenseWarning->setAlignment(Qt::AlignCenter);
    m_licenseWarning->setWordWrap(true);
    root->addWidget(m_licenseWarning);

    // ── Paths group ──────────────────────────────────────────────────────────
    QGroupBox *pathsBox = new QGroupBox("PATHS");
    QGridLayout *grid = new QGridLayout(pathsBox);
    grid->setSpacing(6);
    grid->setContentsMargins(10, 8, 10, 10);
    grid->setColumnStretch(1, 1);

    auto makeLabel = [](const QString &t) {
        QLabel *l = new QLabel(t);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };

    m_vsimEdit   = new QLineEdit();
    m_vsimEdit->setPlaceholderText("/path/to/questa_fse/linux_x86_64/vsim");
    auto *vsimBrowse = new QPushButton("Browse"); vsimBrowse->setObjectName("browseSmall");

    m_licenseEdit = new QLineEdit();
    m_licenseEdit->setPlaceholderText("/path/to/questa_license.dat   ← REQUIRED");
    auto *licBrowse = new QPushButton("Browse"); licBrowse->setObjectName("browseSmall");

    m_folderEdit = new QLineEdit();
    m_folderEdit->setPlaceholderText("/home/user/projects/");
    auto *folBrowse = new QPushButton("Browse"); folBrowse->setObjectName("browseSmall");

    grid->addWidget(makeLabel("vsim binary"),  0, 0);
    grid->addWidget(m_vsimEdit,                0, 1);
    grid->addWidget(vsimBrowse,                0, 2);

    grid->addWidget(makeLabel("license file"), 1, 0);
    grid->addWidget(m_licenseEdit,             1, 1);
    grid->addWidget(licBrowse,                 1, 2);

    grid->addWidget(makeLabel("default folder"), 2, 0);
    grid->addWidget(m_folderEdit,                2, 1);
    grid->addWidget(folBrowse,                   2, 2);

    // ── Tool binaries (optional — used by Compile For) ────────────────────────
    m_quartusEdit = new QLineEdit();
    m_quartusEdit->setPlaceholderText("/path/to/quartus  (optional — for Compile For → Quartus)");
    auto *quartusBrowse = new QPushButton("Browse"); quartusBrowse->setObjectName("browseSmall");

    m_gowinEdit = new QLineEdit();
    m_gowinEdit->setPlaceholderText("/path/to/gw_ide  (optional — for Compile For → Gowin EDA)");
    auto *gowinBrowse = new QPushButton("Browse"); gowinBrowse->setObjectName("browseSmall");

    grid->addWidget(makeLabel("Quartus binary"), 3, 0);
    grid->addWidget(m_quartusEdit,               3, 1);
    grid->addWidget(quartusBrowse,               3, 2);

    grid->addWidget(makeLabel("Gowin IDE binary"), 4, 0);
    grid->addWidget(m_gowinEdit,                  4, 1);
    grid->addWidget(gowinBrowse,                  4, 2);

    root->addWidget(pathsBox);

    // ── Environment group ────────────────────────────────────────────────────
    QGroupBox *envBox = new QGroupBox("ENVIRONMENT VARIABLES");
    QGridLayout *envGrid = new QGridLayout(envBox);
    envGrid->setSpacing(6);
    envGrid->setContentsMargins(10, 8, 10, 10);
    envGrid->setColumnStretch(1, 1);

    m_saltEdit = new QLineEdit();
    m_saltEdit->setPlaceholderText("same as license file path, or server@host");

    m_mtiEdit  = new QLineEdit();
    m_mtiEdit->setPlaceholderText("64");

    envGrid->addWidget(makeLabel("SALT_LICENSE_SERVER"), 0, 0);
    envGrid->addWidget(m_saltEdit,                       0, 1);
    envGrid->addWidget(makeLabel("MTI_VCO_MODE"),        1, 0);
    envGrid->addWidget(m_mtiEdit,                        1, 1);

    // Informational note about env vars
    QLabel *envNote = new QLabel(
        "These are injected into Questa's process environment at launch.\n"
        "LM_LICENSE_FILE is set automatically from the license file path above.");
    envNote->setStyleSheet("color:#334455; font-size:10px; font-family:'Courier New',monospace;");
    envNote->setWordWrap(true);

    QVBoxLayout *envWrap = new QVBoxLayout();
    envWrap->setSpacing(6);
    envWrap->addWidget(envBox);
    envWrap->addWidget(envNote);
    root->addLayout(envWrap);

    root->addStretch();

    // ── Buttons ──────────────────────────────────────────────────────────────
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    auto *cancelBtn = new QPushButton("Cancel"); cancelBtn->setObjectName("cancelBtn");
    m_saveBtn = new QPushButton("✓   SAVE"); m_saveBtn->setObjectName("saveBtn");
    cancelBtn->setFixedHeight(38);
    m_saveBtn->setFixedHeight(38);
    btnRow->addWidget(cancelBtn, 1);
    btnRow->addWidget(m_saveBtn, 2);
    root->addLayout(btnRow);

    // ── Wire up ──────────────────────────────────────────────────────────────
    connect(vsimBrowse,    &QPushButton::clicked, this, &ConfigDialog::browseVsim);
    connect(licBrowse,     &QPushButton::clicked, this, &ConfigDialog::browseLicense);
    connect(folBrowse,     &QPushButton::clicked, this, &ConfigDialog::browseFolder);
    connect(quartusBrowse, &QPushButton::clicked, this, &ConfigDialog::browseQuartus);
    connect(gowinBrowse,   &QPushButton::clicked, this, &ConfigDialog::browseGowin);
    connect(cancelBtn,   &QPushButton::clicked, this, &QDialog::reject);
    connect(m_saveBtn,   &QPushButton::clicked, this, [this]{ saveSettings(); accept(); });

    // Re-validate whenever the license path changes
    connect(m_licenseEdit, &QLineEdit::textChanged, this, &ConfigDialog::validate);
    connect(m_vsimEdit,    &QLineEdit::textChanged, this, &ConfigDialog::validate);

    loadSettings();
    validate();
}

// ── accessors ────────────────────────────────────────────────────────────────
QString ConfigDialog::vsimPath()      const { return m_vsimEdit->text().trimmed(); }
QString ConfigDialog::licenseFile()   const { return m_licenseEdit->text().trimmed(); }
QString ConfigDialog::saltServer()    const { return m_saltEdit->text().trimmed(); }
QString ConfigDialog::mtiVcoMode()    const { return m_mtiEdit->text().trimmed(); }
QString ConfigDialog::defaultFolder() const { return m_folderEdit->text().trimmed(); }
QString ConfigDialog::quartusPath()   const { return m_quartusEdit->text().trimmed(); }
QString ConfigDialog::gowinPath()     const { return m_gowinEdit->text().trimmed(); }

// ── browse slots ─────────────────────────────────────────────────────────────
void ConfigDialog::browseVsim()
{
    QString f = QFileDialog::getOpenFileName(this, "Select vsim binary",
        m_vsimEdit->text().isEmpty() ? "/home" : m_vsimEdit->text());
    if (!f.isEmpty()) m_vsimEdit->setText(f);
}

void ConfigDialog::browseLicense()
{
    QString f = QFileDialog::getOpenFileName(this, "Select Questa license file",
        m_licenseEdit->text().isEmpty() ? QDir::homePath() : m_licenseEdit->text(),
        "License files (*.dat *.lic);;All files (*)");
    if (!f.isEmpty()) m_licenseEdit->setText(f);
}

void ConfigDialog::browseFolder()
{
    QString d = QFileDialog::getExistingDirectory(this, "Default project folder",
        m_folderEdit->text().isEmpty() ? QDir::homePath() : m_folderEdit->text());
    if (!d.isEmpty()) m_folderEdit->setText(d);
}

void ConfigDialog::browseQuartus()
{
    QString f = QFileDialog::getOpenFileName(this, "Select Quartus binary",
        m_quartusEdit->text().isEmpty() ? "/home" : m_quartusEdit->text());
    if (!f.isEmpty()) m_quartusEdit->setText(f);
}

void ConfigDialog::browseGowin()
{
    QString f = QFileDialog::getOpenFileName(this, "Select Gowin IDE binary",
        m_gowinEdit->text().isEmpty() ? "/home" : m_gowinEdit->text());
    if (!f.isEmpty()) m_gowinEdit->setText(f);
}

// ── validation ───────────────────────────────────────────────────────────────
bool ConfigDialog::licenseOk() const
{
    QString p = m_licenseEdit->text().trimmed();
    return !p.isEmpty() && QFile::exists(p);
}

void ConfigDialog::validate()
{
    bool licOk  = licenseOk();
    bool vsimOk = !m_vsimEdit->text().trimmed().isEmpty()
                  && QFile::exists(m_vsimEdit->text().trimmed());

    // License warning banner
    if (!licOk) {
        QString licPath = m_licenseEdit->text().trimmed();
        if (licPath.isEmpty()) {
            m_licenseWarning->setObjectName("warning");
            m_licenseWarning->setText(
                "⚠  LICENSE FILE NOT SET  —  Questa will not start without a valid license.\n"
                "Browse for your  questa_license.dat  file below.");
        } else {
            m_licenseWarning->setObjectName("warning");
            m_licenseWarning->setText(
                "⚠  LICENSE FILE NOT FOUND  at:  " + licPath + "\n"
                "Questa will refuse to start.  Check the path.");
        }
    } else {
        m_licenseWarning->setObjectName("ok");
        m_licenseWarning->setText("✓  License file found — Questa should start correctly.");
    }
    // Force QSS re-evaluation after objectName change
    m_licenseWarning->style()->unpolish(m_licenseWarning);
    m_licenseWarning->style()->polish(m_licenseWarning);

    // Highlight invalid fields
    m_licenseEdit->setProperty("valid", licOk  ? "true" : "false");
    m_vsimEdit->setProperty("valid",    vsimOk  ? "true" : "false");
    m_licenseEdit->style()->unpolish(m_licenseEdit); m_licenseEdit->style()->polish(m_licenseEdit);
    m_vsimEdit->style()->unpolish(m_vsimEdit);       m_vsimEdit->style()->polish(m_vsimEdit);
}

// ── persistence ──────────────────────────────────────────────────────────────
void ConfigDialog::loadSettings()
{
    QSettings s("MultiHDLauncher", "MultiHDLauncher");
    m_vsimEdit->setText(s.value("vsimPath",
        "/home/bismillah/altera/25.1std/questa_fse/linux_x86_64/vsim").toString());
    m_licenseEdit->setText(s.value("licenseFile",
        "/home/bismillah/.licenses/questa_license.dat").toString());
    m_saltEdit->setText(s.value("saltServer",
        "/home/bismillah/.licenses/questa_license.dat").toString());
    m_mtiEdit->setText(s.value("mtiVcoMode", "64").toString());
    m_folderEdit->setText(s.value("defaultFolder",
        "/home/bismillah/Downloads/SystemVerilogCourse/").toString());
    m_quartusEdit->setText(s.value("quartusPath", "").toString());
    m_gowinEdit->setText(s.value("gowinPath", "").toString());
}

void ConfigDialog::saveSettings()
{
    QSettings s("MultiHDLauncher", "MultiHDLauncher");
    s.setValue("vsimPath",      m_vsimEdit->text().trimmed());
    s.setValue("licenseFile",   m_licenseEdit->text().trimmed());
    s.setValue("saltServer",    m_saltEdit->text().trimmed());
    s.setValue("mtiVcoMode",    m_mtiEdit->text().trimmed());
    s.setValue("defaultFolder", m_folderEdit->text().trimmed());
    s.setValue("quartusPath",   m_quartusEdit->text().trimmed());
    s.setValue("gowinPath",     m_gowinEdit->text().trimmed());
}
