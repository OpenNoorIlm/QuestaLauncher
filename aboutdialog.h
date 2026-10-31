#pragma once
#include <QDialog>

class QLabel;

// ── About / Help dialog (writer-c: Feature C) ─────────────────────────────────
// Dark-cyber styled QDialog showing app info, logo, feature list and the
// install/license URLs (Questa FSE download + Intel licensing portal).
class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);
};