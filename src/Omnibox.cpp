#include "Omnibox.h"
#include <QStyle>

Omnibox::Omnibox(QWidget *parent) : QWidget(parent) {
    m_lockLabel = new QLabel(this);
    m_lockLabel->setObjectName("lockIcon");
    m_lockLabel->setFixedSize(20, 20);
    m_lockLabel->setAlignment(Qt::AlignCenter);
    m_lockLabel->setVisible(false);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(false);
    m_lineEdit->setPlaceholderText("Pesquisar ou digitar endereço…");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(4);
    layout->addWidget(m_lockLabel);
    layout->addWidget(m_lineEdit, 1);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusProxy(m_lineEdit);

    connect(m_lineEdit, &QLineEdit::returnPressed,
            this, &Omnibox::onLineEditReturnPressed);
}

QString Omnibox::text() const {
    return m_lineEdit->text();
}

void Omnibox::setText(const QString &text) {
    m_lineEdit->setText(text);
}

void Omnibox::setLockIcon(const QString &icon, const QString &tooltip) {
    m_lockLabel->setText(icon);
    m_lockLabel->setToolTip(tooltip);
    m_lockLabel->setVisible(true);
}

void Omnibox::clearLockIcon() {
    m_lockLabel->clear();
    m_lockLabel->setToolTip(QString());
    m_lockLabel->setVisible(false);
}

void Omnibox::onLineEditReturnPressed() {
    emit returnPressed();
}