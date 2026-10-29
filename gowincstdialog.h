#pragma once
#include <QDialog>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QStringList>

// ─────────────────────────────────────────────────────────────────────────────
// GowinCstDialog — interactive Gowin Physical Constraint (.cst) file editor.
//
// Parses ports from the top-module .sv file, generates a constraint template,
// and lets the user fill in LOC / IOSTANDARD values before writing the file.
// ─────────────────────────────────────────────────────────────────────────────
class GowinCstDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GowinCstDialog(const QString &projectDir,
                            const QString &topModuleFile,
                            QWidget       *parent = nullptr);

    QString cstContent()   const;   // final text as edited by the user
    QString deviceName()   const;   // selected device string
    QString devicePn()     const;   // device PN string (same family-name format)

private slots:
    void autoDetectPorts();
    void updatePreviewLabel();

private:
    QPlainTextEdit *m_editor;
    QComboBox      *m_deviceCombo;
    QLabel         *m_hintLabel;

    QString m_svFile;
    QString m_projectDir;

    // ── helpers ───────────────────────────────────────────────────────────────
    struct Port {
        QString name;
        QString direction;   // input / output / inout
        QString width;       // "" = 1-bit, "[7:0]" etc.
    };

    QList<Port> parsePorts(const QString &svPath);
    QString buildTemplate(const QList<Port> &ports, const QString &device);
};
