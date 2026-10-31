#include "mainwindow.h"
#include "prioritydialog.h"
#include "configdialog.h"
#include "excludedialog.h"
#include "librarydialog.h"
#include "gowincstdialog.h"
// ── writer-a: conf.svconf (Feature A) ────────────────────────────────────────
#include "confdialog.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QProcess>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QSvgRenderer>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QDesktopServices>
#include <QUrl>
#include <QGroupBox>
#include <QSettings>
#include <QScrollArea>
#include <QLabel>
#include <QCheckBox>
// ── writer-b: launch-mode selector needs QComboBox ───────────────────────────
#include <QComboBox>
#include <QDateTime>

// ─────────────────────────────────────────────────────────────────────────────
static const QString STYLE = R"(
    QMainWindow, QWidget { background-color: #0d0d1a; color: #e0e0e0; }
    QLineEdit {
        background-color: #131326; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 6px;
        padding: 6px 10px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#browseBtn {
        background-color: transparent; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#browseBtn:hover { background-color: #00e5ff22; }
    QPushButton#priorityBtn {
        background-color: transparent; color: #ffaa00;
        border: 1px solid #ffaa00; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#priorityBtn:hover { background-color: #ffaa0022; }
    QPushButton#configBtn {
        background-color: transparent; color: #aaaaff;
        border: 1px solid #aaaaff; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#configBtn:hover { background-color: #aaaaff18; }
    // ── writer-a: conf.svconf button (dark-cyber green) ─────────────────────
    QPushButton#confBtn {
        background-color: transparent; color: #00ff88;
        border: 1px solid #00ff88; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#confBtn:hover { background-color: #00ff8818; }
    QPushButton#excludeBtn {
        background-color: transparent; color: #cc6666;
        border: 1px solid #cc4444; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#excludeBtn:hover { background-color: #cc444418; }
    QPushButton#libBtn {
        background-color: transparent; color: #88ddff;
        border: 1px solid #44aacc; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#libBtn:hover { background-color: #44aacc18; }
    QPushButton#launchBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0057a8,stop:1 #00b4d8);
        color: white; border: none; border-radius: 8px;
        font-size: 15px; font-weight: bold; letter-spacing: 1px;
    }
    QPushButton#launchBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0070cc,stop:1 #00d4f0);
    }
    QPushButton#launchBtn:pressed { background: #003d7a; }
    QPushButton#launchBtnWarn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7a0000,stop:1 #cc2200);
        color: white; border: none; border-radius: 8px;
        font-size: 14px; font-weight: bold; letter-spacing: 1px;
    }
    QPushButton#launchBtnWarn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #990000,stop:1 #ee3300);
    }
    QPushButton#installBtn {
        background-color: transparent; color: #00ff88;
        border: 1px solid #00ff88; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#installBtn:hover { background-color: #00ff8822; }
    // ── writer-c: About button (Feature C) — cyan, like installBtn ─────────────
    QPushButton#aboutBtn {
        background-color: transparent; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#aboutBtn:hover { background-color: #00e5ff22; }
    QPushButton#licenseBtn {
        background-color: transparent; color: #bb88ff;
        border: 1px solid #bb88ff; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#licenseBtn:hover { background-color: #bb88ff22; }
    QPushButton#restoreBtn {
        background-color: #1a0000; color: #ff6666;
        border: 1px solid #552222; border-radius: 4px;
        font-family: 'Courier New', monospace; font-size: 10px;
        padding: 2px 7px;
    }
    QPushButton#restoreBtn:hover { background-color: #330000; border-color: #cc4444; color: #ffaaaa; }
    QTextEdit {
        background-color: #080812; color: #00e5ff;
        border: 1px solid #1a1a3a; border-radius: 6px;
        font-family: 'Courier New', monospace; font-size: 11px; padding: 6px;
    }
    QComboBox#launchMode {
        background-color: #131326; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 6px;
        padding: 4px 10px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QComboBox#launchMode::drop-down {
        subcontrol-origin: padding; subcontrol-position: top right;
        width: 22px; border-left: 1px solid #1a1a3a; border-radius: 0 6px 6px 0;
    }
    QComboBox#launchMode QAbstractItemView {
        background-color: #0d0d1a; color: #00e5ff;
        border: 1px solid #00e5ff; selection-background-color: #004466;
        font-family: 'Courier New', monospace; font-size: 12px;
    }
    QLabel#subtitle {
        color: #446688; font-size: 10px;
        font-family: 'Courier New', monospace; letter-spacing: 2px;
    }
    QLabel#licOk {
        color: #00ff88; font-size: 10px; font-weight: bold;
        font-family: 'Courier New', monospace;
        padding: 3px 8px; border: 1px solid #00ff88; border-radius: 4px;
        background-color: #001a0a;
    }
    QLabel#licBad {
        color: #ff4444; font-size: 10px; font-weight: bold;
        font-family: 'Courier New', monospace;
        padding: 3px 8px; border: 1px solid #ff4444; border-radius: 4px;
        background-color: #1a0000;
    }
    QLabel#chipLabel {
        color: #ff6666; font-family: 'Courier New', monospace; font-size: 10px;
    }
    QLabel#noExclLabel {
        color: #223344; font-family: 'Courier New', monospace; font-size: 10px;
        padding: 2px 4px;
    }
    QGroupBox {
        border: 1px solid #1a1a3a; border-radius: 6px;
        margin-top: 6px; padding-top: 4px;
        font-family: 'Courier New', monospace; font-size: 10px;
        color: #446688;
    }
    QGroupBox#excludedBox { border: 1px solid #331111; }
    QGroupBox::title {
        subcontrol-origin: margin; subcontrol-position: top left;
        padding: 0 4px; left: 8px; color: #446688;
        font-family: 'Courier New', monospace; font-size: 10px; letter-spacing: 1px;
    }
    QScrollArea { border: none; background: transparent; }
    QScrollBar:vertical {
        background: #0d0d1a; width: 6px; border-radius: 3px;
    }
    QScrollBar::handle:vertical { background: #223344; border-radius: 3px; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

    /* ── Compile For checkboxes ── */
    QCheckBox {
        font-family: 'Courier New', monospace; font-size: 11px; spacing: 7px;
    }
    QCheckBox::indicator {
        width: 14px; height: 14px; border-radius: 3px; background-color: #0d0d1a;
    }
    QCheckBox#chkQuesta            { color: #00e5ff; }
    QCheckBox#chkQuesta::indicator { border: 1px solid #00e5ff; }
    QCheckBox#chkQuesta::indicator:checked { background-color: #0057a8; border-color: #00e5ff; }

    QCheckBox#chkQuartus               { color: #ffaa00; }
    QCheckBox#chkQuartus::indicator    { border: 1px solid #ffaa00; }
    QCheckBox#chkQuartus::indicator:checked { background-color: #7a4400; border-color: #ffaa00; }

    QCheckBox#chkGowin                 { color: #88ff88; }
    QCheckBox#chkGowin::indicator      { border: 1px solid #44aa44; }
    QCheckBox#chkGowin::indicator:checked { background-color: #1a4a1a; border-color: #88ff88; }

    QGroupBox#compileForBox { border-color: #1a2a1a; }
)";

