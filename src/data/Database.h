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

