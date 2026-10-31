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
#include "librarymanager.h"
#include <QCheckBox>
// ── writer-b: launch-mode selector needs QComboBox ───────────────────────────
#include <QComboBox>
// ── writer-c: About/Help dialog (Feature C) ───────────────────────────────────
#include "aboutdialog.h"
// ── writer-a: conf.svconf model (Feature A) ──────────────────────────────────
struct SvConf;   // forward-declared; full def lives in confdialog.h

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
    void openLibraryManager();
    void restoreFile(const QString &filename);
    void launch();
    void launchForQuesta();
    void launchForQuartus();
    void launchForGowin();
    void installQuesta();
    void openLicenseRequest();

    // ── writer-c: About/Help dialog (Feature C) ─────────────────────────────
    void openAbout();

    // ── writer-a: conf.svconf (Feature A) ─────────────────────────────────
    void openConfDialog();

private:
    QLineEdit   *m_pathEdit;
    QPushButton *m_browseBtn;
    QPushButton *m_priorityBtn;
    QPushButton *m_configBtn;
    QPushButton *m_excludeBtn;
    QPushButton *m_libBtn;
    QPushButton *m_launchBtn;
    // ── writer-a: ⚙ EDIT conf.svconf button ─────────────────────────────────
    QPushButton *m_confBtn;
    // ── writer-b: launch-mode selector (Run GUI / Run CLI / Show WaveForm) ────
    QComboBox   *m_launchMode;
    QTextEdit   *m_log;
    QLabel      *m_logoLabel;
    QLabel      *m_subtitleLabel;
    QLabel      *m_licenseStatusLabel;
    QStringList  m_customOrder;

    // Per-file exclude list (filenames only, relative to project folder)
    QStringList  m_excludedFiles;

    // Scrollable chip area inside EXCLUDED FILES group
    QGroupBox   *m_excludedBox;
    QWidget     *m_chipContainer;
    QVBoxLayout *m_chipLayout;

    // Install buttons
    QPushButton *m_installBtn;
    QPushButton *m_licenseBtn;

    // Library manager (owns global registry)
    LibraryManager m_libMgr;

    // Compile For checkboxes
    QCheckBox *m_chkQuesta;
    QCheckBox *m_chkQuartus;
    QCheckBox *m_chkGowin;

    // QSettings helpers  (all stored under MultiHDLauncher organisation)
    QString settingVsim()          const;
    QString settingLicenseFile()   const;
    QString settingSaltServer()    const;
    QString settingMtiVcoMode()    const;
    QString settingDefaultFolder() const;

    void updateLicenseStatus();
    void rebuildExcludedChips();

    // ── writer-b: current launch mode ("gui" | "cli" | "wave") ───────────────
    QString currentLaunchMode() const;

    QStringList activeFiles(const QString &dir) const;

    // Per-tool file generators
    // ── writer-b: added mode param ("gui"|"cli"), gui = Qt byte-parity ───────
    QString generateRunDo(const QString &dir,
                          const QString &mode = "gui");            // Questa
    void    generateQuartusFiles(const QString &dir,               // Quartus
                                 const QStringList &active);
    bool    generateGowinFiles(const QString &dir,                 // Gowin
                               const QStringList &active);

    void log(const QString &msg);
};
