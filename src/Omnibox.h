#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

class Omnibox : public QWidget {
    Q_OBJECT
public:
    explicit Omnibox(QWidget *parent = nullptr);

    QString text() const;
    void setText(const QString &text);
    void setLockIcon(const QString &icon, const QString &tooltip = QString());
    void clearLockIcon();

signals:
    void returnPressed();

private slots:
    void onLineEditReturnPressed();

private:
    QLabel *m_lockLabel;
    QLineEdit *m_lineEdit;
};