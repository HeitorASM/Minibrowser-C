#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include "SearchEngine.h"

class HomepageWidget : public QWidget {
    Q_OBJECT
public:
    explicit HomepageWidget(QWidget *parent = nullptr);
    ~HomepageWidget();

    void setSearchEngine(SearchEngine engine);

signals:
    void navigateRequested(const QString &url);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void updateClock();
    void onShortcutClicked();

private:
    void setupUi();
    void updateEngineBadge();

    QTimer *m_timer;
    QLabel *m_clockLabel;
    QLabel *m_dateLabel;
    QLabel *m_engineBadge;
    SearchEngine m_currentEngine = SearchEngine::DuckDuckGo;
};