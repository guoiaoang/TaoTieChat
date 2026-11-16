#include "Utils.h"
#include "Accounts.h"

namespace Utils {

namespace {
struct FontData {
    QString path;
    QString name;
    int id;
    FontData(QString font_path, QString font_families_name, int font_id)
        : path(font_path), name(font_families_name), id(font_id) {}
};

// 仅在当前编译单元维护字体缓存，避免头文件暴露内部实现细节。
QVector<FontData> &f_member_fonts() {
    static QVector<FontData> m_vector_fonts;
    return m_vector_fonts;
}
}

QString app_path() {
    return QCoreApplication::applicationFilePath();
}

QString app_dir_path() {
    QString path_document = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return path_document + "/TaoTie/Chat/Client";
    return QCoreApplication::applicationDirPath();
}

QString app_data_path() {
    // return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return app_dir_path() + "/data";
}

QString app_config_path() {
    // return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return app_data_path() + "/config.ini";
}

QString users_data_path() {
    // return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.taotiechat";
    return app_data_path() + "/users";
}

QString user_home_path(QString user_id) {
    if(user_id.isEmpty()) {
        auto contact_data = Accounts::instance()->get_current_account();
        if(contact_data && !contact_data->account_id.isEmpty()) {
            user_id = contact_data->account_id;
        }
        if(user_id.isEmpty()) {
            user_id = "tmp";
        }
    }
    return users_data_path() + "/" + user_id;
}

QString user_storage_path(QString user_id) {
    return user_home_path(user_id) + "/storage";
}

QString user_download_path(QString user_id) {
    return user_home_path(user_id) + "/download";
}

QString user_cache_path(QString user_id) {
    return user_home_path(user_id) + "/cache";
}

QString get_dir(const QString& path) {
    // 获取路径中的目录部分
    QFileInfo file_info(path);
    return file_info.absolutePath();
}

bool create_dirs(const QString &full_path) {
    auto last_path = full_path;
    QDir dir(last_path);
    // 判断当前输入的 fullpath 目录是否为相对路径
    if(dir.isRelative()) {
        dir.makeAbsolute();    // 将相对路径转成绝对路径
        last_path = dir.path(); // dir.path() 将输出绝对路径
    }

    // 如果当前目录已经存在，不在创建
    if (dir.exists()) {
        return true;
    }

    return dir.mkpath(last_path); // 创建多级目录
}

bool remove_dirs(const QString &full_path) {
    auto last_path = full_path;
    QDir dir(last_path);
    // 判断当前输入的 full_path 目录是否为相对路径
    if(dir.isRelative()) {
        dir.makeAbsolute(); // 将相对路径转成绝对路径
        last_path = dir.path(); // dir.path() 将输出绝对路径
    }

    // 如果当前目录已经不存在，返回
    if (!dir.exists()) {
        return true;
    }

    return dir.removeRecursively(); // 删除目录
}

bool rename(const QString &src_file_path, const QString &dst_file_path) {
    // 源路径与目标路径都不能为空，源路径不能不存在，但目标路径不能存在
    QFileInfo src_file_info(src_file_path);
    QFileInfo dst_file_info(dst_file_path);
    if (src_file_path.isEmpty() || dst_file_path.isEmpty() || !src_file_info.exists() || dst_file_info.exists()) {
        return false;
    }

    // 目标文件不应存在，不要覆盖原始文件
    if (QFile(dst_file_path).exists()) {
        return false;
    }

    // 确保目标文件的父目录存在，如果不存在则自动级联创建
    auto dst_dir_path = dst_file_info.absolutePath();
    if (!QDir().exists(dst_dir_path)) {
        QDir().mkpath(dst_dir_path);
    }

    // // 无论是文件还是目录，QFile::rename 都能一行搞定，它在底层调用操作系统的重命名 API，效率极高
    // return QFile::rename(src, dst_file_path);

//     // QString src_file_path = "path/to/src_file_path.txt";
//     // QString dst_file_path = "path/to/dst_file_path.txt";
//
//     // 确保 src_file_path 文件确实存在
//     if (!QFile::exists(src_file_path)) {
//         return false;
//     }
//
//     // 如果 dst_file_path 已经存在，先将其删除，否则 rename 必然失败
//     if (QFile::exists(dst_file_path)) {
//         QFile::remove(dst_file_path);
//     }

    // 使用实例化的对象进行重命名，方便获取错误信息
    QFile src_file_writer(src_file_path);
    if (src_file_writer.rename(dst_file_path)) {
        return true;
    }
    // 这里会打印出具体的失败原因，例如 "Permission denied" 或 "File access error"
    qDebug().noquote().nospace() << QString("failed to rename %1 to %2 with error: %3")
        .arg(src_file_path).arg(dst_file_path).arg(src_file_writer.errorString());
    // 直接重命名失败时尝试复制
    if (src_file_writer.copy(dst_file_path)) {
        return true;
    }
    qDebug().noquote().nospace() << QString("failed to copy %1 to %2 with error: %3")
        .arg(src_file_path).arg(dst_file_path).arg(src_file_writer.errorString());
    return false;
}

void init_debug_output_format() {
    constexpr const char* QT_MESSAGE_PATTERN =
        "[%{time yyyy-MM-dd hh:mm:ss.zzz ttt}"
        " %{if-debug}D%{endif}"
        "%{if-info}I%{endif}"
        "%{if-warning}W%{endif}"
        "%{if-critical}C%{endif}"
        "%{if-fatal}F%{endif}]"
        " %{file}:%{line} - %{function}"
        " >> %{message}";
    qSetMessagePattern(QT_MESSAGE_PATTERN);
}

bool load_translator(QApplication &application) {
    // ":/rc/translations/TaoTieChat_zh_CN.qm"
    static QTranslator translator;  // QTranslator 对象必须始终存在，翻译才会生效
    const QStringList ui_languages = QLocale::system().uiLanguages();
    for (const QString &locale : ui_languages) {
        const QString base_name = QString("%1_%2").arg(TOSTRING(PROJECT_NAME)).arg(QLocale(locale).name());
        if (translator.load(":/rc/translations/" + base_name)) {
            application.installTranslator(&translator);
            // break;
            return true;
        }
    }
    return false;
}

// 遍历指定路径获取所有文件和文件夹列表
// path: 目标路径; recursive: 递归子目录
QList<QFileInfo> dir_contents(const QString &path, bool recursive) {
    QList<QFileInfo> lists;
    using ItFlag = QDirListing::IteratorFlag;
    for (const auto &dirEntry : QDirListing(path, recursive ? ItFlag::Recursive : ItFlag::Default)) {
        lists << dirEntry.fileInfo();
    }
    return lists;
}

// 获取给定大小内指定字体最适合的最大大小
// 字体名称，矩形大小，绘制文本，精度
QFont get_suitable_font_size(const QString&font_name, const QSizeF&size, const QString&text, const qreal precision) {
    if(font_name.isEmpty()) {
        return QFont();
    }
    QFont font(font_name);
    return get_suitable_font_size(font, size, text);
}

// 字体，矩形大小，绘制文本，精度
QFont get_suitable_font_size(QFont&font, const QSizeF&size, const QString&text, const qreal precision) {
    QRectF rect;
    rect.setSize(size);
    if(!rect.isValid() || text.isEmpty()) {
        return QFont();
    }
    constexpr qreal font_size_min = 1;
    constexpr qreal font_size_max = 2500;

    // QFont font(font_name);
    // qreal font_size_old = font.pointSizeF();
    QRectF rect_src_normalized(0, 0, rect.width(), rect.height());

    // 测试是否有意义
    {
        font.setPointSizeF(font_size_min);
        QFontMetricsF font_metricsf_min(font);
        QRectF rc_min = font_metricsf_min.boundingRect(text);

        font.setPointSizeF(font_size_max);
        QFontMetricsF font_metricsf_max(font);
        QRectF rc_max = font_metricsf_max.boundingRect(text);

        // 重置左上角后再比较
        QRectF rect_min_normalized(0, 0, rc_min.width(), rc_min.height());
        QRectF rect_max_normalized(0, 0, rc_max.width(), rc_max.height());

        // 矩形太小或无实际可显示内容
        if(!rect_src_normalized.contains(rect_min_normalized) || rect_min_normalized == rect_max_normalized) {
            return QFont();
        }
    }
    // 二分测试
    qreal font_size_left  = font_size_min;
    qreal font_size_right = font_size_max;
    // 浮点数精度设置为 1 即可，不必设置到 0.1
    while(font_size_left + precision < font_size_right) {
        qreal font_size_calc = font_size_left + (font_size_right - font_size_left) / 2;
        font.setPointSizeF(font_size_calc);
        QFontMetricsF font_metricsf_calc(font);
        QRectF rc_calc = font_metricsf_calc.boundingRect(text);
        QRectF rect_calc_normalized(0, 0, rc_calc.width(), rc_calc.height());
        if(rect_src_normalized == rect_calc_normalized) {
            font_size_left = font_size_calc;
            break;
        }
        if(rect_src_normalized.contains(rect_calc_normalized)) {
            font_size_left = font_size_calc;
        }
        else {
            font_size_right = font_size_calc;
        }
    }
    font.setPointSizeF(font_size_left);
    return font;
}

// 注册指定路径的字体文件
bool load_font(const QString&font_path) {
    int font_id = QFontDatabase::addApplicationFont(font_path);
    // int font_id = QFontDatabase::addApplicationEmojiFontFamily(font_path);
    // int font_id = QFontDatabase::addApplicationFallbackFontFamily(font_path);
    if (font_id != -1) {
        QString font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        qDebug().noquote().nospace() << QString(R"(load font "%1" with path "%2" successed!)").arg(font_family).arg(font_path);
        // 图标资源，不要设置为显示的字体
        // QFont icon_font(font_family);
        // QApplication::setFont(icon_font);
        // QApplication::setApplicationName("󰤱󰤰");  // 测试
        f_member_fonts().append(FontData(font_path, font_family, font_id));
        return true;
    }
    else {
        qDebug().noquote().nospace() << QString(R"(load font with path "%2" error!)").arg(font_path);
    }
    return false;
}

// 用于按钮
QIcon to_icon(const QString&text, const QPen&pen, const QSize size) {
    QFont icon_font = get_suitable_font_size("Inconsolata Nerd Font Mono", size, text);
    if(icon_font == QFont()) {
        return QIcon();
    }

    // 1. 在内存中创建一个指定大小和格式的图像
    // Format_ARGB32_Premultiplied 支持透明背景，且绘图效率最高
    QImage image(size, QImage::Format_ARGB32_Premultiplied);

    // 2. 初始化背景（如果不填充，内存中的原始数据可能是花屏的）
    image.fill(Qt::transparent); // 设置为透明背景

    // 3. 创建画家，绑定到 image 内存块
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 4. 绘图操作
    // painter.setPen(Qt::blue);
    // painter.setFont(QFont("Arial", 20, QFont::Bold));
    painter.setPen(pen);
    painter.setFont(icon_font);
    painter.drawText(image.rect(), Qt::AlignCenter, text);

    painter.end(); // 完成绘制，数据已写入 image 内存

    // // 5. 使用结果
    // // 可以直接转为 QPixmap 显示
    // ui->label->setPixmap(QPixmap::fromImage(image));
    // // 或者保存到本地文件
    // image.save("output.png");

    QIcon icon(QPixmap::fromImage(image));
    return icon;
}

// 用于头像
QPixmap to_pixmap(const QString&text, const QSize size) {
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent); // 设置透明背景
    QPainter painter(&pixmap); // 将画笔绑定到 pixmap
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿，画圆更平滑

