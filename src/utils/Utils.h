#pragma once

#include "Headers.h"
#include "taotie_chat.qpb.h"
#include "taotie_chat_client.grpc.qpb.h"

// using ::TaoTieChat::ResultCodeGadget::ResultCode;

class Utils {
public:
    Utils() = default;

public:
    static void init_debug_output_format();

    static bool load_translator(QApplication&application);

    // 遍历指定路径获取所有文件和文件夹列表
    // path: 目标路径; recursive: 递归子目录
    static QList<QFileInfo> dir_contents(const QString &path, bool recursive = false);

    // 获取给定大小内指定字体最适合的最大大小
    // 字体名称，矩形大小，绘制文本，精度
    static QFont get_suitable_font_size(const QString&font_name, const QSizeF&size, const QString&text, const qreal precision = 1);

    // 字体，矩形大小，绘制文本，精度
    static QFont get_suitable_font_size(QFont&font, const QSizeF&size, const QString&text, const qreal precision = 1);

    // 注册指定路径的字体文件
    static bool load_font(const QString&font_path);

    // 用于按钮
    static QIcon to_icon(const QString&text, const QPen&pen = QPen(Qt::white), const QSize size = QSize(128, 128));
    // 用于头像
    static QPixmap to_pixamp(const QString&text, const QSize size = QSize(128, 128));

    // ====================================================================== //
    // 窗口前置显示
    static void widget_raise(QWidget*widget);
    // 添加圆角遮罩
    static QPixmap to_round_pixmap(const QPixmap& src, int radius = 10);
    // 添加图形遮罩
    static QPixmap apply_mask(const QPixmap& source, const QPixmap& mask);

    // ====================================================================== //
    // 获取随机字符串作为全局唯一的 ID
    // QUuid::WithBraces 形如：{00000000-0000-0000-0000-000000000000}  // 8-4-4-4-12=32 位
    // QUuid::Id128 : 不使用间隔符
    static QString get_new_uuid_string();

    // ====================================================================== //
    // 获取错误信息
    static QString to_string(::TaoTieChat::SuccessCodeGadget::SuccessCode code);
    static QString to_string(::TaoTieChat::ErrorCodeGadget::ErrorCode     code);
    static void set(TaoTieChat::Result&result, const ::TaoTieChat::SuccessCodeGadget::SuccessCode&success_code) {
        result.setSuccessCode(success_code);
        result.setMessage(Utils::to_string(success_code));
    }
    static void set(TaoTieChat::Result&result, const ::TaoTieChat::ErrorCodeGadget::ErrorCode&error_code) {
        result.setErrorCode(error_code);
        result.setMessage(Utils::to_string(error_code));
    }

private:
    struct FontData{
        QString path;
        QString name;
        int     id;
        FontData(QString font_path, QString font_families_name, int font_id)
            : path(font_path), name(font_families_name), id(font_id) {}
    };
    // static QVector<FontData> m_vector_fonts;
    // 实际为类内静态成员，但暂不使用类外初始化等方式
    static QVector<FontData> &f_member_fonts();
};
