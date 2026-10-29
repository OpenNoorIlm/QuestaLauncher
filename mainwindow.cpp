#include "mainwindow.h"
#include "prioritydialog.h"
#include "configdialog.h"
#include "excludedialog.h"
#include "librarydialog.h"
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
    QPushButton#licenseBtn {
        background-color: transparent; color: #bb88ff;
        border: 1px solid #bb88ff; border-radius: 6px;
        padding: 6px 14px; font-family: 'Courier New', monospace; font-size: 12px;
    }
    QPushButton#licenseBtn:hover { background-color: #bb88ff22; }
    /* Restore chip buttons inside the excluded panel */
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
    QGroupBox#excludedBox {
        border: 1px solid #331111;
    }
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
    QCheckBox#chkQuesta                { color: #00e5ff; }
    QCheckBox#chkQuesta::indicator     { border: 1px solid #00e5ff; }
    QCheckBox#chkQuesta::indicator:checked { background-color: #0057a8; border-color: #00e5ff; }

    QCheckBox#chkQuartus               { color: #ffaa00; }
    QCheckBox#chkQuartus::indicator    { border: 1px solid #ffaa00; }
    QCheckBox#chkQuartus::indicator:checked { background-color: #7a4400; border-color: #ffaa00; }

    QCheckBox#chkGowin                 { color: #88ff88; }
    QCheckBox#chkGowin::indicator      { border: 1px solid #44aa44; }
    QCheckBox#chkGowin::indicator:checked { background-color: #1a4a1a; border-color: #88ff88; }

    QGroupBox#compileForBox { border-color: #1a2a1a; }
)";

// ── QSettings helpers ─────────────────────────────────────────────────────────
QString MainWindow::settingVsim() const {
    return QSettings("QuestaLauncher","QuestaLauncher")
        .value("vsimPath", "/home/bismillah/altera/25.1std/questa_fse/linux_x86_64/vsim").toString();
}
QString MainWindow::settingLicenseFile() const {
    return QSettings("QuestaLauncher","QuestaLauncher")
        .value("licenseFile", "/home/bismillah/.licenses/questa_license.dat").toString();
}
QString MainWindow::settingSaltServer() const {
    return QSettings("QuestaLauncher","QuestaLauncher")
        .value("saltServer", "/home/bismillah/.licenses/questa_license.dat").toString();
}
QString MainWindow::settingMtiVcoMode() const {
    return QSettings("QuestaLauncher","QuestaLauncher")
        .value("mtiVcoMode", "64").toString();
}
QString MainWindow::settingDefaultFolder() const {
    return QSettings("QuestaLauncher","QuestaLauncher")
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
        m_licenseStatusLabel->setText("⚠ LICENSE NOT SET — configure before launching!");
    }
    m_licenseStatusLabel->style()->unpolish(m_licenseStatusLabel);
    m_licenseStatusLabel->style()->polish(m_licenseStatusLabel);

    if (ok) {
        m_launchBtn->setObjectName("launchBtn");
        m_launchBtn->setText("▶   LAUNCH QUESTA");
    } else {
        m_launchBtn->setObjectName("launchBtnWarn");
        m_launchBtn->setText("⚠   LAUNCH QUESTA  (no license!)");
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
    // Remove all existing chip widgets
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

            QString fname = f;   // capture for lambda
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

    // Reset custom order since the active file set has changed
    if (!m_customOrder.isEmpty()) {
        m_customOrder.clear();
        m_priorityBtn->setText("⇅   SET COMPILE ORDER");
        m_priorityBtn->setStyleSheet("");
    }

    // Update exclude button label
    if (!m_excludedFiles.isEmpty())
        m_excludeBtn->setText(
            QString("✕   DISINCLUDE  [%1 hidden]").arg(m_excludedFiles.size()));
    else
        m_excludeBtn->setText("✕   DISINCLUDE");
}