// ── QSettings helpers  (org: MultiHDLauncher) ────────────────────────────────
QString MainWindow::settingVsim() const {
    return QSettings("MultiHDLauncher","MultiHDLauncher")
        .value("vsimPath", "/home/bismillah/altera/25.1std/questa_fse/linux_x86_64/vsim").toString();
}
QString MainWindow::settingLicenseFile() const {
    return QSettings("MultiHDLauncher","MultiHDLauncher")
        .value("licenseFile", "/home/bismillah/.licenses/questa_license.dat").toString();
}
QString MainWindow::settingSaltServer() const {
    return QSettings("MultiHDLauncher","MultiHDLauncher")
        .value("saltServer", "/home/bismillah/.licenses/questa_license.dat").toString();
}
QString MainWindow::settingMtiVcoMode() const {
    return QSettings("MultiHDLauncher","MultiHDLauncher")
        .value("mtiVcoMode", "64").toString();
}
QString MainWindow::settingDefaultFolder() const {
    return QSettings("MultiHDLauncher","MultiHDLauncher")
        .value("defaultFolder", "/home/bismillah/Downloads/SystemVerilogCourse/").toString();
}

// ── license status badge ──────────────────────────────────────────────────────
void MainWindow::updateLicenseStatus()
{
    bool ok = !settingLicenseFile().isEmpty() && QFile::exists(settingLicenseFile());
    if (ok) {
        m_licenseStatusLabel->setObjectName("licOk");
        m_licenseStatusLabel->setText("✓ LICENSE OK");
    } else {
        m_licenseStatusLabel->setObjectName("licBad");
        m_licenseStatusLabel->setText("⚠ LICENSE NOT SET — configure before launching Questa!");
    }
    m_licenseStatusLabel->style()->unpolish(m_licenseStatusLabel);
    m_licenseStatusLabel->style()->polish(m_licenseStatusLabel);

    if (ok) {
        m_launchBtn->setObjectName("launchBtn");
        m_launchBtn->setText("▶   LAUNCH");
    } else {
        m_launchBtn->setObjectName("launchBtnWarn");
        m_launchBtn->setText("⚠   LAUNCH  (Questa: no license!)");
    }
    m_launchBtn->style()->unpolish(m_launchBtn);
    m_launchBtn->style()->polish(m_launchBtn);
}

// ── activeFiles: all .sv in dir, minus excluded ───────────────────────────────
QStringList MainWindow::activeFiles(const QString &dir) const
{
    QDir d(dir);
    QStringList all = d.entryList(QStringList() << "*.sv", QDir::Files);
    QStringList result;
    for (const QString &f : all)
        if (!m_excludedFiles.contains(f))
            result << f;
    return result;
}

