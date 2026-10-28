#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QProcess>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPaintEvent>
#include <QStringList>
#include <QSettings>
#include <QScrollArea>
#include <QGroupBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void browseFolder();
    void openPriorityDialog();
    void openConfig();
    void openExcludeDialog();
    void restoreFile(const QString &filename);
    void launch();
    void installQuesta();
    void openLicenseRequest();

private:
    QLineEdit   *m_pathEdit;
    QPushButton *m_browseBtn;
    QPushButton *m_priorityBtn;
    QPushButton *m_configBtn;
    QPushButton *m_excludeBtn;      // opens ExcludeDialog
    QPushButton *m_launchBtn;
    QTextEdit   *m_log;
    QLabel      *m_logoLabel;
    QLabel      *m_subtitleLabel;
    QLabel      *m_licenseStatusLabel;
    QStringList  m_customOrder;

    // Per-file exclude list (filenames only, relative to project folder)
    QStringList  m_excludedFiles;

    // The scrollable chip area inside the EXCLUDED FILES group
    QGroupBox   *m_excludedBox;
    QWidget     *m_chipContainer;
    QVBoxLayout *m_chipLayout;

    // Install buttons
    QPushButton *m_installBtn;
    QPushButton *m_licenseBtn;

    // QSettings helpers
    QString settingVsim()          const;
    QString settingLicenseFile()   const;
    QString settingSaltServer()    const;
    QString settingMtiVcoMode()    const;
    QString settingDefaultFolder() const;

    void updateLicenseStatus();
    void rebuildExcludedChips();   // refreshes the chip panel

    // Returns all .sv files in dir minus excluded ones
    QStringList activeFiles(const QString &dir) const;

    QString generateRunDo(const QString &dir);
    void    log(const QString &msg);
};
