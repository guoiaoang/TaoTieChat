#pragma once

#include "Headers.h"
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
    bool do_search   (const QString&keyword);
    bool search_user (const QString&keyword);
    bool search_group(const QString&keyword);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::unique_ptr<Ui::SearchWindow> m_ui            ;
    std::unique_ptr<QAction         > m_action_search ;
    // model/view/delegate
    std::unique_ptr<ContactModel    > m_users_model   ;
    std::unique_ptr<ContactDelegate > m_users_delegate;
    std::unique_ptr<ContactWindow   > m_user_window   ;
};