// ── chip panel ────────────────────────────────────────────────────────────────
void MainWindow::rebuildExcludedChips()
{
    QLayoutItem *item;
    while ((item = m_chipLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    if (m_excludedFiles.isEmpty()) {
        auto *lbl = new QLabel("no files excluded");
        lbl->setObjectName("noExclLabel");
        m_chipLayout->addWidget(lbl);
        m_excludedBox->setTitle("EXCLUDED FILES");
    } else {
        for (const QString &f : m_excludedFiles) {
            QHBoxLayout *row = new QHBoxLayout();
            row->setSpacing(6);
            row->setContentsMargins(0, 0, 0, 0);

            auto *lbl = new QLabel(f);
            lbl->setObjectName("chipLabel");

            auto *btn = new QPushButton("↩ restore");
            btn->setObjectName("restoreBtn");
            btn->setFixedHeight(20);
            btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QString fname = f;
            connect(btn, &QPushButton::clicked, this, [this, fname](){
                restoreFile(fname);
            });

            row->addWidget(lbl);
            row->addStretch();
            row->addWidget(btn);

            QWidget *rowW = new QWidget();
            rowW->setLayout(row);
            rowW->setStyleSheet("background:transparent;");
            m_chipLayout->addWidget(rowW);
        }
        m_excludedBox->setTitle(
            QString("EXCLUDED FILES  [%1]").arg(m_excludedFiles.size()));
    }
    m_chipLayout->addStretch();

    if (!m_customOrder.isEmpty()) {
        m_customOrder.clear();
        m_priorityBtn->setText("⇅   SET COMPILE ORDER");
        m_priorityBtn->setStyleSheet("");
    }

    if (!m_excludedFiles.isEmpty())
        m_excludeBtn->setText(
            QString("✕   DISINCLUDE  [%1 hidden]").arg(m_excludedFiles.size()));
    else
        m_excludeBtn->setText("✕   DISINCLUDE");
}

// ── constructor ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("MultiHD Launcher");
    setFixedSize(560, 700);
    setStyleSheet(STYLE);

    QIcon appIcon;
    QSvgRenderer renderer(QString(":/multihd_launcher.svg"));
    QPixmap iconPix(256, 256);
    iconPix.fill(Qt::transparent);
    QPainter iconPainter(&iconPix);
    renderer.render(&iconPainter);
    iconPainter.end();
    appIcon.addPixmap(iconPix);
    setWindowIcon(appIcon);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setSpacing(8);
    layout->setContentsMargins(20, 14, 20, 14);

    // Logo
    QSvgRenderer *svgRender = new QSvgRenderer(QString(":/multihd_launcher.svg"), this);
    QPixmap logoPix(64, 64);
    logoPix.fill(Qt::transparent);
    QPainter logoPainter(&logoPix);
    svgRender->render(&logoPainter);
    logoPainter.end();

    m_logoLabel = new QLabel();
    m_logoLabel->setPixmap(logoPix);
    m_logoLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel("QUESTA FSE  •  QUARTUS PRIME  •  GOWIN EDA");
    m_subtitleLabel->setObjectName("subtitle");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_logoLabel);
    layout->addWidget(m_subtitleLabel);

    m_licenseStatusLabel = new QLabel();
    m_licenseStatusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_licenseStatusLabel);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #1a1a3a;");
    layout->addWidget(line);

    // Path row
    QHBoxLayout *pathRow = new QHBoxLayout();
    m_pathEdit  = new QLineEdit();
    m_pathEdit->setPlaceholderText("Select project folder...");
    m_browseBtn = new QPushButton("Browse");
    m_browseBtn->setObjectName("browseBtn");
    m_browseBtn->setFixedWidth(80);
    pathRow->addWidget(m_pathEdit);
    pathRow->addWidget(m_browseBtn);
    layout->addLayout(pathRow);

    // ── EXCLUDED FILES panel ──────────────────────────────────────────────────
    m_excludedBox = new QGroupBox("EXCLUDED FILES");
    m_excludedBox->setObjectName("excludedBox");
    QVBoxLayout *exBoxLayout = new QVBoxLayout(m_excludedBox);
    exBoxLayout->setContentsMargins(8, 6, 8, 6);
    exBoxLayout->setSpacing(0);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFixedHeight(72);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    m_chipContainer = new QWidget();
    m_chipContainer->setStyleSheet("background: transparent;");
    m_chipLayout = new QVBoxLayout(m_chipContainer);
    m_chipLayout->setSpacing(2);
    m_chipLayout->setContentsMargins(0, 0, 0, 0);
    scroll->setWidget(m_chipContainer);

    exBoxLayout->addWidget(scroll);
    layout->addWidget(m_excludedBox);

    // ── Action row ────────────────────────────────────────────────────────────
    QHBoxLayout *actionRow = new QHBoxLayout();
    actionRow->setSpacing(6);

    m_priorityBtn = new QPushButton("⇅   COMPILE ORDER");
    m_priorityBtn->setObjectName("priorityBtn");
    m_priorityBtn->setFixedHeight(34);

    m_excludeBtn = new QPushButton("✕   DISINCLUDE");
    m_excludeBtn->setObjectName("excludeBtn");
    m_excludeBtn->setFixedHeight(34);

    m_configBtn = new QPushButton("⚙   CONFIG");
    m_configBtn->setObjectName("configBtn");
    m_configBtn->setFixedHeight(34);
    m_configBtn->setFixedWidth(110);

    m_libBtn = new QPushButton("📦   LIBRARIES");
    m_libBtn->setObjectName("libBtn");
    m_libBtn->setFixedHeight(34);

    // ── writer-a: ⚙ EDIT conf.svconf button ───────────────────────────────────
    m_confBtn = new QPushButton("⚙   EDIT conf.svconf");
    m_confBtn->setObjectName("confBtn");
    m_confBtn->setFixedHeight(34);

    actionRow->addWidget(m_priorityBtn);
    actionRow->addWidget(m_excludeBtn);
    actionRow->addWidget(m_libBtn);
    actionRow->addWidget(m_configBtn);
    // ── writer-a: conf.svconf button sits right after CONFIG ──────────────────
    actionRow->addWidget(m_confBtn);
    layout->addLayout(actionRow);

    // ── COMPILE FOR section ───────────────────────────────────────────────────
    QGroupBox *compileForBox = new QGroupBox("COMPILE FOR");
    compileForBox->setObjectName("compileForBox");
    QHBoxLayout *cfRow = new QHBoxLayout(compileForBox);
    cfRow->setContentsMargins(12, 4, 12, 6);
    cfRow->setSpacing(24);

    m_chkQuesta  = new QCheckBox("⬡  Questa / Modelsim");
    m_chkQuartus = new QCheckBox("◈  Quartus Prime");
    m_chkGowin   = new QCheckBox("◆  Gowin EDA");

    m_chkQuesta->setObjectName("chkQuesta");
    m_chkQuartus->setObjectName("chkQuartus");
    m_chkGowin->setObjectName("chkGowin");

    m_chkQuesta->setChecked(true);

    cfRow->addWidget(m_chkQuesta);
    cfRow->addWidget(m_chkQuartus);
    cfRow->addWidget(m_chkGowin);
    cfRow->addStretch();
    layout->addWidget(compileForBox);

    // ── writer-b: launch-mode selector (Run GUI / Run CLI / Show WaveForm) ────
    QHBoxLayout *modeRow = new QHBoxLayout();
    modeRow->setSpacing(8);
    QLabel *modeLbl = new QLabel("🎮 LAUNCH MODE");
    modeLbl->setStyleSheet("color: #00e5ff; font-family: 'Courier New'; font-size: 13px;");
    m_launchMode = new QComboBox();
    m_launchMode->setObjectName("launchMode");
    m_launchMode->setFixedHeight(34);
    m_launchMode->addItem("Run GUI");       // index 0 = default
    m_launchMode->addItem("Run CLI");
    m_launchMode->addItem("Show WaveForm");
    m_launchMode->setCurrentIndex(0);
    modeRow->addWidget(modeLbl);
    modeRow->addWidget(m_launchMode, 1);
    layout->addLayout(modeRow);

    // Launch button
    m_launchBtn = new QPushButton("▶   LAUNCH");
    m_launchBtn->setObjectName("launchBtn");
    m_launchBtn->setFixedHeight(46);
    QGraphicsDropShadowEffect *glow = new QGraphicsDropShadowEffect();
    glow->setBlurRadius(20);
    glow->setColor(QColor(0, 180, 255, 160));
    glow->setOffset(0, 0);
    m_launchBtn->setGraphicsEffect(glow);
    layout->addWidget(m_launchBtn);

    // ── Install section ───────────────────────────────────────────────────────
    QGroupBox *installBox = new QGroupBox("INSTALL");
    QHBoxLayout *instRow = new QHBoxLayout(installBox);
    instRow->setContentsMargins(10, 4, 10, 6);
    instRow->setSpacing(10);
    m_installBtn = new QPushButton("⬇   INSTALL QUESTA");
    m_installBtn->setObjectName("installBtn");
    m_installBtn->setFixedHeight(34);
    m_licenseBtn = new QPushButton("🔑   LICENSE REQUEST");
    m_licenseBtn->setObjectName("licenseBtn");
    m_licenseBtn->setFixedHeight(34);
    instRow->addWidget(m_installBtn);
    instRow->addWidget(m_licenseBtn);

    // ── writer-c: About/Help dialog (Feature C) ─────────────────────────────────
    QPushButton *aboutBtn = new QPushButton("❓   ABOUT");
    aboutBtn->setObjectName("aboutBtn");
    aboutBtn->setFixedHeight(34);
    instRow->addWidget(aboutBtn);

    layout->addWidget(installBox);

    // Log
    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_log->setPlaceholderText("// output will appear here...");
    layout->addWidget(m_log);

    connect(m_browseBtn,   &QPushButton::clicked, this, &MainWindow::browseFolder);
    connect(m_priorityBtn, &QPushButton::clicked, this, &MainWindow::openPriorityDialog);
    connect(m_excludeBtn,  &QPushButton::clicked, this, &MainWindow::openExcludeDialog);
    connect(m_libBtn,      &QPushButton::clicked, this, &MainWindow::openLibraryManager);
    connect(m_configBtn,   &QPushButton::clicked, this, &MainWindow::openConfig);
    // ── writer-a: conf.svconf button ──────────────────────────────────────────
    connect(m_confBtn,     &QPushButton::clicked, this, &MainWindow::openConfDialog);
    connect(m_launchBtn,   &QPushButton::clicked, this, &MainWindow::launch);
    connect(m_installBtn,  &QPushButton::clicked, this, &MainWindow::installQuesta);
    connect(m_licenseBtn,  &QPushButton::clicked, this, &MainWindow::openLicenseRequest);
    // ── writer-c: About/Help dialog (Feature C) ─────────────────────────────────
    connect(aboutBtn,      &QPushButton::clicked, this, &MainWindow::openAbout);

    connect(m_pathEdit, &QLineEdit::textChanged, this, [this](){
        if (!m_excludedFiles.isEmpty()) {
            m_excludedFiles.clear();
            rebuildExcludedChips();
        }
    });

    QStringList args = QCoreApplication::arguments();
    if (args.size() > 1 && QDir(args[1]).exists())
        m_pathEdit->setText(args[1]);

    rebuildExcludedChips();
    updateLicenseStatus();
}

