#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QStringList>

class PriorityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PriorityDialog(const QStringList &files, QWidget *parent = nullptr);
    QStringList orderedFiles() const;

private slots:
    void onItemClicked(QListWidgetItem *item);
    void reverseOrder();
    void resetOrder();

private:
    QListWidget  *m_list;
    QPushButton  *m_reverseBtn;
    QPushButton  *m_resetBtn;
    QPushButton  *m_doneBtn;
    QStringList   m_allFiles;
    QStringList   m_ordered;   // files in user-selected priority order

    void rebuildList();
    void updateBadges();
};
