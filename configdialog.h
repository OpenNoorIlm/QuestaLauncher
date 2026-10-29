#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(QWidget *parent = nullptr);

    QString vsimPath()      const;
    QString licenseFile()   const;
    QString saltServer()    const;
    QString mtiVcoMode()    const;
    QString defaultFolder() const;
    QString quartusPath()   const;
    QString gowinPath()     const;

private slots:
    void browseVsim();
    void browseLicense();
    void browseFolder();
    void browseQuartus();
    void browseGowin();
    void validate();

private:
    QLineEdit *m_vsimEdit;
    QLineEdit *m_licenseEdit;
    QLineEdit *m_saltEdit;
    QLineEdit *m_mtiEdit;
    QLineEdit *m_folderEdit;
    QLineEdit *m_quartusEdit;
    QLineEdit *m_gowinEdit;
    QLabel    *m_licenseWarning;

    QPushButton *m_saveBtn;

    void loadSettings();
    void saveSettings();
    bool licenseOk() const;
};