    // 文字头像
    QRect rect(0, 0, size.width(), size.height());
    // // 画背景
    // painter.setPen(Qt::NoPen);
    // painter.setBrush(QColor(100, 150, 200));
    // painter.drawRoundedRect(rect, 5, 5);
    // 画文本
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(qMin(size.height(), size.width()) * 0.5);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, text.trimmed().left(2).toUpper());

    painter.end(); // 完成绘画（或者等 painter 析构）
    return pixmap;
}

// 添加圆角遮罩
QPixmap to_round_pixmap(const QPixmap& src, int radius) {
    QPixmap dest(src.size());
    dest.fill(Qt::transparent); // 1. 先准备一张全透明底图

    QPainter painter(&dest);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿

    // 2. 画出你想要的“形状”（这就是遮罩区域）
    painter.setBrush(Qt::black);
    painter.drawRoundedRect(dest.rect(), radius, radius);

    // 3. 关键一步：设置合成模式为“取交集”
    // SourceIn 模式：只保留新画内容与已有内容重叠的部分
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawPixmap(0, 0, src); // 画上原图

    return dest;
}

// 添加圆角遮罩
QImage to_round_image(const QImage& src, int radius) {
    QImage dest(src.size(), QImage::Format_ARGB32);
    dest.fill(Qt::transparent); // 1. 先准备一张全透明底图

    QPainter painter(&dest);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿

    // 2. 画出你想要的“形状”（这就是遮罩区域）
    painter.setBrush(Qt::black);
    painter.drawRoundedRect(dest.rect(), radius, radius);

    // 3. 关键一步：设置合成模式为“取交集”
    // SourceIn 模式：只保留新画内容与已有内容重叠的部分
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawImage(0, 0, src); // 画上原图

    return dest;
}

