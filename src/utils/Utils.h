#pragma once

#include "Headers.h"
#include "data.h"
#include "taotie_chat.qpb.h"
#include "taotie_chat_client.grpc.qpb.h"

// using ::TaoTieChat::ResultCodeGadget::ResultCode;

namespace Utils {

QString app_path();
QString app_dir_path();
QString app_data_path();
QString app_config_path();
QString users_data_path();
QString get_dir(const QString& path);
bool create_dirs(QString full_path);
bool remove_dirs(QString full_path);

void init_debug_output_format();

bool load_translator(QApplication&application);

// 遍历指定路径获取所有文件和文件夹列表
// path: 目标路径; recursive: 递归子目录
QList<QFileInfo> dir_contents(const QString &path, bool recursive = false);

// 获取给定大小内指定字体最适合的最大大小
// 字体名称，矩形大小，绘制文本，精度
QFont get_suitable_font_size(const QString&font_name, const QSizeF&size, const QString&text, const qreal precision = 1);

// 字体，矩形大小，绘制文本，精度
QFont get_suitable_font_size(QFont&font, const QSizeF&size, const QString&text, const qreal precision = 1);

// 注册指定路径的字体文件
bool load_font(const QString&font_path);

// 用于按钮
QIcon to_icon(const QString&text, const QPen&pen = QPen(Qt::white), const QSize size = QSize(128, 128));
// 用于头像
QPixmap to_pixamp(const QString&text, const QSize size = QSize(128, 128));

// ====================================================================== //
// 窗口前置显示
void widget_raise(QWidget*widget);
// 添加圆角遮罩
QPixmap to_round_pixmap(const QPixmap& src, int radius = 10);
// 添加图形遮罩
QPixmap apply_mask(const QPixmap& source, const QPixmap& mask);

// ====================================================================== //
// 获取随机字符串作为全局唯一的 ID
// QUuid::WithBraces 形如：{00000000-0000-0000-0000-000000000000}  // 8-4-4-4-12=32 位
// QUuid::Id128 : 不使用间隔符
QString get_new_uuid_string();

// ====================================================================== //
// 获取错误信息
QString to_string(::TaoTieChat::SuccessCodeGadget::SuccessCode code);
QString to_string(::TaoTieChat::ErrorCodeGadget::ErrorCode     code);
void set(TaoTieChat::Result&result, const ::TaoTieChat::SuccessCodeGadget::SuccessCode&success_code);
void set(TaoTieChat::Result&result, const ::TaoTieChat::ErrorCodeGadget   ::ErrorCode &error_code  );

// ====================================================================== //
// 查找指定 id 的好友
Contact get_contact(Contacts contacts, QString contact_id);
// 从好友列表中获取第 index 个好友
Contact get_contact(Contacts contacts, int index);
};
