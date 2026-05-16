#pragma once

#include "Utils.h"
#include "MessageWindow.h"
#include "ContactModel.h"
#include "ContactDelegate.h"
#include "ContactWindow.h"
#include "SearchWindow.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private:
    // 显示设置菜单
    void on_show_settings_menu();
    // 显示 创建/添加 好友/群组 菜单
    void on_show_add_menu();

    // // 聊天室管理
    // 单击聊天室中的一个对话  // 显示聊天窗口
    void on_chat_room_item_selected(const QModelIndex &index);
    // 双击聊天室中的一个对话  // 微信会弹窗聊天窗口
    void on_chat_room_item_activated(const QModelIndex &index);

public Q_SLOTS:
    // // 聊天室管理
    // 添加到聊天室列表
    void add_chat_room(const std::shared_ptr<ContactInfo>&contact_info);
    // 显示指定聊天室的对话
    void show_chat_room_messages(const std::shared_ptr<ContactInfo>&contact_info);

    // // 联系人管理
    // 添加到联系人列表
    void add_contact(const std::shared_ptr<ContactInfo>&contact_info);
    // model/view 模式下的选择
    void handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous);

signals:
    void signout();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::shared_ptr<Ui::ChatWindow > m_ui              ;
    std::shared_ptr<QMenu          > m_menu_settings   ;
    std::shared_ptr<QAction        > m_action_signout  ;
    std::shared_ptr<QAction        > m_action_settings ;
    std::shared_ptr<QMenu          > m_menu_add        ;
    std::shared_ptr<QAction        > m_action_add      ;
    std::shared_ptr<QAction        > m_action_create   ;
    // model/view/delegate
    std::shared_ptr<ContactModel   > m_contact_model   ;
    std::shared_ptr<ContactDelegate> m_contact_delegate;
    std::shared_ptr<SearchWindow   > m_search_window   ;
    std::shared_ptr<ContactWindow  > m_contact_window  ;  // 当前显示的联系人详情窗口
};
