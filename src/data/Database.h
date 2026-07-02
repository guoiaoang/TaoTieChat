#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class Database{
    // Q_OBJECT
private:
    Database() = default;
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
public:
    ~Database() = default;
    static Database& instance();
public:
    class FileIDPathMap {
    public:
        FileIDPathMap();
        ~FileIDPathMap();
        void set_config_dir(const QString& config_dir);

        bool reload();

        bool save();

        QString get_local_file_id_path(const QString&file_id);
        bool save_local_file_id_path(const QString&file_id, const QString&file_path);
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
    QString get_local_file_id_path(const QString&file_id);
    bool save_local_file_id_path(const QString&file_id, const QString&file_path);
    void set_config_dir(const QString& config_dir);
private:
    FileIDPathMap m_file_id_path_map;
};

