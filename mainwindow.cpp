#include "mainwindow.h"
#include "prioritydialog.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QProcess>
#include <QHBoxLayout>
#include <QTimer>
#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QSvgRenderer>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>

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
    QPushButton#launchBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0057a8,stop:1 #00b4d8);
        color: white; border: none; border-radius: 8px;
        font-size: 15px; font-weight: bold; letter-spacing: 1px;
    }
    QPushButton#launchBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0070cc,stop:1 #00d4f0);
    }
    QPushButton#launchBtn:pressed { background: #003d7a; }
    QTextEdit {
        background-color: #080812; color: #00e5ff;
        border: 1px solid #1a1a3a; border-radius: 6px;
        font-family: 'Courier New', monospace; font-size: 11px; padding: 6px;
    }
    QLabel#subtitle {
        color: #446688; font-size: 10px;
        font-family: 'Courier New', monospace; letter-spacing: 2px;
    }
)";

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Questa Launcher");
    setFixedSize(540, 460);
    setStyleSheet(STYLE);

    // Set window icon from SVG
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
    layout->setSpacing(10);
    layout->setContentsMargins(20, 16, 20, 16);

    // SVG Logo in window
    QSvgRenderer *svgRender = new QSvgRenderer(QString(":/questa_launcher.svg"), this);
    QPixmap logoPix(72, 72);
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

    // Priority button
    m_priorityBtn = new QPushButton("⇅   SET COMPILE ORDER");
    m_priorityBtn->setObjectName("priorityBtn");
    m_priorityBtn->setFixedHeight(36);
    layout->addWidget(m_priorityBtn);

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

    // Log
    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_log->setPlaceholderText("// output will appear here...");
    layout->addWidget(m_log);

    connect(m_browseBtn,  &QPushButton::clicked, this, &MainWindow::browseFolder);
    connect(m_priorityBtn,&QPushButton::clicked, this, &MainWindow::openPriorityDialog);
    connect(m_launchBtn,  &QPushButton::clicked, this, &MainWindow::launch);

    QStringList args = QCoreApplication::arguments();
    if (args.size() > 1 && QDir(args[1]).exists())
        m_pathEdit->setText(args[1]);
}

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

void MainWindow::browseFolder()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Project Folder",
        "/home/bismillah/Downloads/SystemVerilogCourse/");
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
        m_customOrder.clear();  // reset order when folder changes
        m_priorityBtn->setText("⇅   SET COMPILE ORDER");
    }
}

void MainWindow::openPriorityDialog()
{
    QString dir = m_pathEdit->text().trimmed();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Error", "Please select a project folder first.");
        return;
    }

    QDir d(dir);
    QStringList files = d.entryList(QStringList() << "*.sv", QDir::Files);
    if (files.isEmpty()) {
        QMessageBox::warning(this, "Error", "No .sv files found in folder.");
        return;
    }

    // Pre-populate with existing custom order if any
    if (!m_customOrder.isEmpty()) files = m_customOrder;

    PriorityDialog dlg(files, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_customOrder = dlg.orderedFiles();
        log("<span style='color:#ffaa00'>⚡ Custom compile order set:</span>");
        int i = 1;
        for (const QString &f : m_customOrder)
            log(QString("<span style='color:#ffaa00'>  [%1] %2</span>").arg(i++).arg(f));
        m_priorityBtn->setText("⇅   ORDER SET  ✓");
        m_priorityBtn->setStyleSheet(m_priorityBtn->styleSheet() +
            "QPushButton#priorityBtn { color: #00ff88; border-color: #00ff88; }");
    }
}

QString MainWindow::generateRunDo(const QString &dir)
{
    QDir d(dir);

    // Use custom order if set, otherwise auto-detect
    QStringList allFiles;
    if (!m_customOrder.isEmpty()) {
        allFiles = m_customOrder;
    } else {
        QStringList tbFiles    = d.entryList(QStringList() << "*_tb.sv", QDir::Files);
        QStringList otherFiles = d.entryList(QStringList() << "*.sv",    QDir::Files);
        if (tbFiles.isEmpty()) return QString();
        for (const QString &f : tbFiles) otherFiles.removeAll(f);
        allFiles = otherFiles + tbFiles;
    }

    // Find tb module (last *_tb.sv in list)
    QString tbFile;
    for (const QString &f : allFiles)
        if (f.endsWith("_tb.sv")) tbFile = f;
    if (tbFile.isEmpty()) return QString();

    QString tbModule = tbFile;
    tbModule.remove(".sv");
    QString vlogFiles = allFiles.join(" ");

    QString content;
    QTextStream ts(&content);
    ts << "quietly vlib work\n";
    ts << "vlog -work work " << vlogFiles << "\n";
    ts << "vopt +acc work." << tbModule << " -o " << tbModule << "_opt\n";
    ts << "vsim " << tbModule << "_opt\n";
    ts << "add wave -r /*\n";
    ts << "run -all\n";

    QFile f(dir + "/run.do");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << content;
        f.close();
    }
    return tbModule;
}

void MainWindow::launch()
{
    QString dir = m_pathEdit->text().trimmed();
    if (dir.isEmpty() || !QDir(dir).exists()) {
        QMessageBox::warning(this, "Error", "Please select a valid project folder.");
        return;
    }

    // Always regenerate run.do so compile order is up to date
    log("<span style='color:#ffaa00'>⚡ Generating run.do...</span>");
    QString tb = generateRunDo(dir);
    if (tb.isEmpty()) {
        log("<span style='color:#ff4444'>✗ ERROR: No *_tb.sv found!</span>");
        QMessageBox::critical(this, "Error", "No *_tb.sv testbench found in:\n" + dir);
        return;
    }
    log("<span style='color:#00ff88'>✓ run.do ready for: " + tb + "</span>");
    log("<span style='color:#00e5ff'>▶ Launching Questa...</span>");

    QString vsim = "/home/bismillah/altera/25.1std/questa_fse/linux_x86_64/vsim";
    QStringList vsimArgs = {"-gui", "-do", QString("cd %1; do %1/run.do").arg(dir)};

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", "/home/bismillah/altera/25.1std/questa_fse/linux_x86_64:" + env.value("PATH"));
    env.insert("LM_LICENSE_FILE",    "/home/bismillah/.licenses/questa_license.dat");
    env.insert("SALT_LICENSE_SERVER","/home/bismillah/.licenses/questa_license.dat");
    env.insert("MTI_VCO_MODE", "64");

    QProcess *proc = new QProcess(this);
    proc->setProcessEnvironment(env);
    proc->setWorkingDirectory(dir);
    proc->setProgram(vsim);
    proc->setArguments(vsimArgs);
    proc->startDetached();

    log("<span style='color:#00ff88'>✓ Done — Questa is starting!</span>");
    QTimer::singleShot(1500, this, &QWidget::close);
}

void MainWindow::log(const QString &msg)
{
    m_log->append(msg);
}
