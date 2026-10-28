#include "excludedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>

static const QString EXCL_STYLE = R"(
    QDialog, QWidget { background-color: #0d0d1a; color: #e0e0e0; }
    QListWidget {
        background-color: #080812; color: #e0e0e0;
        border: 1px solid #1a1a3a; border-radius: 6px;
        font-family: 'Courier New', monospace; font-size: 12px;
        padding: 4px;
    }
    QListWidget::item { padding: 4px 6px; border-radius: 3px; }
    QListWidget::item:hover    { background-color: #1a1a3a; }
    QListWidget::item:selected { background-color: #1a1a3a; color: #00e5ff; }
    QLabel#title {
        color: #00e5ff; font-size: 13px; font-weight: bold;
        font-family: 'Courier New', monospace; letter-spacing: 1px;
    }
    QLabel#hint {
        color: #334455; font-size: 10px;
        font-family: 'Courier New', monospace;
    }
    QPushButton#applyBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0057a8,stop:1 #00b4d8);
        color: white; border: none; border-radius: 7px;
        font-size: 13px; font-weight: bold; padding: 8px 0;
        font-family: 'Courier New', monospace;
    }
    QPushButton#applyBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #0070cc,stop:1 #00d4f0);
    }
    QPushButton#cancelBtn {
        background-color: transparent; color: #556688;
        border: 1px solid #223344; border-radius: 7px;
        font-size: 13px; padding: 8px 0;
        font-family: 'Courier New', monospace;
    }
    QPushButton#cancelBtn:hover { color: #8899bb; border-color: #446688; }
    QPushButton#selBtn {
        background-color: transparent; color: #cc6666;
        border: 1px solid #cc4444; border-radius: 5px;
        font-size: 10px; padding: 3px 8px;
        font-family: 'Courier New', monospace;
    }
    QPushButton#selBtn:hover { background-color: #cc444418; }
    QPushButton#clrBtn {
        background-color: transparent; color: #00ff88;
        border: 1px solid #00ff88; border-radius: 5px;
        font-size: 10px; padding: 3px 8px;
        font-family: 'Courier New', monospace;
    }
    QPushButton#clrBtn:hover { background-color: #00ff8818; }
)";

ExcludeDialog::ExcludeDialog(const QStringList &allFiles,
                             const QStringList &currentlyExcluded,
                             QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Disinclude Files");
    setFixedSize(420, 460);
    setStyleSheet(EXCL_STYLE);
    setModal(true);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(16, 14, 16, 14);

    QLabel *title = new QLabel("✕   DISINCLUDE FILES");
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    QLabel *hint = new QLabel("Checked files are excluded from compilation and the compile-order dialog.");
    hint->setObjectName("hint");
    hint->setWordWrap(true);
    hint->setAlignment(Qt::AlignCenter);
    root->addWidget(hint);

    // Quick-select toolbar
    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(6);
    auto *selTb  = new QPushButton("Select all _tb.sv"); selTb->setObjectName("selBtn");
    auto *selAll = new QPushButton("Select all .sv");     selAll->setObjectName("selBtn");
    auto *clrAll = new QPushButton("Clear all");          clrAll->setObjectName("clrBtn");
    toolbar->addWidget(selTb);
    toolbar->addWidget(selAll);
    toolbar->addStretch();
    toolbar->addWidget(clrAll);
    root->addLayout(toolbar);

    m_list = new QListWidget();
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    for (const QString &f : allFiles) {
        auto *item = new QListWidgetItem(f, m_list);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(currentlyExcluded.contains(f) ? Qt::Checked : Qt::Unchecked);
        // dim excluded items
        if (currentlyExcluded.contains(f))
            item->setForeground(QColor(0xcc, 0x44, 0x44));
    }
    root->addWidget(m_list);

    // Update row colour live when checked state changes
    connect(m_list, &QListWidget::itemChanged, [](QListWidgetItem *it) {
        it->setForeground(it->checkState() == Qt::Checked
                          ? QColor(0xcc, 0x44, 0x44)
                          : QColor(0xe0, 0xe0, 0xe0));
    });

    // Quick-select helpers
    connect(selTb, &QPushButton::clicked, [this] {
        for (int i = 0; i < m_list->count(); ++i) {
            auto *it = m_list->item(i);
            if (it->text().endsWith("_tb.sv")) it->setCheckState(Qt::Checked);
        }
    });
    connect(selAll, &QPushButton::clicked, [this] {
        for (int i = 0; i < m_list->count(); ++i)
            m_list->item(i)->setCheckState(Qt::Checked);
    });
    connect(clrAll, &QPushButton::clicked, [this] {
        for (int i = 0; i < m_list->count(); ++i)
            m_list->item(i)->setCheckState(Qt::Unchecked);
    });

    // Buttons
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    auto *cancelBtn = new QPushButton("Cancel"); cancelBtn->setObjectName("cancelBtn");
    auto *applyBtn  = new QPushButton("✓   APPLY"); applyBtn->setObjectName("applyBtn");
    cancelBtn->setFixedHeight(36);
    applyBtn->setFixedHeight(36);
    btnRow->addWidget(cancelBtn, 1);
    btnRow->addWidget(applyBtn, 2);
    root->addLayout(btnRow);

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(applyBtn,  &QPushButton::clicked, this, &QDialog::accept);
}

QStringList ExcludeDialog::excludedFiles() const
{
    QStringList result;
    for (int i = 0; i < m_list->count(); ++i) {
        auto *it = m_list->item(i);
        if (it->checkState() == Qt::Checked)
            result << it->text();
    }
    return result;
}
