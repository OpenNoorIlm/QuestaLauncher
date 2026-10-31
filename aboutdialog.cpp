#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QGraphicsDropShadowEffect>

// ── dark-cyber palette, matching the main window ─────────────────────────────
static const QString ABOUT_STYLE = R"(
    QDialog, QWidget { background-color: #0d0d1a; color: #e0e0e0; }
    QLabel {
        font-family: 'Courier New', monospace; font-size: 12px; color: #8899bb;
    }
    QLabel#title {
        color: #00e5ff; font-size: 16px; font-weight: bold; letter-spacing: 1px;
    }
    QLabel#subtitle {
        color: #00b4d8; font-size: 11px; letter-spacing: 1px;
    }
    QLabel#version {
        color: #446688; font-size: 11px;
    }
    QLabel#section {
        color: #ffaa00; font-size: 11px; font-weight: bold;
    }
    QLabel#feature {
        color: #00ff88; font-size: 11px;
    }
    QLabel#url {
        color: #00e5ff; font-size: 10px; text-decoration: underline;
    }
    QPushButton#closeBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0057a8,stop:1 #00b4d8);
        color: white; border: none; border-radius: 7px;
        font-size: 12px; font-weight: bold; letter-spacing: 1px;
        padding: 8px 28px;
    }
    QPushButton#closeBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0070cc,stop:1 #00d4f0);
    }
    QFrame#rule { color: #1a2a3a; }
)";

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("About MultiHD Launcher");
    setMinimumSize(460, 480);
    setStyleSheet(ABOUT_STYLE);
    setModal(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(22, 18, 22, 18);

    // Logo (multihd_launcher.svg via resources, like the main window)
    QSvgRenderer renderer(QString(":/multihd_launcher.svg"));
    QPixmap logoPix(96, 96);
    logoPix.fill(Qt::transparent);
    QPainter p(&logoPix);
    renderer.render(&p);
    p.end();

    QLabel *logo = new QLabel();
    logo->setPixmap(logoPix);
    logo->setAlignment(Qt::AlignCenter);
    root->addWidget(logo);

    QLabel *title = new QLabel("MultiHD Launcher");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    QLabel *subtitle = new QLabel("QUESTA FSE  •  QUARTUS PRIME  •  GOWIN EDA");
    subtitle->setObjectName("subtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    root->addWidget(subtitle);

    QLabel *version = new QLabel("Version 1.3.0");
    version->setObjectName("version");
    version->setAlignment(Qt::AlignCenter);
    root->addWidget(version);

    QFrame *line = new QFrame();
    line->setObjectName("rule");
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #1a2a3a;");
    root->addWidget(line);

    QLabel *featHead = new QLabel("Features");
    featHead->setObjectName("section");
    root->addWidget(featHead);

    const QStringList features = {
        "⬡  Questa FSE / Modelsim — run.do generation + license guard",
        "◈  Quartus Prime — .qpf / .qsf / .sdc project generation",
        "◆  Gowin EDA — interactive .cst pin editor + .gprj / .f",
        "⇅  Compile order control",
        "✕  Per-file exclusion",
        "📦  Library manager (global registry + per-project deps)",
        "⚙  Persistent configuration + conf.svconf",
        "🎮  Run GUI / Run CLI / Show WaveForm launch modes",
    };
    for (const QString &f : features) {
        QLabel *lbl = new QLabel(f);
        lbl->setObjectName("feature");
        root->addWidget(lbl);
    }

    root->addStretch();

    QLabel *urlHead = new QLabel("Resources");
    urlHead->setObjectName("section");
    root->addWidget(urlHead);

    QLabel *qUrl = new QLabel(
        "Questa FSE download:\n"
        "https://www.intel.com/content/www/us/en/software-kit/825277/"
        "questa-intel-fpgas-standard-edition-software-version-23-1-1.html");
    qUrl->setObjectName("url");
    qUrl->setWordWrap(true);
    qUrl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(qUrl);

    QLabel *lUrl = new QLabel(
        "Intel licensing portal:\n"
        "https://licensing.intel.com/psg/s/sales-signup-evaluationlicenses");
    lUrl->setObjectName("url");
    lUrl->setWordWrap(true);
    lUrl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(lUrl);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    QPushButton *closeBtn = new QPushButton("✓   CLOSE");
    closeBtn->setObjectName("closeBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
}