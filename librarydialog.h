#pragma once
#include "librarymanager.h"
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class LibraryDialog : public QDialog
{
    Q_OBJECT
public:
    // projectDir may be empty if no folder is selected yet
    explicit LibraryDialog(LibraryManager *mgr,
                           const QString  &projectDir,
                           const QString  &vsimBinaryPath,
                           QWidget        *parent = nullptr);

private slots:
    void addLibrary();
    void removeLibrary();
    void compileLibrary();
    void toggleProjectDep(QListWidgetItem *item);
    void refreshGlobalList();
    void refreshProjectList();

private:
    LibraryManager *m_mgr;
    QString         m_projectDir;
    QString         m_vsimBin;

    // Global registry panel
    QListWidget *m_globalList;
    QPushButton *m_addBtn;
    QPushButton *m_removeBtn;
    QPushButton *m_compileBtn;

    // Per-project panel
    QListWidget *m_projectList;   // checkable — ticked = this project uses it
    QLabel      *m_projectNote;

    QLabel      *m_statusLabel;

    void status(const QString &msg, bool error = false);
};
