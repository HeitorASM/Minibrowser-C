#include "HomepageWidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QDateTime>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QColor>

HomepageWidget::HomepageWidget(QWidget *parent)
    : QWidget(parent), m_timer(new QTimer(this)) {
    setupUi();
    connect(m_timer, &QTimer::timeout, this, &HomepageWidget::updateClock);
    m_timer->setInterval(1000);
    updateClock();
}

HomepageWidget::~HomepageWidget() = default;

void HomepageWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->addStretch();

    m_clockLabel = new QLabel(this);
    m_clockLabel->setObjectName("clockLabel");
    m_clockLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_clockLabel);

    m_dateLabel = new QLabel(this);
    m_dateLabel->setObjectName("dateLabel");
    m_dateLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_dateLabel);

    m_engineBadge = new QLabel(this);
    m_engineBadge->setObjectName("engineBadge");
    m_engineBadge->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_engineBadge);

    // Shortcut grid
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(16);
    grid->setHorizontalSpacing(20);

    // Ícones simplificados (caracteres comuns)
    const struct { QString label; QString url; QString icon; } shortcuts[] = {
        { "GitHub", "https://github.com", "G" },
        { "Wikipedia", "https://wikipedia.org", "W" },
        { "YouTube", "https://youtube.com", "▶" },
        { "Hacker News", "https://news.ycombinator.com", "Y" },
        { "Reddit", "https://reddit.com", "R" },
        { "OpenStreetMap", "https://openstreetmap.org", "O }
    };

    constexpr int shortcutCount = sizeof(shortcuts) / sizeof(shortcuts[0]);
    for (int i = 0; i < shortcutCount; ++i) {
        QPushButton *btn = new QPushButton(this);
        btn->setProperty("shortcut", true);
        btn->setFixedSize(120, 90);
        btn->setCursor(Qt::PointingHandCursor);

        // QSS não suporta box-shadow; usamos QGraphicsDropShadowEffect para o efeito real
        auto *shadow = new QGraphicsDropShadowEffect(btn);
        shadow->setBlurRadius(18);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0, 0, 0, 60));
        btn->setGraphicsEffect(shadow);

        QVBoxLayout *bl = new QVBoxLayout(btn);
        bl->setSpacing(2);
        bl->setContentsMargins(8, 8, 8, 8);

        QLabel *iconLbl = new QLabel(shortcuts[i].icon, btn);
        iconLbl->setProperty("iconLabel", true);
        iconLbl->setAlignment(Qt::AlignCenter);
        QLabel *nameLbl = new QLabel(shortcuts[i].label, btn);
        nameLbl->setProperty("nameLabel", true);
        nameLbl->setAlignment(Qt::AlignCenter);

        bl->addWidget(iconLbl);
        bl->addWidget(nameLbl);

        btn->setProperty("url", shortcuts[i].url);
        connect(btn, &QPushButton::clicked, this, &HomepageWidget::onShortcutClicked);
        grid->addWidget(btn, i / 3, i % 3);
    }

    mainLayout->addLayout(grid);
    mainLayout->addStretch();
}

void HomepageWidget::updateClock() {
    QDateTime now = QDateTime::currentDateTime();
    m_clockLabel->setText(now.toString("hh:mm:ss"));
    m_dateLabel->setText(now.toString("dddd, dd 'de' MMMM 'de' yyyy"));
}

void HomepageWidget::setSearchEngine(SearchEngine engine) {
    m_currentEngine = engine;
    updateEngineBadge();
}

void HomepageWidget::updateEngineBadge() {
    const auto &info = SEARCH_ENGINES[static_cast<int>(m_currentEngine)];
    m_engineBadge->setText(QString("%1 %2 ativo").arg(info.icon, info.name));
}

void HomepageWidget::showEvent(QShowEvent *event) {
    m_timer->start();
    QWidget::showEvent(event);
}

void HomepageWidget::hideEvent(QHideEvent *event) {
    m_timer->stop();
    QWidget::hideEvent(event);
}

void HomepageWidget::onShortcutClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString url = btn->property("url").toString();
    if (!url.isEmpty())
        emit navigateRequested(url);
}