// ── constructor ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Questa Launcher");
    setFixedSize(560, 700);
    setStyleSheet(STYLE);

    QIcon appIcon;
    QSvgRenderer renderer(QString(":/questa_launcher.svg"));
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
    QSvgRenderer *svgRender = new QSvgRenderer(QString(":/questa_launcher.svg"), this);
    QPixmap logoPix(64, 64);
    logoPix.fill(Qt::transparent);
    QPainter logoPainter(&logoPix);
    svgRender->render(&logoPainter);
    logoPainter.end();

    m_logoLabel = new QLabel();
    m_logoLabel->setPixmap(logoPix);
    m_logoLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel("SIEMENS EDA  •  FPGA STARTER EDITION  •  v2025.2");
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

    // Scrollable chip area (max 3 rows visible)
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

    // ── Action row: priority + disinclude + config ────────────────────────────
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

    actionRow->addWidget(m_priorityBtn);
    actionRow->addWidget(m_excludeBtn);
    actionRow->addWidget(m_libBtn);
    actionRow->addWidget(m_configBtn);
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

    m_chkQuesta->setChecked(true);   // default — this app is for Questa

    cfRow->addWidget(m_chkQuesta);
    cfRow->addWidget(m_chkQuartus);
    cfRow->addWidget(m_chkGowin);
    cfRow->addStretch();
    layout->addWidget(compileForBox);

    // Launch button
    m_launchBtn = new QPushButton("▶   LAUNCH QUESTA");
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
    connect(m_launchBtn,   &QPushButton::clicked, this, &MainWindow::launch);
    connect(m_installBtn,  &QPushButton::clicked, this, &MainWindow::installQuesta);
    connect(m_licenseBtn,  &QPushButton::clicked, this, &MainWindow::openLicenseRequest);

    // Also clear exclusions when a new folder is picked
    connect(m_pathEdit, &QLineEdit::textChanged, this, [this](){
        if (!m_excludedFiles.isEmpty()) {
            m_excludedFiles.clear();
            rebuildExcludedChips();
        }
    });

    QStringList args = QCoreApplication::arguments();
    if (args.size() > 1 && QDir(args[1]).exists())
        m_pathEdit->setText(args[1]);

    rebuildExcludedChips();   // start with empty panel
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

// ── priority dialog (only shows active files) ─────────────────────────────────
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
        // Keep only files that are still active
        QStringList filtered;
        for (const QString &f : m_customOrder)
            if (files.contains(f)) filtered << f;
        // Add any new active files not in the existing order
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
    // Deps may have changed — log current project deps
    QString dir = m_pathEdit->text().trimmed();
    if (!dir.isEmpty()) {
        QStringList deps = m_libMgr.projectDeps(dir);
        if (!deps.isEmpty())
            log(QString("<span style='color:#88ddff'>📦 Project libraries: %1</span>")
                    .arg(deps.join(", ")));
    }
}

