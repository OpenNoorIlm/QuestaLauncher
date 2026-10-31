#include "confdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

// ── shared dark-cyber palette (mirrors other dialogs) ─────────────────────────
static const QString CONF_STYLE = R"(
    QDialog, QWidget { background-color: #0d0d1a; color: #e0e0e0; }
    QLineEdit, QTextEdit, QComboBox {
        background-color: #131326; color: #00e5ff;
        border: 1px solid #00e5ff; border-radius: 6px;
        padding: 5px 9px; font-family: 'Courier New', monospace; font-size: 11px;
    }
    QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border-color: #00ffcc; }
    QComboBox::drop-down { border: none; width: 20px; }
    QComboBox::down-arrow {
        image: none; border-left: 5px solid transparent; border-right: 5px solid transparent;
        border-top: 6px solid #00e5ff; margin-right: 6px;
    }
    QComboBox QAbstractItemView {
        background-color: #131326; color: #00e5ff; selection-background-color: #1a3a4a;
    }
    QLabel {
        font-family: 'Courier New', monospace; font-size: 11px; color: #8899bb;
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
    QPushButton#templateBtn {
        background-color: transparent; color: #00ff88;
        border: 1px solid #00ff88; border-radius: 6px;
        padding: 6px 12px; font-family: 'Courier New', monospace; font-size: 11px;
    }
    QPushButton#templateBtn:hover { background-color: #00ff8818; }
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

// ═════════════════════════════════════════════════════════════════════════════
// conf.svconf parser — mirrors extension src/core/project.ts parseConf()
// ═════════════════════════════════════════════════════════════════════════════
SvConf parseConf(const QString &confPath)
{
    SvConf conf;

    QFile f(confPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return conf;

    QTextStream in(&f);
    QStringList lines;
    while (!in.atEnd())
        lines << in.readLine();

    bool inPriority = false;
    bool inTbPriority = false;

    QRegularExpression compileRe("^compile\\s+@\\((.+?)\\)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression tbNameRe("^@tb\\s+\\(([^)]+)\\)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression topRe("^@topmodule\\s*:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression vsimRe("^@vsimargs\\s*:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression vlogRe("^@vlogargs\\s*:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression workRe("^@work\\s*:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression priorityRe("^@priority\\s+list", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression tbPriorityRe("^@tb\\s+priority\\s+list", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression itemRe("^[-\\d.]\\s*");

    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // compile @(all) / compile @(1) / compile @(list)
        auto cm = compileRe.match(line);
        if (cm.hasMatch()) {
            const QString val = cm.captured(1).trimmed().toLower();
            if (val == "all")            conf.compileMode = "all";
            else if (val == "list")      conf.compileMode = "list";
            else {
                bool ok = false;
                int n = val.toInt(&ok);
                conf.compileMode = (ok && n > 0) ? QString::number(n) : "all";
            }
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // @tb (filename.sv) — explicit testbench
        auto tb = tbNameRe.match(line);
        if (tb.hasMatch()) {
            conf.tbName = tb.captured(1).trimmed();
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // @tb priority list
        if (tbPriorityRe.match(line).hasMatch()) {
            inTbPriority = true;
            inPriority = false;
            conf.tbPriorityList.clear();
            continue;
        }

        // @priority list encoder
        if (priorityRe.match(line).hasMatch()) {
            inPriority = true;
            inTbPriority = false;
            conf.priorityList.clear();
            continue;
        }

        // @topmodule: counter16_tb
        auto top = topRe.match(line);
        if (top.hasMatch()) {
            conf.topModule = top.captured(1).trimmed();
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // @vsimargs: -voptargs=+acc
        auto vsim = vsimRe.match(line);
        if (vsim.hasMatch()) {
            conf.vsimArgs = vsim.captured(1).trimmed();
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // @vlogargs: -sv -quiet
        auto vlog = vlogRe.match(line);
        if (vlog.hasMatch()) {
            conf.vlogArgs = vlog.captured(1).trimmed();
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // @work: work
        auto work = workRe.match(line);
        if (work.hasMatch()) {
            conf.workLib = work.captured(1).trimmed();
            inPriority = false;
            inTbPriority = false;
            continue;
        }

        // inside @tb priority list block — file lines
        if (inTbPriority && (line.startsWith('-') || line.endsWith(".sv") ||
                             QRegularExpression("^\\d+\\.").match(line).hasMatch())) {
            QString fname = line;
            fname.remove(itemRe);
            fname = fname.trimmed();
            if (!fname.isEmpty()) conf.tbPriorityList << fname;
            continue;
        }

        // inside @priority list block — file lines
        if (inPriority && (line.startsWith('-') || line.endsWith(".sv") ||
                           QRegularExpression("^\\d+\\.").match(line).hasMatch())) {
            QString fname = line;
            fname.remove(itemRe);
            fname = fname.trimmed();
            if (!fname.isEmpty()) conf.priorityList << fname;
        }
    }

    return conf;
}

// ═════════════════════════════════════════════════════════════════════════════
// conf.svconf writer — mirrors extension src/core/project.ts saveConf()
// ═════════════════════════════════════════════════════════════════════════════
void saveConf(const QString &confPath, const SvConf &conf)
{
    QString modeStr = conf.compileMode;
    if (modeStr != "all" && modeStr != "list") {
        bool ok = false;
        modeStr.toInt(&ok);
        modeStr = ok ? modeStr : "all";
    }

    QString content;
    QTextStream ts(&content);
    ts << "# conf.svconf — MultiHD Launcher configuration\n";
    ts << "# Generated by MultiHD Launcher\n\n";
    ts << "compile @(" << modeStr << ")\n\n";

    if (!conf.tbName.isEmpty())
        ts << "@tb (" << conf.tbName << ")\n\n";

    if (!conf.tbPriorityList.isEmpty()) {
        ts << "@tb priority list:\n";
        for (const QString &f : conf.tbPriorityList) ts << "  - " << f << "\n";
        ts << "\n";
    }

    ts << "@priority list encoder:\n";
    for (const QString &f : conf.priorityList) ts << "  - " << f << "\n";
    ts << "\n";

    if (!conf.topModule.isEmpty()) ts << "@topmodule: " << conf.topModule << "\n";
    if (!conf.vsimArgs.isEmpty())  ts << "@vsimargs: " << conf.vsimArgs << "\n";
    if (!conf.vlogArgs.isEmpty())  ts << "@vlogargs: " << conf.vlogArgs << "\n";
    if (!conf.workLib.isEmpty() && conf.workLib != "work")
        ts << "@work: " << conf.workLib << "\n";

    QFile f(confPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << content;
        f.close();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// defaultConfTemplate — mirrors extension src/core/project.ts defaultConfTemplate()
// ═════════════════════════════════════════════════════════════════════════════
QString defaultConfTemplate(const QString &openedBase, const QStringList &allBases)
{
    QStringList tbFiles;
    QStringList nonTbFiles;
    for (const QString &b : allBases) {
        if (isTbFile(b)) tbFiles << b;
        else             nonTbFiles << b;
    }

    QString detectedTb;
    if (tbFiles.size() == 1) detectedTb = tbFiles[0];
    else if (!tbFiles.isEmpty()) {
        detectedTb = tbFiles.contains(openedBase) ? openedBase : tbFiles[0];
    }

    QString content;
    QTextStream ts(&content);
    ts << "# conf.svconf — MultiHD Launcher configuration\n";
    ts << "#\n";
    ts << "# compile @(all)         — compile all .sv files in dir\n";
    ts << "# compile @(list)        — compile only files in priority list\n";
    ts << "# compile @(3)           — compile top 3 from priority list\n\n";
    ts << "compile @(all)\n\n";

    if (!detectedTb.isEmpty()) {
        ts << "# Explicit testbench file (compiled last, highest priority for vsim)\n";
        ts << "@tb (" << detectedTb << ")\n\n";
    }

    if (!tbFiles.isEmpty()) {
        ts << "# Testbench compile order (compiled after all RTL)\n";
        ts << "@tb priority list:\n";
        for (const QString &b : tbFiles) ts << "  - " << b << "\n";
        ts << "\n";
    }

    ts << "# RTL compile order (compiled first)\n";
    ts << "@priority list encoder:\n";
    if (nonTbFiles.contains(openedBase))
        ts << "  - " << openedBase << "    # opened file\n";
    for (const QString &b : nonTbFiles)
        if (b != openedBase) ts << "  - " << b << "\n";
    ts << "\n";

    const QString topMod = detectedTb.isEmpty() ? qtModuleName(openedBase) : qtModuleName(detectedTb);
    ts << "@topmodule: " << topMod << "\n";
    ts << "# @vsimargs: -voptargs=+acc\n";
    ts << "# @vlogargs: -sv\n";
    ts << "# @work: work\n";
    return content;
}

// ═════════════════════════════════════════════════════════════════════════════
// ConfDialog — editor UI
// ═════════════════════════════════════════════════════════════════════════════
ConfDialog::ConfDialog(const QString &projectDir, QWidget *parent)
    : QDialog(parent), m_projectDir(projectDir)
{
    setWindowTitle("MultiHD Launcher — conf.svconf");
    setFixedSize(660, 640);
    setStyleSheet(CONF_STYLE);
    setModal(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(18, 16, 18, 16);

    // ── Title ──
    QLabel *title = new QLabel("⚙   EDIT conf.svconf");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:14px; font-weight:bold; color:#00e5ff;"
                         "font-family:'Courier New',monospace; letter-spacing:2px;");
    root->addWidget(title);

    QFrame *sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#1a2a3a;");
    root->addWidget(sep);

    m_pathLabel = new QLabel("Project: " + m_projectDir);
    m_pathLabel->setWordWrap(true);
    m_pathLabel->setStyleSheet("color:#446688; font-size:10px;");
    root->addWidget(m_pathLabel);

    // ── Compile mode row ──
    QHBoxLayout *modeRow = new QHBoxLayout();
    modeRow->setSpacing(8);
    QLabel *modeLbl = new QLabel("compile mode");
    m_compileMode = new QComboBox();
    m_compileMode->addItems({"all", "list", "N"});
    m_compileMode->setMinimumWidth(120);
    m_nCompile = new QLineEdit();
    m_nCompile->setPlaceholderText("N  (when compile @(N))");
    m_nCompile->setFixedWidth(190);
    m_nCompile->setEnabled(false);
    modeRow->addWidget(modeLbl);
    modeRow->addWidget(m_compileMode);
    modeRow->addWidget(m_nCompile);
    modeRow->addStretch();
    root->addLayout(modeRow);

    connect(m_compileMode, &QComboBox::currentTextChanged, this, [this](const QString &t){
        m_nCompile->setEnabled(t == "N");
    });

    // ── Directives group ──
    QGroupBox *scalarsBox = new QGroupBox("DIRECTIVES");
    QGridLayout *grid = new QGridLayout(scalarsBox);
    grid->setSpacing(6);
    grid->setContentsMargins(10, 8, 10, 10);
    grid->setColumnStretch(1, 1);
    auto makeLbl = [](const QString &t){
        QLabel *l = new QLabel(t);
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };

    m_tbName    = new QLineEdit();
    m_topModule = new QLineEdit();
    m_vsimArgs  = new QLineEdit();
    m_vlogArgs  = new QLineEdit();
    m_work      = new QLineEdit();

    grid->addWidget(makeLbl("@tb (name)"),    0, 0);
    grid->addWidget(m_tbName,                 0, 1);
    grid->addWidget(makeLbl("@topmodule:"),   1, 0);
    grid->addWidget(m_topModule,              1, 1);
    grid->addWidget(makeLbl("@vsimargs:"),    2, 0);
    grid->addWidget(m_vsimArgs,               2, 1);
    grid->addWidget(makeLbl("@vlogargs:"),    3, 0);
    grid->addWidget(m_vlogArgs,               3, 1);
    grid->addWidget(makeLbl("@work:"),        4, 0);
    grid->addWidget(m_work,                   4, 1);
    root->addWidget(scalarsBox);

    // ── Order group ──
    QGroupBox *listsBox = new QGroupBox("ORDER");
    QVBoxLayout *listsV = new QVBoxLayout(listsBox);
    listsV->setSpacing(6);
    listsV->setContentsMargins(10, 8, 10, 10);

    QLabel *plLbl = new QLabel("@priority list encoder:   (one file per line, compiled first)");
    m_priorityList = new QTextEdit();
    m_priorityList->setPlaceholderText("- rtl_file_a.sv\n- rtl_file_b.sv");
    m_priorityList->setFixedHeight(90);
    listsV->addWidget(plLbl);
    listsV->addWidget(m_priorityList);

    QLabel *tplLbl = new QLabel("@tb priority list:   (testbench order, compiled after all RTL)");
    m_tbPriorityList = new QTextEdit();
    m_tbPriorityList->setPlaceholderText("- tb_a.sv\n- tb_b.sv");
    m_tbPriorityList->setFixedHeight(90);
    listsV->addWidget(tplLbl);
    listsV->addWidget(m_tbPriorityList);
    root->addWidget(listsBox);

    // ── Buttons ──
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    m_templateBtn = new QPushButton("✦   CREATE TEMPLATE");
    m_templateBtn->setObjectName("templateBtn");
    m_templateBtn->setFixedHeight(38);
    auto *cancelBtn = new QPushButton("Cancel"); cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setFixedHeight(38);
    m_saveBtn = new QPushButton("✓   SAVE"); m_saveBtn->setObjectName("saveBtn");
    m_saveBtn->setFixedHeight(38);
    btnRow->addWidget(m_templateBtn, 1);
    btnRow->addWidget(cancelBtn, 1);
    btnRow->addWidget(m_saveBtn, 2);
    root->addLayout(btnRow);

    connect(m_templateBtn, &QPushButton::clicked, this, &ConfDialog::createTemplate);
    connect(cancelBtn,     &QPushButton::clicked, this, &QDialog::reject);
    connect(m_saveBtn,     &QPushButton::clicked, this, &ConfDialog::saveConfFile);

    loadFromFile();
}

// ── populate widgets from a parsed SvConf ─────────────────────────────────────
void ConfDialog::populateFromConf(const SvConf &conf)
{
    if (conf.compileMode == "list")      m_compileMode->setCurrentText("list");
    else if (conf.compileMode == "all")  m_compileMode->setCurrentText("all");
    else {
        m_compileMode->setCurrentText("N");
        m_nCompile->setText(conf.compileMode);
        m_nCompile->setEnabled(true);
    }
    m_tbName->setText(conf.tbName);
    m_topModule->setText(conf.topModule);
    m_vsimArgs->setText(conf.vsimArgs);
    m_vlogArgs->setText(conf.vlogArgs);
    m_work->setText(conf.workLib == "work" ? "" : conf.workLib);

    m_priorityList->clear();
    for (const QString &f : conf.priorityList)
        m_priorityList->append(f.startsWith("- ") ? f : ("- " + f));

    m_tbPriorityList->clear();
    for (const QString &f : conf.tbPriorityList)
        m_tbPriorityList->append(f.startsWith("- ") ? f : ("- " + f));
}

// ── collect widgets back into an SvConf ───────────────────────────────────────
SvConf ConfDialog::collectFromWidgets() const
{
    SvConf conf;
    const QString mode = m_compileMode->currentText();
    if (mode == "N") {
        bool ok = false;
        int n = m_nCompile->text().trimmed().toInt(&ok);
        conf.compileMode = (ok && n > 0) ? QString::number(n) : "all";
    } else {
        conf.compileMode = mode;
    }
    conf.tbName = m_tbName->text().trimmed();
    conf.topModule = m_topModule->text().trimmed();
    conf.vsimArgs = m_vsimArgs->text().trimmed();
    conf.vlogArgs = m_vlogArgs->text().trimmed();
    conf.workLib = m_work->text().trimmed().isEmpty() ? "work" : m_work->text().trimmed();

    conf.priorityList.clear();
    for (const QString &raw : m_priorityList->toPlainText().split('\n')) {
        QString l = raw.trimmed();
        if (l.isEmpty() || l.startsWith('#')) continue;
        l.remove(QRegularExpression("^[-\\d.]\\s*"));
        l = l.trimmed();
        if (!l.isEmpty() && !conf.priorityList.contains(l)) conf.priorityList << l;
    }

    conf.tbPriorityList.clear();
    for (const QString &raw : m_tbPriorityList->toPlainText().split('\n')) {
        QString l = raw.trimmed();
        if (l.isEmpty() || l.startsWith('#')) continue;
        l.remove(QRegularExpression("^[-\\d.]\\s*"));
        l = l.trimmed();
        if (!l.isEmpty() && !conf.tbPriorityList.contains(l)) conf.tbPriorityList << l;
    }
    return conf;
}

// ── load existing file (or defaults if missing) ───────────────────────────────
void ConfDialog::loadFromFile()
{
    SvConf conf = parseConf(confPathForProject(m_projectDir));
    populateFromConf(conf);
}

// ── create template action ────────────────────────────────────────────────────
void ConfDialog::createTemplate()
{
    QStringList allBases;
    QDir d(m_projectDir);
    const QStringList all = d.entryList(QStringList() << "*.sv", QDir::Files);
    for (const QString &f : all) if (!f.startsWith('.')) allBases << f;
    allBases.sort();

    QString openedBase = allBases.isEmpty() ? QString() : allBases.first();
    const QString content = defaultConfTemplate(openedBase, allBases);

    QFile f(confPathForProject(m_projectDir));
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << content;
        f.close();
    }
    loadFromFile();
}

// ── save action ───────────────────────────────────────────────────────────────
void ConfDialog::saveConfFile()
{
    const SvConf conf = collectFromWidgets();
    saveConf(confPathForProject(m_projectDir), conf);
    accept();
}