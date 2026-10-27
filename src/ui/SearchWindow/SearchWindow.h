#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class ItemModel;
class SearchModel;
class ItemDelegate;
class ContactWindow;

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QScrollArea {
    Q_OBJECT

public:
    explicit SearchWindow(QWidget *parent = nullptr);
    ~SearchWindow();

public:
    void set_keyword(const QString&keyword);
    QString get_keyword() const;
    std::shared_ptr<ContactWindow> get_user_window() const;

// Q_SIGNALS:
//     void signal_search_edit_text_changed(const QString &);

public slots:
    // 更新搜索结果中显示的数据
    void slot_update_search_list_info(std::shared_ptr<ContactData::ItemData> to_contact_data);
    void slot_file_downloaded        (const Network::FileDownloadNotify notify);

private:
    bool do_search   (const QString&keyword);
    bool search_user (const QString&keyword);
    bool search_group(const QString&keyword);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::shared_ptr<Ui::SearchWindow> m_ui           ;
    std::shared_ptr<QAction         > m_action_search;
    // model/view/delegate
    std::shared_ptr<SearchModel  > m_users_model   ;
    std::shared_ptr<ItemDelegate > m_users_delegate;
    std::shared_ptr<ContactWindow> m_user_window   ;
};
