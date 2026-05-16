#include "Utils.h"

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

QString get_dir(const QString& path) {
    // 获取路径中的目录部分
    QFileInfo file_info(path);
    return file_info.absolutePath();
}

bool create_dirs(QString full_path) {
    QDir dir(full_path);
    // 判断当前输入的 fullpath 目录是否为相对路径
    if(dir.isRelative()) {
        dir.makeAbsolute();    // 将相对路径转成绝对路径
        full_path = dir.path(); // dir.path() 将输出绝对路径
    }

    // 如果当前目录已经存在，不在创建
    if (dir.exists()) {
        return true;
    }

    return dir.mkpath(full_path); // 创建多级目录
}

bool remove_dirs(QString full_path) {
    QDir dir(full_path);
    // 判断当前输入的 full_path 目录是否为相对路径
    if(dir.isRelative()) {
        dir.makeAbsolute(); // 将相对路径转成绝对路径
        full_path = dir.path(); // dir.path() 将输出绝对路径
    }

    // 如果当前目录已经不存在，返回
    if (!dir.exists()) {
        return true;
    }

    return dir.removeRecursively(); // 删除目录
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

bool load_translator(QApplication&application) {
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
        qDebug().noquote().nospace() << QString("load font \"%1\" with path \"%2\" successed!").arg(font_family).arg(font_path);
        // 图标资源，不要设置为显示的字体
        // QFont icon_font(font_family);
        // QApplication::setFont(icon_font);
        // QApplication::setApplicationName("󰤱󰤰");  // 测试
        f_member_fonts().append(FontData(font_path, font_family, font_id));
        return true;
    }
    else {
        qDebug().noquote().nospace() << QString("load font with path \"%2\" error!").arg(font_path);
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

// 获取随机字符串作为全局唯一的 ID
// QUuid::WithBraces 形如：{00000000-0000-0000-0000-000000000000}  // 8-4-4-4-12=32 位
// QUuid::Id128 : 不使用间隔符
QString get_new_uuid_string() {
    return QUuid::createUuid().toString(QUuid::Id128);
}

// 获取错误信息
QString to_string(SuccessCode code) {
    static const QMap<SuccessCode, QString> map_codes = {
        {SuccessCode::SUCCESS       , "Success"},
        {SuccessCode::STATUS_SUCCESS, "Success"},
        {SuccessCode::STATUS_RUNNING, "Running"},
        {SuccessCode::STATUS_ONLINE , "Online"},
    };

    if (map_codes.contains(code)) {
        return map_codes[code];
    }

    return "Unknown Success Code";
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

// 获取错误信息
QString to_string(ErrorCode code) {
    static const QMap<ErrorCode, QString> map_codes = {
        {ErrorCode::UNSET                      , "Unset Error Code"                },
        {ErrorCode::UNKNOWN                    , "Unknown Error Code"              },
        {ErrorCode::DEFAULT                    , "Default Error"                   },
        {ErrorCode::ERROR_INIT                 , "Error on Init"                   },
        {ErrorCode::ERROR_CONN                 , "Error on connect"                },
        {ErrorCode::ERROR_RECV                 , "Error on Recv or Parse"          },
        {ErrorCode::ERROR_SEND                 , "Error on Send"                   },
        {ErrorCode::DB_QUERY                   , "DB Query Failed"                 },
        {ErrorCode::DB_QUERY_EMPTY             , "DB Query Empty"                  },
        {ErrorCode::DB_EXECUTE                 , "DB Execute Failed"               },
        {ErrorCode::DB_EXECUTE_EMPTY           , "DB Execute Empty"                },
        {ErrorCode::DB_CHECK                   , "DB Check Failed"                 },
        {ErrorCode::DB_CHECK_EMPTY             , "DB Check Empty"                  },
        {ErrorCode::DB_RECORD_NOT_FOUND        , "DB Record Not Found"             },
        {ErrorCode::USER_ID_EMPTY              , "User ID is empty"                },
        {ErrorCode::USER_ID_NOT_FOUND          , "User ID not found"               },
        {ErrorCode::USER_ID_EXISTS             , "User ID already exists"          },
        {ErrorCode::ACCOUNT_ID_EMPTY           , "Account ID is empty"             },
        {ErrorCode::ACCOUNT_ID_NOT_FOUND       , "Account ID not found"            },
        {ErrorCode::PASSWORD_EMPTY             , "Password is empty"               },
        {ErrorCode::PASSWORD_NOT_FOUND         , "Password not found"              },
        {ErrorCode::TOKEN_EMPTY                , "Token is empty"                  },
        {ErrorCode::TOKEN_NOT_FOUND            , "Token not found"                 },
        {ErrorCode::CONFIRM_PASSWORD_EMPTY     , "Confirm Password is empty"       },
        {ErrorCode::SECURITY_QUESTION_EMPTY    , "Security Question is empty"      },
        {ErrorCode::SECURITY_ANSWER_EMPTY      , "Security Answer is empty"        },
        {ErrorCode::TARGET_USER_ID_EMPTY       , "Target User ID is empty"         },
        {ErrorCode::TARGET_USER_ID_NOT_FOUND   , "Target User ID not found"        },
        {ErrorCode::AUTH_ID_EMPTY              , "Auth ID is empty"                },
        {ErrorCode::AUTH_METHOD_EMPTY          , "Auth method is empty"            },
        {ErrorCode::AUTH_FAILED                , "Authentication failed"           },
        {ErrorCode::DIFF_PASSWORD_WITH_CONFIRM , "Confirm Password is not equal"   },
        {ErrorCode::TARGET_ID_REQUESTED        , "Target already requested"        },
        {ErrorCode::CONTEXT_REQUIRED           , "Context is required"             },
        {ErrorCode::CLIENT_MESSAGE_ID_REQUIRED , "Client message ID is required"   },
        {ErrorCode::DUPLICATE_MESSAGE_ID       , "Duplicate message ID"            },
        {ErrorCode::FRIEND_REQUEST_NOT_FOUND   , "Friend request not found"        },
    };

    if (map_codes.contains(code)) {
        return map_codes[code];
    }

    return "Unknown Error Code";
}

void set(TaoTieChat::Result&result, const ::TaoTieChat::SuccessCodeGadget::SuccessCode&success_code) {
    result.setSuccessCode(success_code);
    result.setMessage(to_string(success_code));
}

void set(TaoTieChat::Result&result, const ::TaoTieChat::ErrorCodeGadget::ErrorCode&error_code) {
    result.setErrorCode(error_code);
    result.setMessage(to_string(error_code));
}

// ====================================================================== //

// 查找指定 id 的好友
std::shared_ptr<ContactInfo> get_contact_info(const std::shared_ptr<ContactInfos>&contact_infos, const QString& contact_account_id) {
    if(contact_infos) {
        for(const auto &contact_info : contact_infos->contactInfos()) {
            if(contact_info.baseInfo().accountId() == contact_account_id) {
                return std::make_shared<ContactInfo>(contact_info);
            }
        }
    }
    return std::make_shared<ContactInfo>();
}

// 从好友列表中获取第 index 个好友
std::shared_ptr<ContactInfo> get_contact_info(const std::shared_ptr<ContactInfos>&contact_infos, int index) {
    if(contact_infos && index >= 0 && index < contact_infos->contactInfos().size()) {
        return std::make_shared<ContactInfo>(contact_infos->contactInfos().at(index));
    }
    return std::make_shared<ContactInfo>();
}

}

namespace Utils::Transform {
QString to_string(const QProtobufMessage * msg) {
    if(msg == nullptr) {
        qDebug() << "Utils::to_string: msg is nullptr";
        return QString();
    }

    // 1. 实例化 JSON 序列化器
    QProtobufJsonSerializer serializer;

    // 2. 将消息序列化为 QByteArray
    QByteArray jsonData = serializer.serialize(msg);

    // 3. 使用 QString::fromUtf8 保证中文不乱码
    return QString::fromUtf8(jsonData);
}

}

// 重载 QDebug 运算符
QDebug operator<<(QDebug debug, const Result &result) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    if(result.hasSuccessCode()) {
        debug.nospace() << "Result("
                        << "success code:" << result.successCode() << ", "
                        << "message:"      << result.message    ()
                        << ")";
    }
    else if(result.hasErrorCode()) {
        debug.nospace() << "Result("
                        << "error code:" << result.errorCode() << ", "
                        << "message:"    << result.message  ()
                        << ")";
    }
    else {
        debug.nospace() << "Result(...)";
    }
    return debug;
}

// register
QDebug operator<<(QDebug debug, const SignUpRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpRequest("
                    << "userId:"           << req.userId()           << ", "
                    << "password:"         << req.password()         << ", "
                    << "confirm:"          << req.confirm()          << ", "
                    << "securityQuestion:" << req.securityQuestion() << ", "
                    << "securityAnswer:"   << req.securityAnswer()
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const SignUpReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignUpReply("
                    << "Request:"   << rep.request  () << ", "
                    << "result:"    << rep.result   () << ", "
                    << "accountId:" << rep.accountId()
                    << ")";
    return debug;
}

// login
QDebug operator<<(QDebug debug, const SignInRequest &req) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    const TaoTieChat::AuthRequest auth = req.hasRequestUserAuth()
                                         ? req.requestUserAuth()
                                         : TaoTieChat::AuthRequest();
    debug.nospace() << "SignInRequest("
                    << "userId:"   << (auth.hasUserId  () ? auth.userId  () : QString()) << ", "
                    << "password:" << (auth.hasPassword() ? auth.password() : QString()) << ", "
                    << "token:"    << (auth.hasToken   () ? auth.token   () : QString())
                    << ")";
    return debug;
}
QDebug operator<<(QDebug debug, const SignInReply &rep) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    debug.nospace() << "SignInReply("
                    << "Request:"   << rep.request  () << ", "
                    << "result:"    << rep.result   () << ", "
                    << "accountId:" << rep.accountId() << ", "
                    << "token:"     << rep.token    ()
                    << ")";
    return debug;
}

// search users
QDebug operator<<(QDebug debug, const ContactInfo& contact_info) {
    QDebugStateSaver saver(debug); // 保持输出格式一致性
    Utils::ContactInfoWrapper ciw(std::make_shared<ContactInfo>(contact_info));
    debug.nospace() << "ContactInfo("
                    << "userId:"   << ciw.user_id  << ", "
                    << "nickname:" << ciw.nickname << ", "
                    // << "avatar:"   << ciw.avatar   << ", "
                    << "sign:"     << ciw.sign     << ", "
                    << "gender:"   << ciw.gender   << ", "
                    << "region:"   << ciw.region   << ", "
                    << "birthday:" << ciw.birthday << ", "
                    << "phones:"   << ciw.phones   << ", "
                    << "emails:"   << ciw.emails   << ", "
                    << "..."
                    << ")";
    return debug;
}
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
