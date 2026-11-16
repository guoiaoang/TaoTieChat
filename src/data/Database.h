#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class Database : public QObject {
    Q_OBJECT
private:
    Database() = default;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

public:
    ~Database();
    static Database* instance();

    static QString get_local_stroage_path(const QString &stroage_dir, const QString &hash, const QString &file_name){
        return QString("%1/%2/%3").arg(stroage_dir).arg(hash).arg(file_name);
    };

    static QString get_local_downloading_path(const QString &stroage_dir, const QString &hash, const QString &file_name){
        return QString("%1/%2/%3.part").arg(stroage_dir).arg(hash).arg(file_name);
    };

public:
    bool reload_stroage_file_info();
    bool save_stroage_file_info();
    void set_config_dir(const QString & config_dir);
    bool save_local_file_id_path(const QString &file_id, const QString &file_path);
    QString get_local_file_id_path(const QString &file_id);

public:
    // 队列中有返回 true
    static bool get_file_hash(const QString &file_path, std::function<void(const QString)> callback) {
        if (file_path.isEmpty() || !QFile(file_path).exists()) {
            if (callback) { callback(""); }
            return false;
        }
        if (!callback) { return false; }

        // 使用线程锁保护全局静态状态
        static QMutex s_mutex;
        // 记录正在计算该文件的回调列表（防止重复计算，且保证所有排队请求都能拿到结果）
        static QMap<QString, QList<std::function<void(const QString)>>> s_pending_callbacks;

        // 添加为列表
        {
            QMutexLocker locker(&s_mutex);
            // 已经在计算中，追加回调
            if (s_pending_callbacks.contains(file_path)) {
                s_pending_callbacks[file_path].append(callback);
                return true;
            }
            s_pending_callbacks[file_path].append(callback);
        }

        // 启动线程计算
        QThread *thread = QThread::create([file_path]() {
            // 1. 在子线程中执行耗时计算
            QString file_hash = Utils::Algorithm::calculate_file_md5(file_path);

            // 2. 加锁取出所有等待的回调
            QList<std::function<void(const QString)>> callbacks;
            {
                QMutexLocker locker(&s_mutex);
                auto ite_callbacks = s_pending_callbacks.find(file_path);
                if (ite_callbacks != s_pending_callbacks.end()) {
                    callbacks = ite_callbacks.value();
                    s_pending_callbacks.remove(ite_callbacks.key());
                }
            }
            for(const auto &cb : callbacks) {
                if (cb) {
                    QMetaObject::invokeMethod(qApp,
                        [cb, file_hash]() mutable {
                            cb(file_hash);
                        }, Qt::QueuedConnection);
                }
            }
        });

        // 更优雅的做法：通过 lambda 捕获 file_path 并安全处理线程结束和回调分发
        // 这里我们把完整逻辑写在连接里
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        // QObject::connect(thread, &QThread::finished, thread, [file_path, thread]() {
        //     // 线程结束时自动回收线程对象内存
        //     thread->deleteLater();
        //     {
        //         QMutexLocker locker(&s_mutex);
        //         auto ite_callbacks = s_pending_callbacks.find(file_path);
        //         if (ite_callbacks != s_pending_callbacks.end()) {
        //             s_pending_callbacks.remove(ite_callbacks.key());
        //         }
        //     }
        // });

        // 启动线程
        thread->start();

        return true;
    }

// public:
//     bool has_file_id_metadata(const QString &id);
//     bool add_file_id_metadata(const QString &id, std::shared_ptr<MessageData::FileData> metadata);
//     bool update_file_id_metadata(const QString &id, std::shared_ptr<MessageData::FileData> metadata);

private:
    bool m_inited = false;  // 未初始化时修改配置路径不需要保存，否则即使为空也要保存
    //
    QMap<QString, QString> m_file_id_path_map;
    QString m_map_file_dir  = Utils::app_data_path();
    QString m_map_file_name = "map_file_id_path.ini";
    QString m_section_name  = "file_ids";
    // //
    // QMap<const QString, std::shared_ptr<MessageData::FileData>> m_map_id_metadates;
};