// ── generate run.do (respects exclusions + library deps) ──────────────────────
QString MainWindow::generateRunDo(const QString &dir)
{
    QStringList allFiles;

    if (!m_customOrder.isEmpty()) {
        // Custom order: strip any that got excluded since order was set
        for (const QString &f : m_customOrder)
            if (!m_excludedFiles.contains(f)) allFiles << f;
    } else {
        QStringList active = activeFiles(dir);
        if (active.isEmpty()) return QString();

        // Split into tb and non-tb, preserving the auto-ordering convention
        QStringList tbFiles, otherFiles;
        for (const QString &f : active) {
            if (f.endsWith("_tb.sv")) tbFiles  << f;
            else                      otherFiles << f;
        }
        allFiles = otherFiles + tbFiles;
    }

    if (allFiles.isEmpty()) return QString();

    // Top module: last *_tb.sv, or last file if none
    QString tbFile;
    for (const QString &f : allFiles)
        if (f.endsWith("_tb.sv")) tbFile = f;
    if (tbFile.isEmpty()) tbFile = allFiles.last();

    QString tbModule = tbFile; tbModule.remove(".sv");
    QString vlogFiles = allFiles.join(" ");

    // Resolve library deps for this project
    QStringList deps     = m_libMgr.projectDeps(dir);
    QStringList libFlags = m_libMgr.linkFlags(deps);   // ["-L", "/path/db", ...]
    QString libFlagsStr  = libFlags.join(" ");

    // Warn about any dep that isn't compiled yet
    for (const QString &dep : deps) {
        if (m_libMgr.hasLibrary(dep) && !m_libMgr.library(dep).compiled)
            log(QString("<span style='color:#ffaa00'>⚠ Library \"%1\" is not compiled — open 📦 LIBRARIES and compile it.</span>").arg(dep));
    }

    QString content;
    QTextStream ts(&content);
    ts << "quietly vlib work\n";
    // Compile each library dep first so work can reference it
    for (const QString &dep : deps) {
        if (!m_libMgr.hasLibrary(dep)) continue;
        LibEntry le = m_libMgr.library(dep);
        QStringList depSv = QDir(le.sourcePath).entryList({"*.sv"}, QDir::Files);
        QStringList depPaths;
        for (const QString &f : depSv) depPaths << (le.sourcePath + "/" + f);
        ts << "vlog -work " << m_libMgr.workDbPath(dep) << " " << depPaths.join(" ") << "\n";
    }
    ts << "vlog -work work " << (libFlagsStr.isEmpty() ? "" : libFlagsStr + " ") << vlogFiles << "\n";
    ts << "vopt +acc" << (libFlagsStr.isEmpty() ? "" : " " + libFlagsStr)
       << " work." << tbModule << " -o " << tbModule << "_opt\n";
    ts << "vsim" << (libFlagsStr.isEmpty() ? "" : " " + libFlagsStr)
       << " " << tbModule << "_opt\n";
    ts << "add wave -r /*\n";
    ts << "run -all\n";

    QFile f(dir + "/run.do");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f); out << content; f.close();
    }
    return tbModule;
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
void MainWindow::launchForQuesta()
{
    QString dir = m_pathEdit->text().trimmed();

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

    log("<span style='color:#00e5ff'>⬡ [Questa] Generating run.do...</span>");
    QString tb = generateRunDo(dir);
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
    log("<span style='color:#00e5ff'>▶ [Questa] Launching...</span>");

    QStringList vsimArgs = {"-gui", "-do", QString("cd %1; do %1/run.do").arg(dir)};

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
    // Only auto-close when Questa is the only target — keep window open if
    // other tools are also being launched so their log output stays visible.
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

    log("<span style='color:#ffaa00'>◈ [Quartus] Generating Quartus project files...</span>");

    // ── 1. <project>.qpf ─────────────────────────────────────────────────────
    QString projName = QDir(dir).dirName();
    QString qpf = dir + "/" + projName + ".qpf";
    {
        QFile f(qpf);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "# Quartus Prime project file — generated by QuestaLauncher\n";
            s << "QUARTUS_VERSION = \"23.1\"\n";
            s << "DATE = \"" << QDateTime::currentDateTime().toString(Qt::ISODate) << "\"\n\n";
            s << "PROJECT_REVISION = \"" << projName << "\"\n";
        }
    }

    // ── 2. <project>.qsf ─────────────────────────────────────────────────────
    QString qsf = dir + "/" + projName + ".qsf";
    {
        QFile f(qsf);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "# Quartus Settings File — generated by QuestaLauncher\n\n";
            s << "set_global_assignment -name FAMILY \"Cyclone V\"\n";
            s << "set_global_assignment -name DEVICE AUTO\n";
            s << "set_global_assignment -name TOP_LEVEL_ENTITY " << active.last().chopped(3) << "\n\n";
            for (const QString &sv : active)
                s << "set_global_assignment -name SYSTEMVERILOG_FILE " << sv << "\n";
        }
    }

    log(QString("<span style='color:#00ff88'>✓ [Quartus] Generated:</span>"));
    log(QString("<span style='color:#446688'>    %1</span>").arg(qpf));
    log(QString("<span style='color:#446688'>    %1</span>").arg(qsf));

    // ── 3. Try to open Quartus if it's on PATH ────────────────────────────────
    QSettings s("QuestaLauncher", "QuestaLauncher");
    QString quartusPath = s.value("quartusPath", "").toString();

    QString quartusExe;
    if (!quartusPath.isEmpty() && QFile::exists(quartusPath))
        quartusExe = quartusPath;
    else {
        // Try to find quartus on PATH
        QProcess which;
        which.start("which", {"quartus"});
        which.waitForFinished(3000);
        QString found = QString::fromLocal8Bit(which.readAllStandardOutput()).trimmed();
        if (!found.isEmpty() && QFile::exists(found))
            quartusExe = found;
    }

    if (!quartusExe.isEmpty()) {
        log("<span style='color:#ffaa00'>▶ [Quartus] Opening Quartus Prime...</span>");
        QProcess::startDetached(quartusExe, {qpf});
        log("<span style='color:#00ff88'>✓ [Quartus] Launched!</span>");
    } else {
        log("<span style='color:#ffaa00'>⚠ [Quartus] quartus binary not found — project files written, open manually.</span>");
        log("<span style='color:#446688'>  Set Quartus path in ⚙ CONFIG → Quartus binary to auto-launch.</span>");
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

    log("<span style='color:#88ff88'>◆ [Gowin] Generating Gowin project file...</span>");

    QString projName = QDir(dir).dirName();

    // ── <project>.gprj (Gowin IDE project XML) ────────────────────────────────
    QString gprj = dir + "/" + projName + ".gprj";
    {
        QFile f(gprj);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            s << "<!-- Gowin EDA project — generated by QuestaLauncher -->\n";
            s << "<Project>\n";
            s << "  <Template>FPGA</Template>\n";
            s << "  <Version>5</Version>\n";
            s << "  <Device name=\"GW1NR-LV9QN88PC6/I5\" pn=\"GW1NR-LV9QN88PC6/I5\"/>\n";
            s << "  <FileList>\n";
            for (const QString &sv : active)
                s << "    <File path=\"" << sv << "\" type=\"file.sv\" enable=\"true\"/>\n";
            s << "  </FileList>\n";
            s << "  <Synthesis>\n";
            s << "    <TopModule>" << active.last().chopped(3) << "</TopModule>\n";
            s << "  </Synthesis>\n";
            s << "</Project>\n";
        }
    }

    log(QString("<span style='color:#00ff88'>✓ [Gowin] Generated:</span>"));
    log(QString("<span style='color:#446688'>    %1</span>").arg(gprj));

    // ── Try to open Gowin IDE if configured ───────────────────────────────────
    QSettings s("QuestaLauncher", "QuestaLauncher");
    QString gowinPath = s.value("gowinPath", "").toString();

    QString gowinExe;
    if (!gowinPath.isEmpty() && QFile::exists(gowinPath))
        gowinExe = gowinPath;
    else {
        QProcess which;
        which.start("which", {"gw_ide"});
        which.waitForFinished(3000);
        QString found = QString::fromLocal8Bit(which.readAllStandardOutput()).trimmed();
        if (!found.isEmpty() && QFile::exists(found))
            gowinExe = found;
    }

    if (!gowinExe.isEmpty()) {
        log("<span style='color:#88ff88'>▶ [Gowin] Opening Gowin IDE...</span>");
        QProcess::startDetached(gowinExe, {gprj});
        log("<span style='color:#00ff88'>✓ [Gowin] Launched!</span>");
    } else {
        log("<span style='color:#88ff88'>⚠ [Gowin] gw_ide not found — project file written, open manually.</span>");
        log("<span style='color:#446688'>  Set Gowin path in ⚙ CONFIG → Gowin IDE binary to auto-launch.</span>");
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

void MainWindow::log(const QString &msg)
{
    m_log->append(msg);
}
