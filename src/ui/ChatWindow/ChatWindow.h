#pragma once

#include "Utils.h"
#include "MessageWindow.h"
#include "ContactModel.h"
#include "ContactDelegate.h"
#include "ContactWindow.h"
#include "SearchWindow.h"

#ifdef DEBUG
#include "Dev.h"
#endif // DEBUG

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog {
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
    void add_chat_room(Contact contact);
    // 显示指定聊天室的对话
    void show_chat_room_messages(Contact contact);

    // // 联系人管理
    // 添加到联系人列表
    void add_contact(Contact contact);
    // model/view 模式下的选择
    void handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous);

signals:
    void signout();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::ChatWindow *ui                = nullptr;
    QSplitter      *m_split_contact   = nullptr;
    QMenu          *m_menu_settings   = nullptr;
    QAction        *m_action_signout  = nullptr;
    QAction        *m_action_settings = nullptr;
    QMenu          *m_menu_add        = nullptr;
    QAction        *m_action_add      = nullptr;
    QAction        *m_action_create   = nullptr;

    // QMap<Contact, MessageWindow*> m_message_windows;  // 聊天室 ID 与对应窗口的绑定

    // model/view/delegate
    ContactModel    *m_contact_model    = nullptr;
    ContactDelegate *m_contact_delegate = nullptr;

    SearchWindow    *m_search_window = nullptr;

    // ====================================================================== //
#ifdef DEBUG
private:
    friend Dev::ChatWindowDev;

    // LocalUserData
    // QSharedPointer<LocalUserData> m_local_user_data = nullptr;
    QSharedPointer<LocalUserData> m_local_user_data = RegistryFactory<LocalUserData>::create("test_local");
#endif // DEBUG
};
