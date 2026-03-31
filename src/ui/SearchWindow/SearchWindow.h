#pragma once

#include <QScrollArea>
#include "Utils.h"
#include "NetworkClient.h"

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QScrollArea {
    Q_OBJECT

public:
    explicit SearchWindow(QWidget *parent = nullptr);
    ~SearchWindow();

Q_SIGNALS:
    void SearchEditTextChanged(const QString &);
private:
    bool do_search(const QString&key);
    bool search_user(const QString&key);
    bool search_group(const QString&key);

private:
    Ui::SearchWindow *ui;
    QAction *m_action_search = nullptr;
};
