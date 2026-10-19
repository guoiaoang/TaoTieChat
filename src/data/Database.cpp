#include "Database.h"

Database* Database::instance() {
    static Database s_database;
    return &s_database;
}

// 使用示例：将 64x64 的 SVG 放大渲染保存为 1024x1024 的 PNG
// resizeSvgToBitmap("icon.svg", "icon_1024.png", QSize(1024, 1024));
bool resizeSvgToBitmap(const QString &inputSvgPath, const QString &outputImagePath, const QSize &newSize) {
    // 1. 加载 SVG 渲染器
    QSvgRenderer renderer(inputSvgPath);
    if (!renderer.isValid()) {
        return false; // SVG 文件解析失败
    }

    // 2. 创建指定大小的透明 QImage（支持 Alpha 通道）
    QImage image(newSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent); // 背景透明

    // 3. 使用 QPainter 将 SVG 绘制到 QImage 上
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);         // 抗锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform); // 顺滑缩放

    // 绘制到指定的区域大小
    renderer.render(&painter, QRectF(0, 0, newSize.width(), newSize.height()));
    painter.end();

    // 4. 保存为目标文件（根据扩展名自动识别 PNG/JPG 等）
    return image.save(outputImagePath);
}

// 使用示例：调整 XML 视口尺寸并保存为新的 svg
// resizeSvgVector("input.svg", "output_resized.svg", QSize(800, 600));
bool resizeSvgVector(const QString &inputSvgPath, const QString &outputSvgPath, const QSize &newSize) {
    QFile file(inputSvgPath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    // 1. 解析 SVG 为 DOM 树
    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    // 2. 获取根节点 <svg>
    QDomElement svgElement = doc.documentElement();
    if (svgElement.tagName() != "svg") return false;

    // 3. 设置/更新宽与高属性
    svgElement.setAttribute("width", QString("%1px").arg(newSize.width()));
    svgElement.setAttribute("height", QString("%1px").arg(newSize.height()));

    // 提示：若需要适配比例，亦可同时调整 viewBox 属性，例如：
    // svgElement.setAttribute("viewBox", QString("0 0 %1 %2").arg(newSize.width()).arg(newSize.height()));

    // 4. 将修改后的 XML 内容写入新 SVG 文件
    QFile outFile(outputSvgPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&outFile);
    doc.save(stream, 4); // 4 表示 XML 缩进空格数
    outFile.close();

    return true;
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
                // 检测文件格式
                QByteArray format = QImageReader::imageFormat(file_path);
                if (format.toLower() == "svg" || format.toLower() == "svgz") {
                    resizeSvgVector(file_path, size_path, QSize(size_width, size_height));
                }
                else {
                    QPixmap(file_path).scaled(QSize(size_width, size_height), Qt::KeepAspectRatio, Qt::SmoothTransformation).save(size_path);
                }
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

bool Database::FileIDPathMap::save() {
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
