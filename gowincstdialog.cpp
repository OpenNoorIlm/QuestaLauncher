#include "gowincstdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

// ── style ─────────────────────────────────────────────────────────────────────
static const QString CST_STYLE = R"(
    QDialog, QWidget { background-color: #0d0d1a; color: #e0e0e0; }
    QLabel {
        font-family: 'Courier New', monospace; font-size: 11px; color: #8899bb;
    }
    QLabel#title {
        color: #88ff88; font-size: 13px; font-weight: bold; letter-spacing: 2px;
    }
    QLabel#hint {
        color: #334455; font-size: 10px; font-family: 'Courier New', monospace;
    }
    QPlainTextEdit {
        background-color: #080812; color: #88ff88;
        border: 1px solid #224422; border-radius: 6px;
        font-family: 'Courier New', monospace; font-size: 11px; padding: 6px;
    }
    QComboBox {
        background-color: #131326; color: #88ff88;
        border: 1px solid #44aa44; border-radius: 5px;
        padding: 4px 8px; font-family: 'Courier New', monospace; font-size: 11px;
    }
    QComboBox::drop-down { border: none; }
    QComboBox QAbstractItemView {
        background-color: #131326; color: #88ff88;
        selection-background-color: #1a3a1a;
    }
    QGroupBox {
        border: 1px solid #1a2a1a; border-radius: 6px;
        margin-top: 8px; padding-top: 6px;
        font-family: 'Courier New', monospace; font-size: 10px; color: #446644;
    }
    QGroupBox::title {
        subcontrol-origin: margin; subcontrol-position: top left;
        padding: 0 4px; left: 10px; color: #446644;
        font-family: 'Courier New', monospace; font-size: 10px; letter-spacing: 1px;
    }
    QPushButton#detectBtn {
        background-color: transparent; color: #88ff88;
        border: 1px solid #44aa44; border-radius: 5px;
        padding: 5px 12px; font-family: 'Courier New', monospace; font-size: 11px;
    }
    QPushButton#detectBtn:hover { background-color: #44aa4418; }
    QPushButton#saveBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1a5a1a,stop:1 #228822);
        color: white; border: none; border-radius: 7px;
        font-size: 13px; font-weight: bold; letter-spacing: 1px; padding: 8px 0;
    }
    QPushButton#saveBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #226622,stop:1 #33aa33);
    }
    QPushButton#cancelBtn {
        background-color: transparent; color: #556688;
        border: 1px solid #223344; border-radius: 7px;
        font-size: 13px; padding: 8px 0;
        font-family: 'Courier New', monospace;
    }
    QPushButton#cancelBtn:hover { color: #8899bb; border-color: #446688; }
)";

// ── device table ──────────────────────────────────────────────────────────────
struct GowinDevice {
    const char *label;      // shown in combo
    const char *name;       // XML name= attribute
    const char *pn;         // XML pn= attribute (identical for all known ones)
};

static const GowinDevice GOWIN_DEVICES[] = {
    { "GW1NR-9  (Tang Nano 9K)",  "GW1NR-LV9QN88PC6/I5",  "GW1NR-LV9QN88PC6/I5"  },
    { "GW1N-1   (Tang Nano 1K)",  "GW1N-LV1QN48C6/I5",    "GW1N-LV1QN48C6/I5"    },
    { "GW1NSR-4 (Tang Nano 4K)",  "GW1NSR-LV4CQN48PC7/I6","GW1NSR-LV4CQN48PC7/I6"},
    { "GW2AR-18 (Tang Nano 20K)", "GW2AR-LV18QN88C8/I7",  "GW2AR-LV18QN88C8/I7"  },
    { "GW1N-4   (Tang Primer 4K)","GW1N-LV4LQ144C6/I5",   "GW1N-LV4LQ144C6/I5"   },
    { "GW5A-25  (Tang Primer 25K)","GW5A-LV25MG121NES",   "GW5A-LV25MG121NES"     },
    { "Custom device …",          "",                       ""                      },
};
static const int GOWIN_DEVICE_COUNT =
    static_cast<int>(sizeof(GOWIN_DEVICES) / sizeof(GOWIN_DEVICES[0]));