// 添加图形遮罩
QPixmap apply_mask(const QPixmap& source, const QPixmap& mask) {
    // 1. 准备一个结果图，大小与原图一致，初始全透明
    QPixmap result(source.size());
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    // 2. 先画原图
    painter.drawPixmap(0, 0, source);

    // 3. 设置合成模式为 DestinationIn
    // 效果：原图中只有遮罩图“不透明”的地方会被保留
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

    // 4. 画上你的遮罩图片
    // 注意：如果遮罩大小不一致，需要用 scaled 拉伸到原图大小
    painter.drawPixmap(0, 0, mask.scaled(source.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    painter.end();
    return result;
}

// 添加图形遮罩
QImage apply_mask(const QImage& source, const QImage& mask) {
    // 1. 准备一个结果图，大小与原图一致，初始全透明
    QImage result(source.size(), QImage::Format_ARGB32);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    // 2. 先画原图
    painter.drawImage(0, 0, source);

    // 3. 设置合成模式为 DestinationIn
    // 效果：原图中只有遮罩图“不透明”的地方会被保留
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

    // 4. 画上你的遮罩图片
    // 注意：如果遮罩大小不一致，需要用 scaled 拉伸到原图大小
    painter.drawImage(0, 0, mask.scaled(source.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    painter.end();
    return result;
}

// 获取随机字符串作为全局唯一的 ID
// QUuid::WithBraces 形如：{00000000-0000-0000-0000-000000000000}  // 8-4-4-4-12=32 位
// QUuid::Id128 : 不使用间隔符
QString get_new_uuid_string() {
    return QUuid::createUuid().toString(QUuid::Id128);
}

// 内存中展开布局器 In-Memory Layout Expander  // 窗口不需要显示即可应用实际布局
void in_memory_layout_expander(const QWidget*widget) {
    if(!widget) { return; }
    // 关键核心：强制触发 Qt 的初始化和样式抛光
    // 这会确保控件的 Font、Palette 以及基本尺寸策略被正确初始化
    widget->ensurePolished();
    // 关键核心：如果该模板控件内部使用了 QLayout 布局
    if (widget->layout()) {
        // 强行让布局管理器抛弃“懒刷新”，立刻在内存中计算所有子控件的真实 geometry
        widget->layout()->activate();
    }
}

// 窗口前置显示
void widget_raise(QWidget*widget) {
    if(!widget) { return; }

    // 1. 获取当前窗口标志位
    Qt::WindowFlags flags = widget->windowFlags();

    // 2. 临时添加“置顶”标志并显示（这一步会让窗口跳到最前）
    widget->setWindowFlags(flags | Qt::WindowStaysOnTopHint);
    widget->show();

    // 3. 立即移除“置顶”标志，恢复普通窗口属性
    widget->setWindowFlags(flags); // flags & ~Qt::WindowStaysOnTopHint);
    widget->show(); // 必须再次调用 show() 刷新标志位

    // 4. 最后确保获取焦点和激活
    widget->raise();
    widget->activateWindow();
}

void set(Result&result, const SuccessCode&success_code) {
    result.setSuccessCode(success_code);
    result.setMessage(Utils::Transform::to_string(success_code));
}

void set(Result&result, const ErrorCode&error_code) {
    result.setErrorCode(error_code);
    result.setMessage(Utils::Transform::to_string(error_code));
}

// ====================================================================== //

// 时间戳转字符串
// 1. [今天, now] -> hh:mm
// 2. [昨天, 今天) -> 昨天 hh:mm
// 3. [一周内， 昨天) -> 周一 hh:mm
// 4. [本年内，一周内) -> MM/dd
// 5. [本年前，本年内) -> yyyy/MM/dd
QString format_smart_time(const QDateTime &datetime) {
    static const QMap<int, QString> week_names = {
        {1, QObject::tr("Mon")},  // 周一  // Monday
        {2, QObject::tr("Tue")},  // 周二  // Tuesday
        {3, QObject::tr("Wed")},  // 周三  // Wednesday
        {4, QObject::tr("Thu")},  // 周四  // Thursday
        {5, QObject::tr("Fri")},  // 周五  // Friday
        {6, QObject::tr("Sat")},  // 周六  // Saturday
        {7, QObject::tr("Sun")},  // 周日  // Sunday
    };

    if (!datetime.isValid()) {
        return QString();
    }

    // 将传入时间转换为本地时间的日期（确保跨时区比较准确）
    QDateTime local_datetime = datetime.toLocalTime();
    QDate target_date  = local_datetime.date();
    QDate current_date = QDate::currentDate();

    // 计算目标日期与今天的天数差 (例如：今天是 10 号，target 是 9 号，则 days_to_today 为 1)
    qint64 days_to_today = target_date.daysTo(current_date);

    // 1. 今天：直接显示 hh:mm
    if (days_to_today == 0) {
        return local_datetime.toString(QObject::tr("hh:mm"));
    }

    // 2. 昨天：显示 "昨天 hh:mm"
    else if (days_to_today == 1) {
        return QString(QObject::tr("Yesterday %1")).arg(local_datetime.toString(QObject::tr("hh:mm")));
    }

    // 3. 一周内（过去 2 到 6 天）：显示 "周X hh:mm"
    else if (days_to_today >= 2 && days_to_today < 7) {
        QString week_str = week_names.value(target_date.dayOfWeek());
        return QString(QObject::tr("%1 %2")).arg(week_str, local_datetime.toString(QObject::tr("hh:mm")));
    }

    // 4. 今年内：显示 MM/dd
    else if (target_date.year() == current_date.year()) {
        return local_datetime.toString(QObject::tr("MM/dd"));
    }

    // 5. 更早之前：显示 yyyy/MM/dd
    return local_datetime.toString(QObject::tr("yyyy/MM/dd"));
}

// 生成文件打开时要设置的过滤器的字符串
QString create_file_dialog_all_filter_string(QFileDialog::AcceptMode accept_mode) {
    // 文本及音视频均无法使用函数获取支持的后缀格式
    QByteArrayList image_suffixs    = accept_mode == QFileDialog::AcceptOpen ? QImageReader::supportedImageFormats() : QImageWriter::supportedImageFormats();
    QByteArrayList document_suffixs = {"txt", "md" , "pdf", "doc" , "docx", "xsl", "xslx", "csv" , "ppt", "pptx"};
    QByteArrayList audio_suffixs    = {"mp3", "wav", "flac", "wma" , "aac"};
    QByteArrayList video_suffixs    = {"mp4", "avi", "mkv" , "mov" , "wmv"};
    QByteArrayList archiver_suffixs = {"zip", "rar", "7z"  , "gz"  };
    QByteArrayList all_suffixs = document_suffixs + image_suffixs + audio_suffixs + video_suffixs + archiver_suffixs;

    QString document_filter = QObject::tr("Document Files (%1)").arg("*." + document_suffixs.join(";*."));
    QString image_filter    = QObject::tr("Image Files (%1)"   ).arg("*." + image_suffixs   .join(";*."));
    QString audio_filter    = QObject::tr("Audio Files (%1)"   ).arg("*." + audio_suffixs   .join(";*."));
    QString video_filter    = QObject::tr("Video Files (%1)"   ).arg("*." + video_suffixs   .join(";*."));
    QString archiver_filter = QObject::tr("Archiver Files (%1)").arg("*." + archiver_suffixs.join(";*."));

    QString all_formats_filter = QObject::tr("All Formats (%1)").arg("*." + all_suffixs.join(";*."));
    QString all_file_filter    = QObject::tr("All Files (*.*)");

    // 两头不加 `;;`
    QString last_filter = all_file_filter   ;
    last_filter += ";;" + all_formats_filter;
    last_filter += ";;" + document_filter   ;
    last_filter += ";;" + image_filter      ;
    last_filter += ";;" + audio_filter      ;
    last_filter += ";;" + video_filter      ;
    last_filter += ";;" + archiver_filter   ;
    return last_filter;
}

// 获取显示名称的接口
QString get_display_name(std::shared_ptr<ContactInfo> contact_info) {
    QString display_name;
    do {
        if (!contact_info) { break; }
        if (!contact_info->hasBaseInfo()) { break; }
        auto base_info = contact_info->baseInfo();

        // 默认显示昵称
        display_name = base_info.nickname();
        // 有备注时显示备注
        if (contact_info->hasFriendInfo() && !contact_info->friendInfo().notes().isEmpty()) {
            display_name = contact_info->friendInfo().notes();
        }
    } while(false);
    return display_name;
}
// 获取显示名称的接口
QString get_display_name(std::shared_ptr<ContactData::ItemData> contact_data) {
    QString display_name;
    do {
        if (!contact_data || contact_data->account_id.isEmpty()) { break; }
        // 默认显示昵称
        display_name = contact_data->nickname;
        // 有备注时显示备注
        // if (contact_data->is_friend && !contact_data->notes.isEmpty()) {
        if (Accounts::instance()->is_friend(contact_data->account_id) && !contact_data->notes.isEmpty()) {
            display_name = contact_data->notes;
        }
    } while(false);
    return display_name;
}

}

namespace Utils::Transform {
QString to_string(const QProtobufMessage * msg) {
    if(msg == nullptr) {
        qDebug() << "[nullptr]";
        return QString();
    }

    // 1. 实例化 JSON 序列化器
    QProtobufJsonSerializer serializer;

    // 2. 将消息序列化为 QByteArray
    QByteArray jsonData = serializer.serialize(msg);

    // 3. 使用 QString::fromUtf8 保证中文不乱码
    return QString::fromUtf8(jsonData);
}

// 获取错误信息
QString to_string(SuccessCode code) {
    static const QMap<SuccessCode, QString> map_codes = {
        {SuccessCode::SUCCESS       , "Success"},
        {SuccessCode::STATUS_SUCCESS, "Success"},
        {SuccessCode::STATUS_RUNNING, "Running"},
        {SuccessCode::STATUS_ONLINE , "Online" },
    };
    if (map_codes.contains(code)) {
        return map_codes[code];
    }
    return "Unknown Success Code";
}

// 获取错误信息
QString to_string(ErrorCode code) {
    const auto it = s_map_error_code_strings.find(code);
    if (it != s_map_error_code_strings.end()) {
        return QCoreApplication::translate("Utils::Transform", it->second);
    }
    return QCoreApplication::translate("Utils::Transform", "Unknown Error Code");

    // if (s_map_error_code_strings.contains(code)) {
    //     return s_map_error_code_strings.at(code);
    // }
    // return "Unknown Error Code";
}

QString to_string(FileStatusCode file_status_code) {
    if(s_map_file_status_code_strings.contains(file_status_code)) {
        return s_map_file_status_code_strings.at(file_status_code);
    }
    return 0;
}

int to_int(const FileStatusCode& file_status_code) {
    if(s_map_file_status_string_ints.contains(file_status_code)) {
        return s_map_file_status_string_ints.at(file_status_code);
    }
    return 0;
}

TaoTieChat::Timestamp to_timestamp(const QDateTime&datatime) {
    // 1. 创建内部的 google::protobuf::Timestamp 对象
    google::protobuf::Timestamp pb_timestamp;
    // 2. 获取当前毫秒级时间戳
    qint64 msecs = datatime.toMSecsSinceEpoch();
    // 3. 填充秒和纳秒
    pb_timestamp.setSeconds(msecs / 1000);
    pb_timestamp.setNanos((msecs % 1000) * 1000000);
    // 4. 创建包装的 TaoTieChat::Timestamp 并设置内部的 google::protobuf::Timestamp
    TaoTieChat::Timestamp timestamp;
    timestamp.setTimestamp(pb_timestamp);
    return timestamp;
}

// 字节数 → 人类可读大小（KB/MB/GB）
QString human_size(qint64 bytes) {
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    const char *units[] = { "KB", "MB", "GB", "TB" };
    double v = double(bytes);
    int u = -1;
    do { v /= 1024.0; ++u; } while (v >= 1024.0 && u < 3);
    return QStringLiteral("%1 %2").arg(QString::number(v, 'f', 1), QLatin1String(units[u]));
}

}

#include "Database.h"
namespace Utils::Transform {
std::shared_ptr<ContactData::ItemData> to_contact_data(std::shared_ptr<ContactInfo> contact_info) {
    auto contact_data = std::make_shared<ContactData::ItemData>();
    do {
        if (!contact_info) { break; }
        if ( contact_info->hasBaseInfo()) {
            auto base_info       = contact_info->baseInfo();
            contact_data->account_id   = base_info.accountId  ();
            contact_data->user_id      = base_info.userId     ();
            contact_data->avatar_id    = base_info.avatarId   ();
            contact_data->nickname     = base_info.nickname   ();
            contact_data->gender       = base_info.gender     ();
            contact_data->birthday     = base_info.birthday   ();
            contact_data->signature    = base_info.signature  ();
            contact_data->region       = base_info.region     ();
            contact_data->description  = base_info.description();
            contact_data->phones       = base_info.phones     ();
            contact_data->emails       = base_info.emails     ();
            contact_data->social_links = base_info.socialLinks();
            contact_data->moments_id   = base_info.momentsId  ();
            contact_data->groups_id    = base_info.groupsId   ();
        }
        if ( contact_info->hasFriendInfo()) {
            auto friend_info     = contact_info->friendInfo();
            contact_data->status       = friend_info.status     ();
            contact_data->starred      = friend_info.starred    ();
            contact_data->silent       = friend_info.silent     ();
            contact_data->notes        = friend_info.notes      ();
            contact_data->added_source = friend_info.addedSource();
            contact_data->added_time   = friend_info.addedTime  ();
            contact_data->tags         = friend_info.tags       ();
        }
        if ( contact_info->hasSelfInfo()) {
            auto self_info            = contact_info->selfInfo();
            contact_data->modify_self_info  = self_info.modifySelfInfo  ();
            contact_data->password          = self_info.password        ();
            contact_data->token             = self_info.token           ();
            contact_data->security_question = self_info.securityQuestion();
            contact_data->security_answer   = self_info.securityAnswer  ();
        }
        // cache
        contact_data->is_me        = Accounts::instance()->is_me    (contact_data->account_id);
        contact_data->is_friend    = Accounts::instance()->is_friend(contact_data->account_id);
        contact_data->display_name = Utils::get_display_name(contact_info);
        contact_data->avatar_path  = Database::instance()->get_local_file_id_path(contact_data->avatar_id);
        contact_data->avatar_image = std::make_shared<QImage>(contact_data->avatar_path);
    } while(false);
    return contact_data;
}

std::shared_ptr<ContactInfo> to_contact_info(std::shared_ptr<ContactData::ItemData> contact_data) {
    auto contact_info = std::make_shared<ContactInfo>();
    do {
        if (!contact_data) { break; }
        // ContactInfoBase
        {
            ContactInfoBase base_info;
            base_info.setAccountId   (contact_data->account_id  );
            base_info.setUserId      (contact_data->user_id     );
            base_info.setAvatarId    (contact_data->avatar_id   );
            base_info.setNickname    (contact_data->nickname    );
            base_info.setGender      (contact_data->gender      );
            base_info.setBirthday    (contact_data->birthday    );
            base_info.setSignature   (contact_data->signature   );
            base_info.setRegion      (contact_data->region      );
            base_info.setDescription (contact_data->description );
            base_info.setPhones      (contact_data->phones      );
            base_info.setEmails      (contact_data->emails      );
            base_info.setSocialLinks (contact_data->social_links);
            base_info.setMomentsId   (contact_data->moments_id  );
            base_info.setGroupsId    (contact_data->groups_id   );
            contact_info->setBaseInfo(base_info);
        }
        //
        if (contact_data->is_friend) {
            ContactInfoFriend friend_info;
            friend_info.setAccountId   (contact_data->account_id  );
            friend_info.setUserId      (contact_data->user_id     );
            friend_info.setStatus      (contact_data->status      );
            friend_info.setStarred     (contact_data->starred     );
            friend_info.setSilent      (contact_data->silent      );
            friend_info.setNotes       (contact_data->notes       );
            friend_info.setAddedSource (contact_data->added_source);
            friend_info.setAddedTime   (contact_data->added_time  );
            friend_info.setTags        (contact_data->tags        );
            contact_info->setFriendInfo(friend_info);
        }
        if (contact_data->modify_self_info) {
            ContactInfoSelf self_info;
            self_info.setModifySelfInfo  (contact_data->modify_self_info );
            self_info.setPassword        (contact_data->password         );
            self_info.setToken           (contact_data->token            );
            self_info.setSecurityQuestion(contact_data->security_question);
            self_info.setSecurityAnswer  (contact_data->security_answer  );
            contact_info->setSelfInfo(self_info);
        }
    } while(false);
    return contact_info;
}

}

// 算法相关的工具函数
namespace Utils::Algorithm {
// 计算字符串的 MD5 哈希值
QString md5(const QString& input) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(input.toUtf8());
    return hash.result().toHex();
}

// 计算文件的 MD5 哈希值，适用于大文件，分块读取
// 使用示例：
// QString md5Str = calculate_file_md5("C:/example/large_file.zip");
// qDebug() << "MD5:" << md5Str;
QString calculate_file_md5(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    // 防御性编程：确保从文件开头开始计算（可选，但更安全）
    file.seek(0);

    // 初始化 Hash 对象，指定为 MD5 算法
    QCryptographicHash hash(QCryptographicHash::Md5);
    if(!hash.addData(&file)) {
        // 添加数据失败，可能是文件读取错误
        return QString();
    }

    file.close();

    // result() 返回二进制的 Hash 值，toHex() 将其转换为我们常见的 32 位 16 进制字符串
    return hash.result().toHex();
}

} // namespace Utils::Algorithm

// class Utils::WidgetTools::LineEdit;
namespace Utils::WidgetTools{

void LineEdit::add_long_text_tip_button(QLineEdit* edit_line) {
    if(!edit_line) { return; }

    // 1. 初始化输入框（这里以一个名称输入框为例）
    edit_line->setReadOnly(true); // 通常这种带省略号的多为只读或失焦状态

    // 2. 创建自定义的 ... 动作按钮（可以使用内置的样式图标，或者自定义的图片）
    // 如果你有专门的 ... 图标，换成 QIcon(":/icons/more.png")
    // QIcon icon_elide = edit_line->style()->standardIcon(QStyle::SP_TitleBarContextHelpButton);
    static QIcon icon_elide = Utils::to_icon("...");
    QAction* action_elide = new QAction(icon_elide, edit_line->text(), edit_line);

    m_map_edit_action.emplace(edit_line, action_elide);

    // 3. 将其丢入 QLineEdit 的末尾（右侧）
    edit_line->addAction(action_elide, QLineEdit::TrailingPosition);

    // 4. 默认先藏起来
    action_elide->setVisible(false);

    // 5. 绑定点击事件（如果用户点击了这个 ...，可以弹窗显示完整内容，也可以不绑定纯当视觉提示）
    connect(action_elide, &QAction::triggered, this, [this, edit_line]() {
        // 点击 ... 弹出气泡或复制全文
        QToolTip::showText(QCursor::pos(), edit_line->text(), edit_line);
    });

    // 6. 监听文本改变信号
    connect(edit_line, &QLineEdit::textChanged, this, [this](){
        auto*edit_line = dynamic_cast<QLineEdit*>(sender());
        if(!edit_line) { return; }
        update_elide_action_status(edit_line);
    });

    // 7. 🌟 关键：输入框被拉伸或缩小时，可见性也会变，所以必须为输入框安装事件过滤器
    edit_line->installEventFilter(this);
}

// 判断文本是否完整展示（接上一题的精准算法）
bool LineEdit::is_text_fully_visible(QLineEdit* edit_line) {
    if (!edit_line) { return true; }

    // 1. 获取输入框的字体度量信息（Font Metrics）
    QFontMetrics metrics(edit_line->font());

    // 2. 计算当前文本在屏幕上渲染所需的绝对像素宽度
    int width_text = metrics.horizontalAdvance(edit_line->text());

    // 3. 计算 QLineEdit 扣除边框和内边距（Padding）后，真正用来显示文字的“可视区域宽度”
    // lineEdit->rect() 是总大小，contentsRect() 会自动帮你扣除掉边框 (border) 等
    // 注意：如果 action 已经显示出来了，可视区域会变小（因为 action 占用了约 20-30px 的宽度）
    // 为了防止显隐状态“反复横跳”闪烁，我们采用总 contentsRect 的宽度做基准
    int width_visible = edit_line->contentsRect().width();

    // 4. 关键修正：QLineEdit 内部左右两端通常会有 2~4 像素的微小 margin（留白）
    // 为了极度精准，我们需要把这个潜在的 padding 扣除掉
    // 动态获取两端留白，如果拿不到默认可以减去 4 到 6 像素
    int margins_text = 0;
    QMargins margins = edit_line->textMargins();
    margins_text += margins.left() + margins.right();

    // 还要考虑 QStyle 里的基本样式微调，通常安全起见可以再减去 4 像素
    width_visible -= (margins_text + 4);

    // 5. 比较：如果文本实际宽度 <= 可视区域宽度，说明完美容纳，全部显示出来了
    return width_text <= width_visible;
}

// 刷新 Action 的状态
void LineEdit::update_elide_action_status(QLineEdit* edit_line) {
    if (!edit_line) { return; }

    QAction* action_elide = m_map_edit_action.at(edit_line);
    if(!action_elide) { return; }

    // 判断当前输入框的内容是否能装得下
    bool is_fully_visible = is_text_fully_visible(edit_line);

    if (is_fully_visible || !edit_line->isReadOnly()) {
        // 能完全显示，把 ... 藏起来，清空提示
        action_elide->setVisible(false);
        edit_line->setToolTip("");
    } else {
        // 显示不全了！立刻让右侧的 ... 动作冒出来
        action_elide->setVisible(true);
        // 人性化体验：把输入框的悬浮提示设为完整文本
        edit_line->setToolTip(edit_line->text());
    }
}

bool LineEdit::eventFilter(QObject *watched, QEvent *event) {
    auto*edit_line = dynamic_cast<QLineEdit*>(watched);
    // 捕捉输入框的大小变化事件
    if (edit_line && event->type() == QEvent::Resize && m_map_edit_action.contains(edit_line)) {
        // 输入框大小一变，立刻重新核算 ... 的显隐
        update_elide_action_status(edit_line);
    }
    return QObject::eventFilter(watched, event);
}

}

// 系统相关的操作
namespace Utils::OS {

// windows 管道可以被多个进程同时访问
bool try_grab_account_pipe(const QString& user_id, bool grab) {
    // 改为指针形式管理，避免 static 的某些局限，方便控制生命周期
    static QLocalServer* server = nullptr;

    if(!grab) {
        if(server) {
            server->close();
            delete server;
            server = nullptr;
        }
        return true;
    }

    if (server && server->isListening()) {
        return true; // 已经由当前进程占用了
    }

    QString pipeName = QString("pipe_%1_%2").arg(qApp->applicationName()).arg(user_id);

    // 在监听前，先用 Socket 去试探连一下
    QLocalSocket socket;
    socket.connectToServer(pipeName);

    // 如果在 50 毫秒内连接成功了，说明已经有一个前置客户端正在 listen 这个管道并活着呢！
    if (socket.waitForConnected(50)) {
        socket.disconnectFromServer();
        return false; // 被别人占用了，直接返回 false！
    }

    // 如果连不上，说明当前没有活着的进程在用这个名字，赶紧占领它
    if (!server) {
        server = new QLocalServer();
    }

    return server->listen(pipeName);
}

}

namespace Utils::Transform {
bool resize_svg_to_bitmap(const QString& inputSvgPath, const QString& outputImagePath, const QSize& newSize);
bool resize_svg_vector(const QString& inputSvgPath, const QString& outputSvgPath, const QSize& newSize);

// 使用示例：将 64x64 的 SVG 放大渲染保存为 1024x1024 的 PNG
// resize_svg_to_bitmap("icon.svg", "icon_1024.png", QSize(1024, 1024));
bool resize_svg_to_bitmap(const QString& inputSvgPath, const QString& outputImagePath, const QSize& newSize) {
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
// resize_svg_vector("input.svg", "output_resized.svg", QSize(800, 600));
bool resize_svg_vector(const QString& inputSvgPath, const QString& outputSvgPath, const QSize& newSize) {
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

}

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const AuthRequest &request) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "AuthRequest{"
        << "oneof auth_id {"
        << "accountId:" << (request.hasAccountId() ? request.accountId() : QString()) << ", "
        << "userId:"    << (request.hasUserId()    ? request.userId()    : QString()) << ", "
        << "},"
        << "oneof auth_id {"
        << "password:"  << (request.hasPassword()  ? request.password()  : QString()) << ", "
        << "token:"     << (request.hasToken()     ? request.token()     : QString())
        << "} "
        << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const Result &result) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    if(result.hasSuccessCode()) {
        debug.nospace() << "Result{"
                        << "success code:" << result.successCode() << ", "
                        << "message:"      << result.message    ()
                        << "}";
    }
    else if(result.hasErrorCode()) {
        debug.nospace() << "Result{"
                        << "error code:" << result.errorCode() << ", "
                        << "message:"    << result.message  ()
                        << "}";
    }
    else {
        debug.nospace() << "Result{...}";
    }
    return debug;
}

// register
QDebug operator<<(QDebug debug, const SignUpRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpRequest{"
                    << "userId:"           << req.userId()           << ", "
                    << "password:"         << req.password()         << ", "
                    << "confirm:"          << req.confirm()          << ", "
                    << "securityQuestion:" << req.securityQuestion() << ", "
                    << "securityAnswer:"   << req.securityAnswer()
                    << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const SignUpReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpReply{"
                    << "Request:"   << rep.request  () << ", "
                    << "result:"    << rep.result   () << ", "
                    << "accountId:" << rep.accountId()
                    << "}";
    return debug;
}

// login
QDebug operator<<(QDebug debug, const SignInRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    const AuthRequest auth = req.hasRequestUserAuth() ? req.requestUserAuth() : AuthRequest();
    debug.nospace() << "SignInRequest{"
                    << "userId:"   << (auth.hasUserId  () ? auth.userId  () : QString()) << ", "
                    << "password:" << (auth.hasPassword() ? auth.password() : QString()) << ", "
                    << "token:"    << (auth.hasToken   () ? auth.token   () : QString())
                    << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const SignInReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignInReply{"
                    << "Request:"   << rep.request  () << ", "
                    << "result:"    << rep.result   () << ", "
                    << "accountId:" << rep.accountId() << ", "
                    << "token:"     << rep.token    ()
                    << "}";
    return debug;
}

// contact info
QDebug operator<<(QDebug debug, const ContactInfo& contact_info) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "ContactInfo{"
    << "base_info:"   << contact_info.baseInfo  () << ", "
    << "oneof extra_info{"
    << "self_info:"   << (contact_info.hasSelfInfo  () ? contact_info.selfInfo  () : ContactInfoSelf  ()) << "{}, "
    << "friend_info:" << (contact_info.hasFriendInfo() ? contact_info.friendInfo() : ContactInfoFriend()) << "{}"
    << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const ContactInfoBase& contact_info_base) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "ContactInfoBase{"
                    << "account_id:"   << contact_info_base.accountId  () << ", "
                    << "user_id:"      << contact_info_base.userId     () << ", "
                    << "avatar_id:"    << contact_info_base.avatarId   () << ", "
                    << "nickname:"     << contact_info_base.nickname   () << ", "
                    << "gender:"       << contact_info_base.gender     () << ", "
                    << "birthday:"     << contact_info_base.birthday   () << ", "
                    << "signature:"    << contact_info_base.signature  () << ", "
                    << "region:"       << contact_info_base.region     () << ", "
                    << "description:"  << contact_info_base.description() << ", "
                    << "phones:"       << contact_info_base.phones     () << ", "
                    << "emails:"       << contact_info_base.emails     () << ", "
                    << "social_links:" << contact_info_base.socialLinks() << ", "
                    << "moments_id:"   << contact_info_base.momentsId  () << ", "
                    << "groups_id:"    << contact_info_base.groupsId   ()
                    << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const ContactInfoSelf& contact_info_self) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "ContactInfoSelf{"
                    << "modify_self_info :" << contact_info_self.modifySelfInfo  () << ","
                    << "user_id:"           << contact_info_self.userId          () << ","
                    << "password:"          << contact_info_self.password        () << ","
                    << "token:"             << contact_info_self.token           () << ","
                    << "security_question:" << contact_info_self.securityQuestion() << ","
                    << "security_answer:"   << contact_info_self.securityAnswer  ()
                    << "}";
    return debug;
}
QDebug operator<<(QDebug debug, const ContactInfoFriend& contact_info_friend) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "ContactInfoFriend{"
                    << "account_id:"   << contact_info_friend.accountId  () << ", "
                    << "user_id:"      << contact_info_friend.userId     () << ", "
                    << "status:"       << contact_info_friend.status     () << ", "
                    << "starred:"      << contact_info_friend.starred    () << ", "
                    << "silent:"       << contact_info_friend.silent     () << ", "
                    << "notes:"        << contact_info_friend.notes      () << ", "
                    << "added_source:" << contact_info_friend.addedSource() << ", "
                    << "added_time:"   << contact_info_friend.addedTime  () << ", "
                    << "tags:"         << contact_info_friend.tags       ()
                    << "}";
    return debug;
}

