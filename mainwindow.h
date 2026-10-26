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
    void launch();

private:
    QLineEdit   *m_pathEdit;
    QPushButton *m_browseBtn;
    QPushButton *m_priorityBtn;
    QPushButton *m_launchBtn;
    QTextEdit   *m_log;
    QLabel      *m_logoLabel;
    QLabel      *m_subtitleLabel;
    QStringList  m_customOrder;   // user-defined compile order

    QString generateRunDo(const QString &dir);
    void    log(const QString &msg);
};
