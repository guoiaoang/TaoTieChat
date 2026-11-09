#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class MessageDelegate;
class MessageModel;

class SignalInner : public QObject {
    Q_OBJECT
private:
    using QObject::QObject;
public:
    static SignalInner *instance() {
        static SignalInner s_signal_inner;
        return &s_signal_inner;
    }

    // 智能指针不建议使用引用，无法增加引用计数导致生命周期未延长

// =============================== ChatWindow =============================== //
signals:
    void signal_signout();

// ============================== ContactWindow ============================== //
Q_SIGNALS:
    void sig_contact_add    (std::shared_ptr<ContactData::ItemData> contact_data);
    void sig_contact_remove (std::shared_ptr<ContactData::ItemData> contact_data);
    void sig_contact_message(std::shared_ptr<ContactData::ItemData> contact_data);
    void sig_contact_voice  (std::shared_ptr<ContactData::ItemData> contact_data);
    void sig_contact_video  (std::shared_ptr<ContactData::ItemData> contact_data);

// ================================ Accounts ================================ //
signals:
    void signal_account_changing     (std::shared_ptr<ContactData::ItemData> from_contact_data, std::shared_ptr<ContactData::ItemData> to_contact_data);
    void signal_account_changed      (std::shared_ptr<ContactData::ItemData> to_contact_data);
    void signal_contact_list_changing(const QVector<std::shared_ptr<ContactData::ItemData>>& from_contact_data, const QVector<std::shared_ptr<ContactData::ItemData>>& to_contact_data);
    void signal_contact_list_changed (const QVector<std::shared_ptr<ContactData::ItemData>>& to_contact_data);
    void signal_contact_info_changing(std::shared_ptr<ContactData::ItemData> from_contact_data, std::shared_ptr<ContactData::ItemData> to_contact_data);
    void signal_contact_info_changed (std::shared_ptr<ContactData::ItemData> to_contact_data);  // 需要复制，否则会导致意外的修改而显示不一致

// ============================== NetworkClient ============================== //
signals:
    // 文件下载成功通知
    void signal_file_download_notify(const Network::FileDownloadNotify notify); // 所有事件
    void signal_file_downloading    (const Network::FileDownloadNotify notify);
    void signal_file_download_error (const Network::FileDownloadNotify notify);
    void signal_file_downloaded     (const Network::FileDownloadNotify notify);

// ============================= MessageDelegate ============================= //
signals:
    // MessageWindow 会有多个副本，造成多次执行动作
    void signal_open_link  (const QPointer<MessageDelegate> signal_sender, const QString& href);
    void signal_open_file  (const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);
    void signal_open_folder(const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);
    void signal_open_data  (const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);
    void signal_copy_data  (const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);
    void signal_copy_file  (const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);
    void signal_copy_path  (const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);
    void signal_save_to    (const QPointer<MessageDelegate> signal_sender, std::shared_ptr<MessageData::ItemData> message_data);

// =============================== AuthWindow =============================== //
signals:
    void signal_signin();

// ============================== MessageModel ============================== //
signals:
    void signal_new_message(const QPointer<MessageModel> sender, std::shared_ptr<MessageData::ItemData> message);

// =============================== MediaWindow =============================== //
signals:
    void signal_media_changed(const QPixmap& pixmap);

// ============================== SearchWindow ============================== //
Q_SIGNALS:
    void signal_search_edit_text_changed(const QString& text);

};

// #include "SignalInner.h"
// emit SignalInner::instance()->sig_contact_add(nullptr);
// connect(SignalInner::instance(), &SignalInner::sig_contact_add, this, [](){});