// ── paint ─────────────────────────────────────────────────────────────────────
void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter p(this);
    p.setPen(QPen(QColor(0, 229, 255, 12)));
    for (int x = 0; x < width();  x += 30) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 30) p.drawLine(0, y, width(), y);
    p.setPen(QPen(QColor(0, 229, 255, 120), 2));
    p.drawLine(0,0,40,0);          p.drawLine(0,0,0,40);
    p.drawLine(width()-40,0,width()-1,0); p.drawLine(width()-1,0,width()-1,40);
    p.drawLine(0,height()-40,0,height()-1); p.drawLine(0,height()-1,40,height()-1);
    p.drawLine(width()-1,height()-40,width()-1,height()-1);
    p.drawLine(width()-40,height()-1,width()-1,height()-1);
}

// ── browse folder ─────────────────────────────────────────────────────────────
void MainWindow::browseFolder()
{
    QString startDir = m_pathEdit->text().isEmpty()
                       ? settingDefaultFolder()
                       : m_pathEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, "Select Project Folder", startDir);
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
        m_customOrder.clear();
        m_priorityBtn->setText("⇅   COMPILE ORDER");
        m_priorityBtn->setStyleSheet("");
        m_excludedFiles.clear();
        rebuildExcludedChips();
    }
}

// ── open exclude dialog ───────────────────────────────────────────────────────
void MainWindow::openExcludeDialog()
{
    QString dir = m_pathEdit->text().trimmed();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Error", "Please select a project folder first.");
        return;
    }

    QDir d(dir);
    QStringList allSv = d.entryList(QStringList() << "*.sv", QDir::Files);
    if (allSv.isEmpty()) {
        QMessageBox::information(this, "No files", "No .sv files found in the selected folder.");
        return;
    }

    ExcludeDialog dlg(allSv, m_excludedFiles, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_excludedFiles = dlg.excludedFiles();
        rebuildExcludedChips();

        if (!m_excludedFiles.isEmpty()) {
            log(QString("<span style='color:#cc6666'>✕ %1 file(s) excluded from compilation:</span>")
                    .arg(m_excludedFiles.size()));
            for (const QString &f : m_excludedFiles)
                log(QString("<span style='color:#552222'>    %1</span>").arg(f));
        } else {
            log("<span style='color:#00ff88'>✓ All files included (no exclusions).</span>");
        }
    }
}

// ── restore single file ───────────────────────────────────────────────────────
void MainWindow::restoreFile(const QString &filename)
{
    m_excludedFiles.removeAll(filename);
    rebuildExcludedChips();
    log(QString("<span style='color:#00ff88'>↩ Restored: %1</span>").arg(filename));
}

// ── config dialog ─────────────────────────────────────────────────────────────
void MainWindow::openConfig()
{
    ConfigDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        log("<span style='color:#aaaaff'>⚙ Configuration saved.</span>");
        updateLicenseStatus();
    }
}

// ── writer-a: ⚙ EDIT conf.svconf (Feature A) ─────────────────────────────────
// Open the conf.svconf editor for the current project folder. If the file does
// not exist yet, prompt to create a template first (mirrors the extension).
void MainWindow::openConfDialog()
{
    QString dir = m_pathEdit->text().trimmed();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Error", "Please select a project folder first.");
        return;
    }

    const QString confPath = confPathForProject(dir);

    // If conf.svconf is missing, offer to create a template (mirrors extension editConf).
    if (!QFile::exists(confPath)) {
        int r = QMessageBox::question(this, "conf.svconf",
            "No conf.svconf found in this project folder.\n\n"
            "Create a template to get started?",
            QMessageBox::Yes | QMessageBox::Cancel);
        if (r != QMessageBox::Yes) return;

        // ── writer-a: write a fresh template so the editor opens populated ─────
        QStringList allBases;
        const QStringList all = QDir(dir).entryList(QStringList() << "*.sv", QDir::Files);
        for (const QString &f : all) if (!f.startsWith('.')) allBases << f;
        allBases.sort();
        QString openedBase = allBases.isEmpty() ? QString() : allBases.first();
        QFile tf(confPath);
        if (tf.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream to(&tf);
            to << defaultConfTemplate(openedBase, allBases);
            tf.close();
        }
    }

    ConfDialog dlg(dir, this);
    if (dlg.exec() == QDialog::Accepted) {
        log("<span style='color:#00e5ff'>⚙ conf.svconf saved.</span>");
    }
}

