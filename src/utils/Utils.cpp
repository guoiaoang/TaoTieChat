#include "Utils.h"
#include "data.h"

void Utils::init_debug_output_format() {
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

bool Utils::load_translator(QApplication&application) {
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
QList<QFileInfo> Utils::dir_contents(const QString &path, bool recursive) {
    QList<QFileInfo> lists;
    using ItFlag = QDirListing::IteratorFlag;
    for (const auto &dirEntry : QDirListing(path, recursive ? ItFlag::Recursive : ItFlag::Default)) {
        lists << dirEntry.fileInfo();
    }
    return lists;
}

// 获取给定大小内指定字体最适合的最大大小
// 字体名称，矩形大小，绘制文本，精度
QFont Utils::get_suitable_font_size(const QString&font_name, const QSizeF&size, const QString&text, const qreal precision) {
    if(font_name.isEmpty()) {
        return QFont();
    }
    QFont font(font_name);
    return get_suitable_font_size(font, size, text);
}

// 字体，矩形大小，绘制文本，精度
QFont Utils::get_suitable_font_size(QFont&font, const QSizeF&size, const QString&text, const qreal precision) {
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
bool Utils::load_font(const QString&font_path) {
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
QIcon Utils::to_icon(const QString&text, const QPen&pen, const QSize size) {
    QFont icon_font = Utils::get_suitable_font_size("Inconsolata Nerd Font Mono", size, text);
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
QPixmap Utils::to_pixamp(const QString&text, const QSize size) {
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
QPixmap Utils::to_round_pixmap(const QPixmap& src, int radius) {
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
QPixmap Utils::apply_mask(const QPixmap& source, const QPixmap& mask) {
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
QString Utils::get_new_uuid_string() {
    return QUuid::createUuid().toString(QUuid::Id128);
}

// struct FontData{
//     QString path;
//     QString name;
//     int     id;
//     FontData(QString font_path, QString font_families_name, int font_id)
//         : path(font_path), name(font_families_name), id(font_id) {}
// };
// static QVector<FontData> m_vector_fonts;
// 实际为类内静态成员，但暂不使用类外初始化等方式
QVector<Utils::FontData> &Utils::f_member_fonts() {
    static QVector<Utils::FontData> m_vector_fonts;
    return m_vector_fonts;
}

// 获取错误信息
QString Utils::to_string(::TaoTieChat::SuccessCodeGadget::SuccessCode code) {
    using sc = ::TaoTieChat::SuccessCodeGadget::SuccessCode;

    static const QMap<sc, QString> map_codes = {
        {sc::SUCCESS       , "Success"},
        {sc::STATUS_SUCCESS, "Success"},
        {sc::STATUS_RUNNING, "Running"},
        {sc::STATUS_ONLINE , "Online"},
    };

    if (map_codes.contains(code)) {
        return map_codes[code];
    }

    return "Unknown Success Code";
}

// 窗口前置显示
void Utils::widget_raise(QWidget*widget) {
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
QString Utils::to_string(::TaoTieChat::ErrorCodeGadget::ErrorCode code) {
    using ec = ::TaoTieChat::ErrorCodeGadget::ErrorCode;

    static const QMap<ec, QString> map_codes = {
        {ec::UNSET                         , "Unset Error Code"       },
        {ec::UNKNOWN                       , "Unknown Error Code"     },
        {ec::ERROR_INIT                    , "Error on Init"          },
        {ec::ERROR_CONN                    , "Error on connect"       },
        {ec::ERROR_RECV                    , "Error on Recv or Parse" },
        {ec::ERROR_SEND                    , "Error on Send"          },
        // 数据库相关错误码
        {ec::DB_QUERY                       , "DB Query Failed"  },
        {ec::DB_QUERY_EMPTY                 , "DB Query Empty"   },
        {ec::DB_EXECUTE                     , "DB Execute Failed"},
        {ec::DB_EXECUTE_EMPTY               , "DB Execute Empty" },
        {ec::DB_CHECK                       , "DB Check Failed"  },
        {ec::DB_CHECK_EMPTY                 , "DB Check Empty"   },
        // error
        {ec::STATUS_TARGET_ID_EMPTY         , "Target ID is empty"            },
        {ec::STATUS_REQUEST_USER_ID_EMPTY   , "Request User ID is empty"      },
        {ec::STATUS_REQUEST_USER_TOKEN_EMPTY, "Request User Token is empty"   },
        {ec::STATUS_TARGET_ID_NOT_FOUND     , "Target ID not found"           },
        {ec::STATUS_USER_ID_INVALID         , "Invalid Request User ID"       },
        {ec::STATUS_USER_TOKEN_INVALID      , "Invalid Request User Token"    },
        {ec::SIGN_UP_USER_ID_EMPTY          , "User ID is empty"              },
        {ec::SIGN_UP_PASSWORD_EMPTY         , "Password is empty"             },
        {ec::SIGN_UP_CONFIRM_PASSWORD_EMPTY , "Confirm Password is empty"     },
        {ec::SIGN_UP_SECURITY_QUESTION_EMPTY, "Security Question is empty"    },
        {ec::SIGN_UP_SECURITY_ANSWER_EMPTY  , "Security Answer is empty"      },
        {ec::SIGN_UP_DIFF_CONFIRM_PASSWORD  , "Confirm Password is not equal" },
        {ec::SIGN_UP_USER_ID_EXISTS         , "User ID already exists"        },
        {ec::SIGN_IN_USER_ID_EMPTY          , "User ID is empty"              },
        {ec::SIGN_IN_PASSWORD_EMPTY         , "Password is empty"             },
        {ec::SIGN_IN_TOKEN_EMPTY            , "Token is empty"                },
        {ec::SIGN_IN_INVALID_LOGIN_METHOD   , "Invalid login method"          },
        {ec::SIGN_IN_USER_ID_NOT_FOUND      , "User ID not found"             },
        {ec::SIGN_IN_PASSWORD_INCORRECT     , "Password is incorrect"         },
        {ec::SIGN_IN_TOKEN_INCORRECT        , "Token is incorrect"            },
        {ec::SIGN_IN_AUTH_FAILED            , "User ID or Password or Token error" },
        {ec::SEARCH_USERS_KEYWORD_EMPTY     , "Search Users is empty"         },
        {ec::SEARCH_USERS_KEYWORD_NOT_FOUND , "Search Users not found"        },
    };

    if (map_codes.contains(code)) {
        return map_codes[code];
    }

    return "Unknown Error Code";
}

void Utils::set(TaoTieChat::Result&result, const ::TaoTieChat::SuccessCodeGadget::SuccessCode&success_code) {
    result.setSuccessCode(success_code);
    result.setMessage(Utils::to_string(success_code));
}

void Utils::set(TaoTieChat::Result&result, const ::TaoTieChat::ErrorCodeGadget::ErrorCode&error_code) {
    result.setErrorCode(error_code);
    result.setMessage(Utils::to_string(error_code));
}

// ====================================================================== //

// 查找指定 id 的好友
Contact Utils::get_contact(Contacts contacts, QString contact_id) {
    if(contacts) {
        for(const auto &contact : *contacts) {
            if(contact && ContactManager(contact).get_account_id() == contact_id) {
                return contact;
            }
        }
    }
    return Contact();
}

// 从好友列表中获取第 index 个好友
Contact Utils::get_contact(Contacts contacts, int index) {
    if(contacts == nullptr || index < 0 || index >= contacts->size()) {
        return Contact();
    }
    return contacts->at(index);
}