// search users
QDebug operator<<(QDebug debug, const SearchUserRequest& req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SearchUserRequest("
                    << "userId:"     << req.userId()
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const SearchUserReply& rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SearchUserReply("
                    << "Request:"  << rep.request () << ", "
                    << "result:"   << rep.result  () << ", "
                    << "contacts.size:" << rep.contactInfo()
                    << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const SyncTag&tag) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SyncTag("
                    << "SyncType:" << tag.type   () << ", "
                    << "id_start:" << tag.idStart() << ", "
                    << "id_end:"   << tag.idEnd  () << ", "
                    << "count:"    << tag.count  ()
                    << ")";
    return debug;
}

// rpc Sync(stream SyncRequest) returns (stream SyncReply) {}
QDebug operator<<(QDebug debug, const SyncRequest&req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SyncRequest("
                    << "request:"  << req.requestUserAuth() << ", "
                    << "sync_tag:" << req.syncTag        ()
                    << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const SyncReplyHeader&header) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SyncReplyHeader("
                    << "request:" << header.request() << ", "
                    << "result:"  << header.result ()
                    << ")";
    return debug;
}

using TaoTieChat::SyncContact;
QDebug operator<<(QDebug debug, const SyncContact&sync_contact) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SyncContact("
                    << "sub_type:" << sync_contact.subType() << ", ";
    ;    if(sync_contact.hasUpdate ()) { debug.nospace() << "update:"  << "..."; }
    else if(sync_contact.hasRequest()) { debug.nospace() << "request:" << "..."; }
    else if(sync_contact.hasReply  ()) { debug.nospace() << "reply:"   << "..."; }
    else if(sync_contact.hasShip   ()) { debug.nospace() << "ship:"    << "..."; }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const SyncReplyBody&body) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SyncReplyBody("
                    << "tag:" << body.tag() << ", ";
    if(body.hasContacts()) { debug.nospace() << "contacts:" << body.contacts().contact(); }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const SyncReply&rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SyncReply(";
    ;    if(rep.hasHeader()) { debug.nospace() << "header:" << rep.header(); }
    else if(rep.hasBody  ()) { debug.nospace() << "body:"   << rep.body  (); }
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Timestamp&ts) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "Timestamp("
        << "timestamp:" << (ts.hasTimestamp() ? ts.timestamp().toDateTime().toLocalTime().toString(Qt::ISODate) : "")
        << ")";
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const MessageContext&req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "MessageContext(";
    if (req.hasRequestUserAuth()) { debug.nospace() << "request_user_auth:" << req.requestUserAuth() << ","; }
    ;                               debug.nospace() << "sender_account_id:" << req.senderAccountId() << ",";
    ;                               debug.nospace() << "client_message_id:" << req.clientMessageId() << ",";
    if (req.hasTimestamp      ()) { debug.nospace() << "timestamp:"         << req.timestamp      () << ","; }
    ;                               debug.nospace() << "message_type:"      << req.messageType    () << ",";
    ;                               debug.nospace() << "oneof chat_room {";
    ;    if (req.hasAccountId ()) { debug.nospace() << "account_id:"        << req.accountId      () << ","; }
    else if (req.hasGroupId   ()) { debug.nospace() << "group_id:"          << req.groupId        () << ","; }
    ;                               debug.nospace() << "}";
    ;                               debug.nospace() << "oneof content:{";
    if (req.hasShortText      ()) { debug.nospace() << "short_text:"        << req.shortText() << ","; }
    ;                               debug.nospace() << "}";
    debug.nospace() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const MessageReply&rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "MessageReply(";
    if(rep.hasRequest  ()) { debug.nospace() << "request:"   << rep.request  (); }
    if(rep.hasResult   ()) { debug.nospace() << "result:"    << rep.result   (); }
    if(rep.hasTimestamp()) { debug.nospace() << "timestamp:" << rep.timestamp(); }
    debug.nospace() << "server_message_id:"  << rep.serverMessageId();
    debug.nospace() << "message:"  << rep.message();
    debug.nospace() << ")";
    return debug;
}