// ── priority dialog ───────────────────────────────────────────────────────────
void MainWindow::openPriorityDialog()
{
    QString dir = m_pathEdit->text().trimmed();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Error", "Please select a project folder first.");
        return;
    }

    QStringList files = activeFiles(dir);
    if (files.isEmpty()) {
        QMessageBox::warning(this, "Error",
            "No .sv files available (all may be excluded).\n"
            "Restore some files from the EXCLUDED FILES panel first.");
        return;
    }

    if (!m_customOrder.isEmpty()) {
        QStringList filtered;
        for (const QString &f : m_customOrder)
            if (files.contains(f)) filtered << f;
        for (const QString &f : files)
            if (!filtered.contains(f)) filtered << f;
        files = filtered;
    }

    PriorityDialog dlg(files, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_customOrder = dlg.orderedFiles();
        log("<span style='color:#ffaa00'>⚡ Custom compile order set:</span>");
        int i = 1;
        for (const QString &f : m_customOrder)
            log(QString("<span style='color:#ffaa00'>  [%1] %2</span>").arg(i++).arg(f));
        m_priorityBtn->setText("⇅   ORDER SET  ✓");
        m_priorityBtn->setStyleSheet(
            "QPushButton#priorityBtn { color: #00ff88; border-color: #00ff88; }");
    }
}

// ── library manager ───────────────────────────────────────────────────────────
void MainWindow::openLibraryManager()
{
    LibraryDialog dlg(&m_libMgr, m_pathEdit->text().trimmed(), settingVsim(), this);
    dlg.exec();
    QString dir = m_pathEdit->text().trimmed();
    if (!dir.isEmpty()) {
        QStringList deps = m_libMgr.projectDeps(dir);
        if (!deps.isEmpty())
            log(QString("<span style='color:#88ddff'>📦 Project libraries: %1</span>")
                    .arg(deps.join(", ")));
    }
}

// ── generate run.do (Questa) ──────────────────────────────────────────────────
// ── writer-b: added `mode` param ("gui"|"cli"); gui keeps Qt byte-parity ─────
QString MainWindow::generateRunDo(const QString &dir, const QString &mode)
{
    // ── writer-a: conf.svconf (Feature A) — parse the per-project config ───────
    SvConf conf = parseConf(confPathForProject(dir));
    const QString workLib = conf.workLib.isEmpty() ? "work" : conf.workLib;

    // ── compile order: conf priority list refines; custom order + exclusion win ─
    QStringList allFiles;
    const QStringList priorityList = conf.priorityList;
    const QStringList tbPriorityList = conf.tbPriorityList;

    if (!m_customOrder.isEmpty()) {
        for (const QString &f : m_customOrder)
            if (!m_excludedFiles.contains(f)) allFiles << f;
    } else {
        QStringList active = activeFiles(dir);
        if (active.isEmpty()) return QString();

        // Apply conf @compile mode: all / list / N.
        QString compileMode = conf.compileMode.trimmed().toLower();
        if (compileMode == "list") {
            // compile only files in the conf priority list (that exist & not excluded)
            for (const QString &f : priorityList) {
                if (active.contains(f) && !m_excludedFiles.contains(f))
                    allFiles << f;
            }
        } else if (compileMode.toInt()) {
            // compile @(N) — first N files of the priority list
            int n = compileMode.toInt();
            int cnt = 0;
            for (const QString &f : priorityList) {
                if (cnt >= n) break;
                if (active.contains(f) && !m_excludedFiles.contains(f)) {
                    allFiles << f; ++cnt;
                }
            }
        } else {
            // "all" (default): non-_tb first (alpha), then _tb (alpha); but honour
            // conf @priority list / @tb priority list / @tb (name) refinements.
            QStringList tbFiles, otherFiles;
            for (const QString &f : active) {
                if (f.endsWith("_tb.sv")) tbFiles  << f;
                else                      otherFiles << f;
            }
            // order RTL by conf priority list (listed first), then the rest
            QStringList orderedRtl;
            for (const QString &f : priorityList)
                if (otherFiles.contains(f)) orderedRtl << f;
            for (const QString &f : otherFiles)
                if (!orderedRtl.contains(f)) orderedRtl << f;
            // order TB by conf tb priority list, then @tb (name) last, then rest
            QStringList orderedTb;
            for (const QString &f : tbPriorityList)
                if (tbFiles.contains(f)) orderedTb << f;
            const QString confTb = conf.tbName.trimmed();
            if (!confTb.isEmpty() && tbFiles.contains(confTb) && !orderedTb.contains(confTb))
                orderedTb << confTb;
            for (const QString &f : tbFiles)
                if (!orderedTb.contains(f)) orderedTb << f;
            allFiles = orderedRtl + orderedTb;
        }
    }

    if (allFiles.isEmpty()) return QString();

    QString tbFile;
    for (const QString &f : allFiles)
        if (f.endsWith("_tb.sv")) tbFile = f;
    if (tbFile.isEmpty()) tbFile = allFiles.last();

    // ── writer-a: @topmodule overrides the detected top module stem ────────────
    QString tbModule = conf.topModule.trimmed();
    if (tbModule.isEmpty()) {
        tbModule = tbFile; tbModule.remove(".sv");
    }
    QString vlogFiles = allFiles.join(" ");

    QStringList deps     = m_libMgr.projectDeps(dir);
    QStringList libFlags = m_libMgr.linkFlags(deps);
    QString libFlagsStr  = libFlags.join(" ");

    for (const QString &dep : deps) {
        if (m_libMgr.hasLibrary(dep) && !m_libMgr.library(dep).compiled)
            log(QString("<span style='color:#ffaa00'>⚠ Library \"%1\" not compiled — open 📦 LIBRARIES.</span>").arg(dep));
    }

    QString content;
    QTextStream ts(&content);
    ts << "# Questa run script — generated by MultiHD Launcher\n";
    ts << "# " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
    ts << "quietly vlib " << workLib << "\n";
    for (const QString &dep : deps) {
        if (!m_libMgr.hasLibrary(dep)) continue;
        LibEntry le = m_libMgr.library(dep);
        QStringList depSv = QDir(le.sourcePath).entryList({"*.sv"}, QDir::Files);
        QStringList depPaths;
        for (const QString &f : depSv) depPaths << (le.sourcePath + "/" + f);
        ts << "vlog -work " << m_libMgr.workDbPath(dep) << " " << depPaths.join(" ") << "\n";
    }
    // ── writer-a: @vlogargs injected after `-work <lib>` (mirrors extension) ───
    const QString vlogArgs = conf.vlogArgs.trimmed();
    ts << "vlog -work " << workLib << " "
       << (vlogArgs.isEmpty() ? "" : vlogArgs + " ")
       << (libFlagsStr.isEmpty() ? "" : libFlagsStr + " ") << vlogFiles << "\n";
    ts << "vopt +acc" << (libFlagsStr.isEmpty() ? "" : " " + libFlagsStr)
       << " " << workLib << "." << tbModule << " -o " << tbModule << "_opt\n";
    // ── writer-a: @vsimargs injected after `vsim` (mirrors extension) ──────────
    const QString vsimArgs = conf.vsimArgs.trimmed();
    ts << "vsim"
       << (vsimArgs.isEmpty() ? "" : " " + vsimArgs)
       << (libFlagsStr.isEmpty() ? "" : " " + libFlagsStr)
       << " " << tbModule << "_opt\n";
    // ── writer-b: GUI keeps `add wave -r /*`; CLI is headless with `quit -f` ──
    if (mode != "cli") ts << "add wave -r /*\n";
    ts << "run -all\n";
    if (mode == "cli") ts << "quit -f\n";

    QFile f(dir + "/run.do");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f); out << content; f.close();
    }
    return tbModule;
}

