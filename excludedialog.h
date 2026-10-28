#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>

// Shows all .sv files in a folder; lets the user tick which ones to exclude.
// Accepted result: excludedFiles() returns the files that should be hidden
// from the compile set.
class ExcludeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExcludeDialog(const QStringList &allFiles,
                           const QStringList &currentlyExcluded,
                           QWidget *parent = nullptr);

    QStringList excludedFiles() const;   // files whose checkbox is ticked

private:
    QListWidget *m_list;
};
