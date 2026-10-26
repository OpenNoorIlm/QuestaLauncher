#include "prioritydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QPainter>
#include <QFont>

static const QString DIALOG_STYLE = R"(
    QDialog {
        background-color: #0d0d1a;
    }
    QListWidget {
        background-color: #080812;
        border: 1px solid #1a1a3a;
        border-radius: 6px;
        color: #00e5ff;
        font-family: 'Courier New', monospace;
        font-size: 12px;
        outline: none;
    }
    QListWidget::item {
        padding: 8px 12px;
        border-bottom: 1px solid #1a1a3a;
        border-radius: 4px;
    }
    QListWidget::item:hover  { background-color: #00e5ff18; }
    QListWidget::item:selected { background-color: #00e5ff22; }
    QPushButton#reverseBtn {
        background-color: transparent;
        color: #ffaa00;
        border: 1px solid #ffaa00;
        border-radius: 6px;
        padding: 7px 16px;
        font-family: 'Courier New', monospace;
        font-size: 12px;
    }
    QPushButton#reverseBtn:hover { background-color: #ffaa0022; }
    QPushButton#resetBtn {
        background-color: transparent;
        color: #ff4455;
        border: 1px solid #ff4455;
        border-radius: 6px;
        padding: 7px 16px;
        font-family: 'Courier New', monospace;
        font-size: 12px;
    }
    QPushButton#resetBtn:hover { background-color: #ff445522; }
    QPushButton#doneBtn {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #0057a8, stop:1 #00b4d8);
        color: white;
        border: none;
        border-radius: 6px;
        padding: 7px 24px;
        font-size: 13px;
        font-weight: bold;
        font-family: 'Courier New', monospace;
    }
    QPushButton#doneBtn:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 #0070cc, stop:1 #00d4f0);
    }
    QLabel#heading {
        color: #00e5ff;
        font-family: 'Courier New', monospace;
        font-size: 15px;
        font-weight: bold;
        letter-spacing: 3px;
    }
    QLabel#hint {
        color: #446688;
        font-family: 'Courier New', monospace;
        font-size: 10px;
        letter-spacing: 1px;
    }
)";

PriorityDialog::PriorityDialog(const QStringList &files, QWidget *parent)
    : QDialog(parent), m_allFiles(files)
{
    setWindowTitle("Compile Order");
    setFixedSize(420, 460);
    setStyleSheet(DIALOG_STYLE);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(16, 16, 16, 16);

    QLabel *heading = new QLabel("⬡  COMPILE ORDER");
    heading->setObjectName("heading");
    heading->setAlignment(Qt::AlignCenter);

    QLabel *hint = new QLabel("CLICK FILE TO SET PRIORITY  •  CLICK AGAIN TO REMOVE");
    hint->setObjectName("hint");
    hint->setAlignment(Qt::AlignCenter);

    layout->addWidget(heading);
    layout->addWidget(hint);

    m_list = new QListWidget();
    m_list->setDragDropMode(QAbstractItemView::InternalMove);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(m_list);

    // Buttons row
    QHBoxLayout *btns = new QHBoxLayout();
    m_reverseBtn = new QPushButton("⇅  REVERSE");
    m_reverseBtn->setObjectName("reverseBtn");
    m_resetBtn   = new QPushButton("✕  RESET");
    m_resetBtn->setObjectName("resetBtn");
    m_doneBtn    = new QPushButton("✓  DONE");
    m_doneBtn->setObjectName("doneBtn");

    btns->addWidget(m_reverseBtn);
    btns->addWidget(m_resetBtn);
    btns->addStretch();
    btns->addWidget(m_doneBtn);
    layout->addLayout(btns);

    rebuildList();

    connect(m_list,       &QListWidget::itemClicked, this, &PriorityDialog::onItemClicked);
    connect(m_reverseBtn, &QPushButton::clicked,     this, &PriorityDialog::reverseOrder);
    connect(m_resetBtn,   &QPushButton::clicked,     this, &PriorityDialog::resetOrder);
    connect(m_doneBtn,    &QPushButton::clicked,     this, &QDialog::accept);
}

void PriorityDialog::rebuildList()
{
    m_list->clear();
    for (const QString &f : m_allFiles) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, f);

        int idx = m_ordered.indexOf(f);
        if (idx >= 0) {
            QString badge = QString("[%1]").arg(idx + 1, 2, 10, QChar('0'));
            item->setText(badge + "  " + f);
            item->setForeground(QColor("#00ff88"));
        } else {
            item->setText("[ ]  " + f);
            item->setForeground(QColor("#446688"));
        }
        m_list->addItem(item);
    }
}

void PriorityDialog::onItemClicked(QListWidgetItem *item)
{
    QString f = item->data(Qt::UserRole).toString();
    if (m_ordered.contains(f))
        m_ordered.removeAll(f);    // deselect
    else
        m_ordered.append(f);       // add next priority
    rebuildList();
}

void PriorityDialog::reverseOrder()
{
    std::reverse(m_ordered.begin(), m_ordered.end());
    // Also reverse unselected in the full list view
    QStringList unselected;
    for (const QString &f : m_allFiles)
        if (!m_ordered.contains(f)) unselected.prepend(f);

    QStringList newAll;
    for (int i = m_allFiles.size()-1; i >= 0; i--)
        newAll << m_allFiles[i];
    m_allFiles = newAll;

    rebuildList();
}

void PriorityDialog::resetOrder()
{
    m_ordered.clear();
    rebuildList();
}

QStringList PriorityDialog::orderedFiles() const
{
    // Return: selected files in priority order, then unselected ones after
    QStringList result = m_ordered;
    for (const QString &f : m_allFiles)
        if (!result.contains(f)) result.append(f);
    return result;
}