// ── generate Quartus files (.qpf / .qsf / .sdc) ──────────────────────────────
void MainWindow::generateQuartusFiles(const QString &dir, const QStringList &active)
{
    QString projName = QDir(dir).dirName();
    QString ts       = QDateTime::currentDateTime().toString(Qt::ISODate);

    // ── .qpf ─────────────────────────────────────────────────────────────────
    {
        QFile f(dir + "/" + projName + ".qpf");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "# Quartus Prime Project File\n";
            s << "# Generated by MultiHD Launcher  " << ts << "\n\n";
            s << "QUARTUS_VERSION = \"23.1\"\n";
            s << "DATE = \"" << ts << "\"\n\n";
            s << "PROJECT_REVISION = \"" << projName << "\"\n";
        }
    }
    log(QString("<span style='color:#446688'>    ↳ %1/%2.qpf</span>").arg(dir).arg(projName));

    // ── Determine top module (last non-tb file, or last if all are tb) ────────
    QString topEntity;
    for (const QString &f : active)
        if (!f.endsWith("_tb.sv")) topEntity = f.chopped(3);
    if (topEntity.isEmpty() && !active.isEmpty())
        topEntity = active.last().chopped(3);

    // ── .qsf ─────────────────────────────────────────────────────────────────
    {
        QFile f(dir + "/" + projName + ".qsf");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "# Quartus Settings File\n";
            s << "# Generated by MultiHD Launcher  " << ts << "\n\n";
            s << "set_global_assignment -name FAMILY \"Cyclone V\"\n";
            s << "set_global_assignment -name DEVICE AUTO\n";
            s << "set_global_assignment -name TOP_LEVEL_ENTITY " << topEntity << "\n";
            s << "set_global_assignment -name PROJECT_OUTPUT_DIRECTORY output_files\n";
            s << "set_global_assignment -name MIN_CORE_JUNCTION_TEMP 0\n";
            s << "set_global_assignment -name MAX_CORE_JUNCTION_TEMP 85\n\n";
            for (const QString &sv : active)
                s << "set_global_assignment -name SYSTEMVERILOG_FILE " << sv << "\n";
            s << "\n";
            s << "set_global_assignment -name SDC_FILE " << projName << ".sdc\n";
        }
    }
    log(QString("<span style='color:#446688'>    ↳ %1/%2.qsf</span>").arg(dir).arg(projName));

    // ── .sdc (timing constraints template) ───────────────────────────────────
    QString sdcPath = dir + "/" + projName + ".sdc";
    if (!QFile::exists(sdcPath)) {
        // Only write if there isn't one already (respect hand-edited files)
        QFile f(sdcPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "# Synopsys Design Constraints (.sdc)\n";
            s << "# Generated by MultiHD Launcher  " << ts << "\n";
            s << "# Adjust the clock period and port names to match your design.\n\n";
            s << "# ── Primary clock ────────────────────────────────────────────────────────\n";
            s << "create_clock -period 10.000 -name clk [get_ports clk]\n\n";
            s << "# ── Derive clock uncertainty automatically ───────────────────────────────\n";
            s << "derive_clock_uncertainty\n\n";
            s << "# ── False paths for async reset / buttons (adjust as needed) ────────────\n";
            s << "# set_false_path -from [get_ports rst_n] -to [all_registers]\n";
        }
        log(QString("<span style='color:#446688'>    ↳ %1/%2.sdc  (new template)</span>").arg(dir).arg(projName));
    } else {
        log(QString("<span style='color:#334455'>    ↳ %1/%2.sdc  (exists — not overwritten)</span>").arg(dir).arg(projName));
    }

    // ── output_files/ directory ───────────────────────────────────────────────
    QDir(dir).mkpath("output_files");
    log(QString("<span style='color:#446688'>    ↳ %1/output_files/  (created)</span>").arg(dir));
}

// ── generate Gowin files (.gprj / .cst / .f) ─────────────────────────────────
// Returns false if the user cancelled the CST dialog.
bool MainWindow::generateGowinFiles(const QString &dir, const QStringList &active)
{
    QString projName = QDir(dir).dirName();
    QString ts       = QDateTime::currentDateTime().toString(Qt::ISODate);

    // ── Determine top-module .sv file ─────────────────────────────────────────
    QString topSv;
    for (const QString &f : active)
        if (!f.endsWith("_tb.sv")) topSv = dir + "/" + f;
    if (topSv.isEmpty() && !active.isEmpty())
        topSv = dir + "/" + active.last();

    // ── Open CST dialog ───────────────────────────────────────────────────────
    GowinCstDialog cstDlg(dir, topSv, this);
    if (cstDlg.exec() != QDialog::Accepted)
        return false;                       // user cancelled

    QString device = cstDlg.deviceName();
    QString pn     = cstDlg.devicePn();
    if (device.isEmpty()) { device = "GW1NR-LV9QN88PC6/I5"; pn = device; }

    // ── .cst ─────────────────────────────────────────────────────────────────
    {
        QFile f(dir + "/" + projName + ".cst");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << cstDlg.cstContent();
        }
    }
    log(QString("<span style='color:#446688'>    ↳ %1/%2.cst</span>").arg(dir).arg(projName));

    // ── .f  (Gowin simulation / compile file list) ────────────────────────────
    {
        QFile f(dir + "/" + projName + ".f");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "// Gowin file list (.f)\n";
            s << "// Generated by MultiHD Launcher  " << ts << "\n";
            s << "// Pass this to vlog/iverilog with  -f " << projName << ".f\n\n";
            s << "+incdir+.\n\n";
            // Non-tb first, then tb
            for (const QString &sv : active)
                if (!sv.endsWith("_tb.sv"))
                    s << "./" << sv << "\n";
            for (const QString &sv : active)
                if (sv.endsWith("_tb.sv"))
                    s << "./" << sv << "\n";
        }
    }
    log(QString("<span style='color:#446688'>    ↳ %1/%2.f</span>").arg(dir).arg(projName));

    // ── .gprj ─────────────────────────────────────────────────────────────────
    QString topEntity = QFileInfo(topSv).baseName();
    {
        QFile f(dir + "/" + projName + ".gprj");
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            s << "<!-- Gowin EDA project — generated by MultiHD Launcher  " << ts << " -->\n";
            s << "<Project>\n";
            s << "  <Template>FPGA</Template>\n";
            s << "  <Version>5</Version>\n";
            s << "  <Device name=\"" << device << "\" pn=\"" << pn << "\"/>\n";
            s << "  <FileList>\n";
            for (const QString &sv : active)
                s << "    <File path=\"" << sv << "\" type=\"file.sv\" enable=\"true\"/>\n";
            // Reference the CST file
            s << "    <File path=\"" << projName << ".cst\" type=\"file.cst\" enable=\"true\"/>\n";
            s << "  </FileList>\n";
            s << "  <Synthesis>\n";
            s << "    <TopModule>" << topEntity << "</TopModule>\n";
            s << "  </Synthesis>\n";
            s << "</Project>\n";
        }
    }
    log(QString("<span style='color:#446688'>    ↳ %1/%2.gprj</span>").arg(dir).arg(projName));

    // ── impl/ directory ───────────────────────────────────────────────────────
    QDir(dir).mkpath("impl/pnr");
    log(QString("<span style='color:#446688'>    ↳ %1/impl/pnr/  (created)</span>").arg(dir));

    return true;
}

