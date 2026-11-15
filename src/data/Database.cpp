#include "Database.h"

Database* Database::instance() {
    static Database s_database;
    return &s_database;
}

Database::~Database() {
    save_stroage_file_info();
}

bool Database::reload_stroage_file_info() {
    m_inited = true;
    m_file_id_path_map.clear();

    // 暂时将映射序列化存到本地
    QString map_file_path =  m_map_file_dir + "/" +  m_map_file_name;

    mINI::INIFile ini_file(map_file_path.toStdString());
    mINI::INIStructure ini_data;
    // 加载数据时读取失败即可返回错误
    if (!ini_file.read(ini_data)) { return false; }

    if ( ini_data.has(m_section_name.toStdString())) {
        auto section_data = ini_data[m_section_name.toStdString()];
        for (const auto&kv : section_data) {
            auto file_id       = QString::fromStdString(kv.first ).trimmed();
            auto path_relative = QString::fromStdString(kv.second).trimmed();
            auto path_full     = QDir(m_map_file_dir).absoluteFilePath(path_relative);
            m_file_id_path_map[file_id] = path_full;
        }
    }
    return true;
}

bool Database::save_stroage_file_info() {
    // 没有初始化时调用不必保存，防止出现无用文件
    if(!m_inited) { return false; }

    QString map_file_path =  m_map_file_dir + "/" +  m_map_file_name;
    Utils::create_dirs(Utils::get_dir(map_file_path));

    mINI::INIFile ini_file(map_file_path.toStdString());
    mINI::INIStructure ini_data;
    // if (!ini_file.read(ini_data)) { return false; }
    // 没有文件时会读取失败，但需要继续保存
    ini_file.read(ini_data);

    for(auto it = m_file_id_path_map.begin(); it != m_file_id_path_map.end(); ++it) {
        // 计算 it.value() 相对于 m_map_file_dir 的路径
        QString path_relative = QDir(m_map_file_dir).relativeFilePath(it.value());
        ini_data[m_section_name.toStdString()][it.key().toStdString()] = path_relative.toStdString();
    }
    if (!ini_file.write(ini_data)) { return false; };  // 写入失败才返回错误
    return true;
}

void Database::set_config_dir(const QString &config_dir) {
    save_stroage_file_info();
    m_map_file_dir = config_dir;
    reload_stroage_file_info();
}

// 不应存储缓存文件，防止发生意外读写
bool Database::save_local_file_id_path(const QString &file_id, const QString &file_path) {
    if(file_id.isEmpty() || file_path.isEmpty()) { return false; }
    if (file_path.endsWith(".part")) { return false; }  // 不应存储缓存文件，防止发生意外读写
    m_file_id_path_map[file_id] = file_path;
    return save_stroage_file_info();
}

QString Database::get_local_file_id_path(const QString &file_id) {
    QString file_path;
    do {
        // 如果命中则直接返回
        if(file_id.isEmpty()) { break; }
        auto ite_cache = m_file_id_path_map.constFind(file_id);
        if (ite_cache != m_file_id_path_map.constEnd()) {
            file_path = ite_cache.value();
            // break;
            if(!file_path.isEmpty() && QFile::exists(file_path)) { break; }
        }

        // file_path = get_local_file_id_path(file_id);
        // if(!file_path.isEmpty() && QFile::exists(file_path)) { break; }

        // // 记录的路径已失效：必须清空。否则下载会把这个不存在的旧路径
        // // 当成可用位置，以 ReadWrite 打开并创建一个空文件。
        // file_path.clear();

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
            file_path = get_local_file_id_path(str_id);
            if(file_path.isEmpty()) { break; }  // 仅 ID 时还是找不到源文件
            // 转换文件
            QString file_name = QFileInfo(file_path).fileName();
            QString size_path = QString("%1/size/%2-%3/%4").arg(Utils::user_cache_path()).arg(size_width).arg(size_height).arg(file_name);
            // 如果不存在则需要转换
            if(!QFileInfo(size_path).exists()) {
                Utils::create_dirs(Utils::get_dir(size_path));
                // 检测文件格式
                QByteArray format = QImageReader::imageFormat(file_path);
                if (format.toLower() == "svg" || format.toLower() == "svgz") {
                    Utils::Transform::resize_svg_vector(file_path, size_path, QSize(size_width, size_height));
                }
                else {
                    QImage(file_path).scaled(QSize(size_width, size_height), Qt::KeepAspectRatio, Qt::SmoothTransformation).save(size_path);
                }
            }
            save_local_file_id_path(file_id, size_path);
            file_path = size_path;
        }
    } while(false);
    return file_path;
}

// bool Database::has_file_id_metadata(const QString &id) {
//     auto ite_id = m_map_id_metadates.constFind(id);
//     // 没有
//     if (ite_id == m_map_id_metadates.constEnd()) { return false; }
//     // 失效指针
//     if (!ite_id.value()) {
//         m_map_id_metadates.erase(ite_id);
//         return false;
//     }
//     const auto m = ite_id.value();
//     // 是否校验内容是否有效??
//     // if (m->file_name.isEmpty() || m->file_hash.isEmpty() || m->file_size < 0) {
//     //     m_map_id_metadates.erase(ite_id);
//     //     return false;
//     // }
//     return true;
// }
// 
// bool Database::add_file_id_metadata(const QString &id, std::shared_ptr<MessageData::FileData> metadata) {
//     if (has_file_id_metadata(id)) { return false; }
//     m_map_id_metadates.insert(id, metadata);
//     return true;
// }
// 
// bool Database::update_file_id_metadata(const QString &id, std::shared_ptr<MessageData::FileData> metadata) {
//     if (!has_file_id_metadata(id)) { return false; }
//     m_map_id_metadates.erase(m_map_id_metadates.constFind(id));
//     m_map_id_metadates.insert(id, metadata);
//     // m_map_id_metadates[id] = metadata;
//     return true;
// }
