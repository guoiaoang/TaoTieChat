#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"
#include "NetworkClient.h"
#include "ContactModel.h"
#include "ContactDelegate.h"
#include "ContactWindow.h"

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
    bool do_search(const QString&keyword);
    bool search_users(const QString&keyword);
    bool search_groups(const QString&keyword);

private:
    Ui::SearchWindow *ui;
    QAction          *m_action_search  = nullptr;
    // model/view/delegate
    ContactModel     *m_users_model    = nullptr;
    ContactDelegate  *m_users_delegate = nullptr;
    ContactWindow    *m_user_window    = nullptr;
};
