#pragma once

#include "Headers.h"
#include "Utils.h"
#include "MediaWindow.h"
#include "Accounts.h"

namespace Ui {
class ContactWindow;
}

// class Wrapper : private Wrapper::Power { // 1. 改为私有继承
// private: // 将内部类声明移到私有区（可选）
//     class Power {
//     public:
//         void init() {}
//         void reset() {}
//         void func1() {}
//         void func2() {}
//         void func3() {}
//     };
// 
// public:
//     // 2. 此时可以直接使用 using 将基类函数导出为公有
//     using Power::func1;
//     using Power::func2;
//     using Power::func3;
// 
//     Wrapper() {
//         // 3. 继承关系下直接调用基类方法，不需要 p.init()
//         init();
//     }
// };

class Database{
public:
    static Database& instance() {
        static Database s_database;
        return s_database;
    }
private:
    Database() {}
    Database(const Database&) {}
    Database& operator=(const Database&) {}
public:
    class FileIDPathMap {
    public:
        FileIDPathMap() {
            // reload();
        }
        ~FileIDPathMap() {
            save();
        }
        void set_config_dir(const QString& config_dir) {
            save();
            m_map_file_dir = config_dir;
            reload();
        }

        bool reload() {
            m_inited = true;
            m_file_id_path_map.clear();
            // 暂时将映射序列化存到本地
            QString m_map_file_path =  m_map_file_dir + "/" +  m_map_file_name;
            QFile file(m_map_file_path);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                while(!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList parts = line.split("=");
                    if(parts.size() == 2) {
                        QString id = parts[0].trimmed();
                        QString path = parts[1].trimmed();
                        m_file_id_path_map[id] = path;
                    }
                }
                file.close();
            }
            return true;
        }

        bool save() {
            if(!m_inited) { return false; }  // 没有初始化时调用不必保存，防止出现无用文件
            QString m_map_file_path =  m_map_file_dir + "/" +  m_map_file_name;
            QFile file(m_map_file_path);
            if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                for(auto it = m_file_id_path_map.begin(); it != m_file_id_path_map.end(); ++it) {
                    out << it.key() << " = " << it.value() << "\n";
                }
                file.close();
                return true;
            }
            return false;
        }

        QString get_local_file_id_path(const QString&file_id) {
            QString file_path;
            if(file_id.isEmpty()) { return file_path; }
            return m_file_id_path_map.value(file_id, QString());
        }
        bool save_local_file_id_path(const QString&file_id, const QString&file_path) {
            if(file_id.isEmpty() || file_path.isEmpty()) { return false; }
            m_file_id_path_map[file_id] = file_path;
            return save();
        }
    private:
        QMap<QString, QString> m_file_id_path_map;
        QString m_map_file_dir  = Utils::app_data_path();
        QString m_map_file_name = "file_id_path_map.txt";
        // QString m_map_file_path =  m_map_file_dir + "/" +  m_map_file_name;
        bool m_inited = false;  // 未初始化时修改配置路径不需要保存，否则即使为空也要保存
    };
    // using FileIDPathMap::get_local_file_id_path;
    // using FileIDPathMap::save_local_file_id_path;
    // using FileIDPathMap::set_config_dir;

    // 无法直接使用子成员中的函数，只能挨个转发
    QString get_local_file_id_path(const QString&file_id) {
        return m_file_id_path_map.get_local_file_id_path(file_id);
    }
    bool save_local_file_id_path(const QString&file_id, const QString&file_path) {
        return m_file_id_path_map.save_local_file_id_path(file_id, file_path);
    }
    void set_config_dir(const QString& config_dir) {
        m_file_id_path_map.set_config_dir(config_dir);
    }
private:
    FileIDPathMap m_file_id_path_map;
};

class ContactWindow : public QWidget {
    Q_OBJECT

public:
    explicit ContactWindow(QWidget *parent = nullptr);
    ~ContactWindow();

public:
    const std::shared_ptr<ContactInfo>&get_cache_contact_info () const;
    const std::shared_ptr<QPixmap    >&get_cache_avatar_pixmap() const;
    void set_cache_contact_info (const std::shared_ptr<ContactInfo>&contact_info );
    void set_cache_avatar_pixmap(const std::shared_ptr<QPixmap    >&avatar_pixmap);
    void show_contact(const std::shared_ptr<ContactInfo>&contact_info, const std::shared_ptr<QPixmap>&avatar_pixmap = nullptr);

private:
    // 显示模式
    bool set_edit_mode(bool edit = true);
    // 按钮和菜单的槽函数
    void on_button_manager ();
    void on_button_avatar  ();
    void on_button_save    ();
    void on_button_cancel  ();
    void on_button_add     ();
    void on_button_messages();
    void on_button_voice   ();
    void on_button_video   ();
    void on_action_edit    ();
    void on_action_remove  ();

    // get_local_avatar_id_pixmap 根据文件 ID 在本地查找文件并加载为头像图片
    std::shared_ptr<QPixmap> get_local_avatar_id_pixmap(const QString&avatar_id);
    // view_avatar 在媒体窗口查看当前头像
    void view_avatar(const std::shared_ptr<ContactInfo>&contact_info);
    // show_avatar 当前窗口更新头像显示，无网络及校验功能
    void show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap);
    // 传入头像 ID，自动从本地或网络加载头像
    void show_avatar_id(const QString&avatar_id);
    // dialog_select_avatar_file 选择头像文件的对话框
    QString dialog_select_avatar_file();

    // network
    void update_profile_text(const ContactInfo&contact_info, const update_profile_callback&callback_update_profile = nullptr);
    void update_profile_avatar(const QString&avatar_path);

Q_SIGNALS:
    void contact_add    (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_remove (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_message(const std::shared_ptr<ContactInfo>&contact_info);
    void contact_voice  (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_video  (const std::shared_ptr<ContactInfo>&contact_info);

private:
    std::shared_ptr<Ui::ContactWindow> m_ui                 ;
    std::shared_ptr<QMenu            > m_menu_manager       ;
    std::shared_ptr<QAction          > m_action_edit        ;
    std::shared_ptr<QAction          > m_action_remove      ;
    std::shared_ptr<ContactInfo      > m_cache_contact_info ;
    std::shared_ptr<QPixmap          > m_cache_avatar       ;
    bool m_edit_mode = false;
};