// ── writer-b: map the combo selection to a mode string ("gui"|"cli"|"wave") ──
QString MainWindow::currentLaunchMode() const
{
    if (!m_launchMode) return "gui";                 // safe fallback = default
    switch (m_launchMode->currentIndex()) {
        case 1: return "cli";
        case 2: return "wave";
        default: return "gui";
    }
}

// ── launch (dispatcher) ───────────────────────────────────────────────────────
void MainWindow::launch()
{
    QString dir = m_pathEdit->text().trimmed();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Error", "Please select a valid project folder.");
        return;
    }

    if (!m_chkQuesta->isChecked() && !m_chkQuartus->isChecked() && !m_chkGowin->isChecked()) {
        QMessageBox::warning(this, "Nothing selected",
            "Please tick at least one target in the COMPILE FOR section.");
        return;
    }

    if (m_chkQuesta->isChecked())  launchForQuesta();
    if (m_chkQuartus->isChecked()) launchForQuartus();
    if (m_chkGowin->isChecked())   launchForGowin();
}

// ── Questa ────────────────────────────────────────────────────────────────────
// ── writer-b: branches on the selected launch mode (Run GUI / Run CLI / ───────
// ──            Show WaveForm), mirroring the extension's flow logic ──────────
void MainWindow::launchForQuesta()
{
    QString dir  = m_pathEdit->text().trimmed();
    QString mode = currentLaunchMode();          // "gui" | "cli" | "wave"

    // ── Show WaveForm: do NOT re-run the simulation ────────────────────────────
    if (mode == "wave") {
        QString wlf = dir + "/vsim.wlf";
        if (!QFile::exists(wlf)) {
            QMessageBox::information(this, "No Waveform",
                "No vsim.wlf found in this directory.\n"
                "Run a simulation first (Run CLI or Run GUI) to generate the waveform file.");
            return;
        }

        // Write a small wave.do (mirrors the extension Show WaveForm flow).
        QFile waveDoFile(dir + "/wave.do");
        if (waveDoFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream w(&waveDoFile);
            w << "# wave.do — generated by MultiHD Launcher (Show WaveForm)\n";
            w << "add wave -r /*\n";
            w << "wave zoom full\n";
            waveDoFile.close();
        }

        log("<span style='color:#00e5ff'>〰 [Questa] Opening waveform viewer...</span>");
        log(QString("<span style='color:#446688'>  wlf:     %1</span>").arg(wlf));
        log(QString("<span style='color:#446688'>  wave.do: %1/wave.do</span>").arg(dir));

        QString vsim    = settingVsim();
        QString lic     = settingLicenseFile();
        QString salt    = settingSaltServer();
        QString mtiMode = settingMtiVcoMode();

        QStringList viewArgs = {"-view", wlf, "-do", dir + "/wave.do"};

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PATH", QFileInfo(vsim).absolutePath() + ":" + env.value("PATH"));
        env.insert("LM_LICENSE_FILE",     lic);
        env.insert("SALT_LICENSE_SERVER", salt.isEmpty() ? lic : salt);
        env.insert("MTI_VCO_MODE",        mtiMode.isEmpty() ? "64" : mtiMode);

        QProcess *proc = new QProcess(this);
        proc->setProcessEnvironment(env);
        proc->setWorkingDirectory(dir);
        proc->setProgram(vsim);
        proc->setArguments(viewArgs);
        proc->startDetached();

        log("<span style='color:#00ff88'>✓ [Questa] WaveForm viewer launched!</span>");
        if (!m_chkQuartus->isChecked() && !m_chkGowin->isChecked())
            QTimer::singleShot(1500, this, &QWidget::close);
        return;
    }

    // ── Run GUI / Run CLI: the simulator actually runs ────────────────────────
    QString licPath = settingLicenseFile();
    if (licPath.isEmpty() || !QFile::exists(licPath)) {
        log("<span style='color:#ff4444'>✗ [Questa] BLOCKED: License file not configured or not found!</span>");
        int r = QMessageBox::critical(this, "License Not Configured",
            "Questa will not start without a valid license file.\n\n"
            "Open Configuration now to set it up?",
            QMessageBox::Yes | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) openConfig();
        return;
    }

    if (!m_excludedFiles.isEmpty())
        log(QString("<span style='color:#cc6666'>✕ %1 file(s) excluded from this run.</span>")
                .arg(m_excludedFiles.size()));

    QString runDoMode = (mode == "cli") ? "cli" : "gui";
    log("<span style='color:#00e5ff'>⬡ [Questa] Generating run.do...</span>");
    QString tb = generateRunDo(dir, runDoMode);
    if (tb.isEmpty()) {
        log("<span style='color:#ff4444'>✗ [Questa] No simulation files remaining after exclusions!</span>");
        QMessageBox::critical(this, "Error",
            "No .sv files left to compile.\n"
            "Restore some files from the EXCLUDED FILES panel.");
        return;
    }
    log("<span style='color:#00ff88'>✓ [Questa] run.do ready — top: " + tb + "</span>");

    QString vsim    = settingVsim();
    QString lic     = settingLicenseFile();
    QString salt    = settingSaltServer();
    QString mtiMode = settingMtiVcoMode();

    log(QString("<span style='color:#446688'>  vsim:    %1</span>").arg(vsim));
    log(QString("<span style='color:#446688'>  license: %1</span>").arg(lic));
    log(QString("<span style='color:#00e5ff'>▶ [Questa] Launching (%1)...</span>").arg(runDoMode));

    // ── writer-b: Run CLI uses -c (headless); Run GUI keeps -gui (current) ───
    QStringList vsimArgs;
    if (mode == "cli") {
        vsimArgs = {"-c", "-do", QString("cd %1; do %1/run.do").arg(dir)};
    } else {
        vsimArgs = {"-gui", "-do", QString("cd %1; do %1/run.do").arg(dir)};
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", QFileInfo(vsim).absolutePath() + ":" + env.value("PATH"));
    env.insert("LM_LICENSE_FILE",     lic);
    env.insert("SALT_LICENSE_SERVER", salt.isEmpty() ? lic : salt);
    env.insert("MTI_VCO_MODE",        mtiMode.isEmpty() ? "64" : mtiMode);

    QProcess *proc = new QProcess(this);
    proc->setProcessEnvironment(env);
    proc->setWorkingDirectory(dir);
    proc->setProgram(vsim);
    proc->setArguments(vsimArgs);
    proc->startDetached();

    log("<span style='color:#00ff88'>✓ [Questa] Launched!</span>");
    if (!m_chkQuartus->isChecked() && !m_chkGowin->isChecked())
        QTimer::singleShot(1500, this, &QWidget::close);
}

// ── Quartus ───────────────────────────────────────────────────────────────────
void MainWindow::launchForQuartus()
{
    QString dir = m_pathEdit->text().trimmed();
    QStringList active = activeFiles(dir);

    if (active.isEmpty()) {
        log("<span style='color:#ff4444'>✗ [Quartus] No active .sv files to add.</span>");
        return;
    }

    log("<span style='color:#ffaa00'>◈ [Quartus] Generating project files...</span>");
    generateQuartusFiles(dir, active);
    log("<span style='color:#00ff88'>✓ [Quartus] Files written.</span>");

    // Try to open Quartus if configured / on PATH
    QSettings s("MultiHDLauncher", "MultiHDLauncher");
    QString quartusPath = s.value("quartusPath", "").toString();
    QString quartusExe;
    if (!quartusPath.isEmpty() && QFile::exists(quartusPath))
        quartusExe = quartusPath;
    else {
        QProcess which;
        which.start("which", {"quartus"});
        which.waitForFinished(3000);
        QString found = QString::fromLocal8Bit(which.readAllStandardOutput()).trimmed();
        if (!found.isEmpty() && QFile::exists(found)) quartusExe = found;
    }

    QString qpf = dir + "/" + QDir(dir).dirName() + ".qpf";
    if (!quartusExe.isEmpty()) {
        log("<span style='color:#ffaa00'>▶ [Quartus] Opening Quartus Prime...</span>");
        QProcess::startDetached(quartusExe, {qpf});
        log("<span style='color:#00ff88'>✓ [Quartus] Launched!</span>");
    } else {
        log("<span style='color:#ffaa00'>⚠ [Quartus] quartus binary not found — files written, open manually.</span>");
        log("<span style='color:#446688'>  Set Quartus path in ⚙ CONFIG → Quartus binary.</span>");
    }
}

// ── Gowin EDA ─────────────────────────────────────────────────────────────────
void MainWindow::launchForGowin()
{
    QString dir = m_pathEdit->text().trimmed();
    QStringList active = activeFiles(dir);

    if (active.isEmpty()) {
        log("<span style='color:#ff4444'>✗ [Gowin] No active .sv files to add.</span>");
        return;
    }

    log("<span style='color:#88ff88'>◆ [Gowin] Opening constraint editor...</span>");
    bool ok = generateGowinFiles(dir, active);
    if (!ok) {
        log("<span style='color:#cc6666'>✗ [Gowin] Cancelled — no files written.</span>");
        return;
    }
    log("<span style='color:#00ff88'>✓ [Gowin] Files written.</span>");

    // Try to open Gowin IDE if configured / on PATH
    QSettings s("MultiHDLauncher", "MultiHDLauncher");
    QString gowinPath = s.value("gowinPath", "").toString();
    QString gowinExe;
    if (!gowinPath.isEmpty() && QFile::exists(gowinPath))
        gowinExe = gowinPath;
    else {
        QProcess which;
        which.start("which", {"gw_ide"});
        which.waitForFinished(3000);
        QString found = QString::fromLocal8Bit(which.readAllStandardOutput()).trimmed();
        if (!found.isEmpty() && QFile::exists(found)) gowinExe = found;
    }

    QString gprj = dir + "/" + QDir(dir).dirName() + ".gprj";
    if (!gowinExe.isEmpty()) {
        log("<span style='color:#88ff88'>▶ [Gowin] Opening Gowin IDE...</span>");
        QProcess::startDetached(gowinExe, {gprj});
        log("<span style='color:#00ff88'>✓ [Gowin] Launched!</span>");
    } else {
        log("<span style='color:#88ff88'>⚠ [Gowin] gw_ide not found — files written, open manually.</span>");
        log("<span style='color:#446688'>  Set Gowin path in ⚙ CONFIG → Gowin IDE binary.</span>");
    }
}

// ── install / license ─────────────────────────────────────────────────────────
void MainWindow::installQuesta()
{
    log("<span style='color:#00ff88'>⬇ Opening Questa FSE download page...</span>");
    QDesktopServices::openUrl(QUrl(
        "https://www.intel.com/content/www/us/en/software-kit/825277/"
        "questa-intel-fpgas-standard-edition-software-version-23-1-1.html"));
}

void MainWindow::openLicenseRequest()
{
    log("<span style='color:#bb88ff'>🔑 Opening license request page...</span>");
    QDesktopServices::openUrl(QUrl(
        "https://licensing.intel.com/psg/s/sales-signup-evaluationlicenses"));
}

// ── writer-c: About/Help dialog (Feature C) ───────────────────────────────────
void MainWindow::openAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::log(const QString &msg)
{
    m_log->append(msg);
}