// ── constructor ───────────────────────────────────────────────────────────────
GowinCstDialog::GowinCstDialog(const QString &projectDir,
                               const QString &topModuleFile,
                               QWidget       *parent)
    : QDialog(parent), m_svFile(topModuleFile), m_projectDir(projectDir)
{
    setWindowTitle("MultiHD Launcher — Gowin Constraints (.cst)");
    setFixedSize(680, 580);
    setStyleSheet(CST_STYLE);
    setModal(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(18, 16, 18, 16);

    // ── Title ─────────────────────────────────────────────────────────────────
    auto *titleLbl = new QLabel("◆   GOWIN CONSTRAINT FILE (.cst)");
    titleLbl->setObjectName("title");
    titleLbl->setAlignment(Qt::AlignCenter);
    root->addWidget(titleLbl);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#1a2a1a;");
    root->addWidget(sep);

    // ── Device selector ───────────────────────────────────────────────────────
    QGroupBox *devBox = new QGroupBox("TARGET DEVICE");
    QHBoxLayout *devRow = new QHBoxLayout(devBox);
    devRow->setContentsMargins(10, 6, 10, 8);
    devRow->setSpacing(10);
    devRow->addWidget(new QLabel("Device:"));
    m_deviceCombo = new QComboBox();
    for (int i = 0; i < GOWIN_DEVICE_COUNT; ++i)
        m_deviceCombo->addItem(GOWIN_DEVICES[i].label);
    devRow->addWidget(m_deviceCombo, 1);
    root->addWidget(devBox);

    // ── Editor group ──────────────────────────────────────────────────────────
    QGroupBox *edBox = new QGroupBox("CONSTRAINT EDITOR  —  edit LOC numbers to match your board's pin map");
    QVBoxLayout *edLayout = new QVBoxLayout(edBox);
    edLayout->setContentsMargins(8, 6, 8, 8);
    edLayout->setSpacing(6);

    m_editor = new QPlainTextEdit();
    m_editor->setPlaceholderText(
        "// Click 'Auto-detect ports' to generate a constraint template\n"
        "// from the top-module .sv file, then fill in the LOC numbers.\n"
        "//\n"
        "// Example:\n"
        "//   IO_LOC \"clk\"    52 ;\n"
        "//   IO_PORT \"clk\"   PULL_MODE=UP DRIVE=8 IOSTD=LVCMOS33 ;\n");
    m_editor->setMinimumHeight(280);
    edLayout->addWidget(m_editor);

    m_hintLabel = new QLabel();
    m_hintLabel->setObjectName("hint");
    m_hintLabel->setWordWrap(true);
    edLayout->addWidget(m_hintLabel);

    root->addWidget(edBox, 1);

    // ── Action row ────────────────────────────────────────────────────────────
    QHBoxLayout *actRow = new QHBoxLayout();
    actRow->setSpacing(8);

    auto *detectBtn = new QPushButton("⟳  Auto-detect ports from .sv");
    detectBtn->setObjectName("detectBtn");
    detectBtn->setFixedHeight(34);
    actRow->addWidget(detectBtn);
    actRow->addStretch();

    auto *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setFixedHeight(34);
    cancelBtn->setFixedWidth(90);

    auto *saveBtn = new QPushButton("✓  SAVE  .cst");
    saveBtn->setObjectName("saveBtn");
    saveBtn->setFixedHeight(34);
    saveBtn->setFixedWidth(140);

    actRow->addWidget(cancelBtn);
    actRow->addWidget(saveBtn);
    root->addLayout(actRow);

    // ── Wire up ───────────────────────────────────────────────────────────────
    connect(detectBtn,       &QPushButton::clicked, this, &GowinCstDialog::autoDetectPorts);
    connect(m_deviceCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GowinCstDialog::autoDetectPorts);
    connect(cancelBtn,       &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn,         &QPushButton::clicked, this, &QDialog::accept);

    // Trigger first auto-detect if we have an sv file
    if (!m_svFile.isEmpty())
        autoDetectPorts();
}

// ── accessors ─────────────────────────────────────────────────────────────────
QString GowinCstDialog::cstContent() const
{
    return m_editor->toPlainText();
}

QString GowinCstDialog::deviceName() const
{
    int i = m_deviceCombo->currentIndex();
    if (i < 0 || i >= GOWIN_DEVICE_COUNT) return QString();
    return QString(GOWIN_DEVICES[i].name);
}

QString GowinCstDialog::devicePn() const
{
    int i = m_deviceCombo->currentIndex();
    if (i < 0 || i >= GOWIN_DEVICE_COUNT) return QString();
    return QString(GOWIN_DEVICES[i].pn);
}

// ── auto-detect ports ─────────────────────────────────────────────────────────
void GowinCstDialog::autoDetectPorts()
{
    if (m_svFile.isEmpty()) {
        m_hintLabel->setText("No top-module file set — type constraints manually.");
        return;
    }

    QList<Port> ports = parsePorts(m_svFile);
    if (ports.isEmpty()) {
        m_hintLabel->setText(
            "⚠  No ports detected in " + QFileInfo(m_svFile).fileName() +
            "  — the file may use an unsupported port style. Edit manually.");
        // Still regenerate with empty list so header/comments are fresh
    } else {
        m_hintLabel->setText(
            QString("✓  %1 port(s) detected from %2  —  fill in the LOC numbers.")
                .arg(ports.size())
                .arg(QFileInfo(m_svFile).fileName()));
    }

    QString dev = deviceName();
    m_editor->setPlainText(buildTemplate(ports, dev));
}

// ── port parser ───────────────────────────────────────────────────────────────
QList<GowinCstDialog::Port> GowinCstDialog::parsePorts(const QString &svPath)
{
    QList<Port> result;
    QFile f(svPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return result;
    QString text = QTextStream(&f).readAll();

    // Strip block comments  /* … */
    text.replace(QRegularExpression(R"(/\*.*?\*/)"), " ");
    // Strip line comments   // …
    text.replace(QRegularExpression(R"(//[^\n]*)"), " ");

    // Match: (input|output|inout) [wire|logic|reg]? [width]? portname [,|)]
    QRegularExpression portRe(
        R"(\b(input|output|inout)\s+(?:wire|logic|reg|signed|unsigned)?\s*)"
        R"((\[\s*[\w\s\+\-\*:/]+\s*:\s*[\w\s\+\-\*:/]+\s*\])?\s*)"
        R"((\w+)\s*(?:[,;\)]))",
        QRegularExpression::CaseInsensitiveOption
    );

    auto it = portRe.globalMatch(text);
    QSet<QString> seen;
    while (it.hasNext()) {
        auto m = it.next();
        QString dir   = m.captured(1).toLower();
        QString width = m.captured(2).trimmed();
        QString name  = m.captured(3);

        // Skip keywords that aren't port names
        static const QSet<QString> kw = {
            "module","endmodule","begin","end","if","else","always","assign",
            "wire","logic","reg","parameter","localparam","integer","real"
        };
        if (kw.contains(name.toLower())) continue;
        if (seen.contains(name)) continue;
        seen.insert(name);

        result.append({name, dir, width});
    }
    return result;
}

// ── template builder ──────────────────────────────────────────────────────────
QString GowinCstDialog::buildTemplate(const QList<Port> &ports, const QString &device)
{
    QString out;
    QTextStream ts(&out);

    ts << "// Gowin Physical Constraint File (.cst)\n";
    ts << "// Generated by MultiHD Launcher\n";
    if (!device.isEmpty())
        ts << "// Device: " << device << "\n";
    ts << "//\n";
    ts << "// Replace the placeholder pin numbers (XX) with actual LOC values\n";
    ts << "// from your board's schematic / pin map.\n";
    ts << "// Reference: Gowin EDA User Guide — Physical Constraints\n";
    ts << "\n";

    if (ports.isEmpty()) {
        ts << "// No ports auto-detected — add constraints manually:\n";
        ts << "//\n";
        ts << "// IO_LOC  \"port_name\" XX ;\n";
        ts << "// IO_PORT \"port_name\" PULL_MODE=UP DRIVE=8 IOSTD=LVCMOS33 ;\n";
        return out;
    }

    // Group by direction for readability
    auto emitGroup = [&](const QString &dir, const QString &comment) {
        bool any = false;
        for (const Port &p : ports) if (p.direction == dir) { any = true; break; }
        if (!any) return;

        ts << "// ── " << comment << " ──\n";
        for (const Port &p : ports) {
            if (p.direction != dir) continue;
            QString wStr = p.width.isEmpty() ? "" : " " + p.width;
            ts << "IO_LOC  \"" << p.name << "\"" << wStr << "  XX ;\n";
            ts << "IO_PORT \"" << p.name << "\""
               << "  PULL_MODE=NONE  DRIVE=8  IOSTD=LVCMOS33 ;\n";
        }
        ts << "\n";
    };

    emitGroup("input",  "Inputs");
    emitGroup("output", "Outputs");
    emitGroup("inout",  "Bidirectional");

    return out;
}

void GowinCstDialog::updatePreviewLabel() { /* kept for future use */ }
