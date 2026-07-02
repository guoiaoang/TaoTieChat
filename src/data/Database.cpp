#include "Database.h"

Database& Database::instance() {
    static Database s_database;
    return s_database;
}

// 无法直接使用子成员中的函数，只能挨个转发
QString Database::get_local_file_id_path(const QString&file_id) {
    // return m_file_id_path_map.get_local_file_id_path(file_id);

    QString file_path;
    do {
        // 如果命中则直接返回
        if(file_id  .isEmpty()) { break; }
        file_path = m_file_id_path_map.get_local_file_id_path(file_id);
        if(!file_path.isEmpty() && QFile::exists(file_path)) { break; }

        // 没有直接命中则尝试转换
        // uuid-xxx-uuid;size:128*128
        QStringList strs_group = file_id.split(";");
        if(strs_group.size() <= 1) { break; }
        QString str_id;
        int size_width  = -1;
        int size_height = -1;
        for(auto&str: strs_group) {
            QStringList strs_kv = str.split(":");
            if(strs_kv.size() <= 1) {
                str_id = strs_kv.at(0);
            }
            else {
                auto key    = strs_kv.at(0);
                auto values = strs_kv.at(1);
                if(key == "size") {
                    auto str_wh = values.split("*");
                    if(str_wh.size() == 2) {
                        size_width  = str_wh.at(0).toInt();
                        size_height = str_wh.at(1).toInt();
                    }
                }
            }
        }
        if(!str_id.isEmpty() && size_width >= 0 && size_height > 0) {
            // 将 id 再次传入进行查找
            file_path = m_file_id_path_map.get_local_file_id_path(str_id);
            if(file_path.isEmpty()) { break; }  // 仅 ID 时还是找不到源文件
            // 转换文件
            QString file_name = QFileInfo(file_path).fileName();
            QString size_path = QString("%1/size/%2-%3/%4").arg(Utils::user_cache_path()).arg(size_width).arg(size_height).arg(file_name);
            // 如果不存在则需要转换
            if(!QFileInfo(size_path).exists()) {
                Utils::create_dirs(Utils::get_dir(size_path));
                QPixmap(file_path).scaled(QSize(size_width, size_height), Qt::KeepAspectRatio, Qt::SmoothTransformation).save(size_path);
            }
            m_file_id_path_map.save_local_file_id_path(file_id, size_path);
            file_path = size_path;
        }
    } while(false);
    return file_path;
}

bool Database::save_local_file_id_path(const QString&file_id, const QString&file_path) {
    return m_file_id_path_map.save_local_file_id_path(file_id, file_path);
}

void Database::set_config_dir(const QString& config_dir) {
    m_file_id_path_map.set_config_dir(config_dir);
}

Database::FileIDPathMap::FileIDPathMap() {
    // reload();
}

Database::FileIDPathMap::~FileIDPathMap() {
    save();
}

void Database::FileIDPathMap::set_config_dir(const QString& config_dir) {
    save();
    m_map_file_dir = config_dir;
    reload();
}

bool Database::FileIDPathMap::reload() {
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

bool Database::FileIDPathMap::save() {
    if(!m_inited) { return false; }  // 没有初始化时调用不必保存，防止出现无用文件
    QString m_map_file_path =  m_map_file_dir + "/" +  m_map_file_name;
    Utils::create_dirs(Utils::get_dir(m_map_file_path));
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

QString Database::FileIDPathMap::get_local_file_id_path(const QString&file_id) {
    QString file_path;
    if(file_id.isEmpty()) { return file_path; }
    return m_file_id_path_map.value(file_id, QString());
}

bool Database::FileIDPathMap::save_local_file_id_path(const QString&file_id, const QString&file_path) {
    if(file_id.isEmpty() || file_path.isEmpty()) { return false; }
    m_file_id_path_map[file_id] = file_path;
    return save();
}
