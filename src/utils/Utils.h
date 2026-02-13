#pragma once

// std cxx
#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <filesystem>
// qt
#include <QPainter>
#include <QBuffer>
#include <QIcon>
#include <QImage>
#include <QBitmap>
#include <QPicture>
#include <QPixmap>
#include <QSvgGenerator>
#include <QImageReader>
#include <QImageWriter>
#include <QFont>
#include <QPalette>
#include <QStyle>
#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QToolBar>
#include <QToolBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>          // 遍历子目录，较老版本 // QDir::entryList()
#include <QDirIterator>  // 遍历子目录，较新版本，单个遍历
#include <QDirListing>   // 遍历子目录，最新版本，c++20 方式
#include <QStringList>
#include <QMetaEnum>
#include <QSvgWidget>
#include <QSvgRenderer>
#include <QPropertyAnimation>
#include <QMovie>
#include <QActionGroup>
#include <QMenu>
#include <QList>
#include <QString>
#include <QObject>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
#include <QtDebug>
#include <QDialog>
#include <QSplitter>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QButtonGroup>
#include <QApplication>
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QScreen>
#include <QThread>
#include <QThreadPool>
#include <QLocalSocket>
#include <QLocalServer>
#include <QMutex>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFontDatabase>
#include <QFont>

class Utils {
public:
    Utils();

public:
    static void InitDebugOutputFormat() {
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

    static bool load_translator(QApplication&application) {
        QTranslator translator;
        const QStringList ui_languages = QLocale::system().uiLanguages();
        for (const QString &locale : ui_languages) {
            const QString base_name = "TaoTieChat_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + base_name)) {
                application.installTranslator(&translator);
                // break;
                return true;
            }
        }
        return false;
    }

    // 遍历指定路径获取所有文件和文件夹列表
    // path: 目标路径; recursive: 递归子目录
    static QList<QFileInfo> dir_contents(const QString &path, bool recursive = false) {
        QList<QFileInfo> lists;
        using ItFlag = QDirListing::IteratorFlag;
        for (const auto &dirEntry : QDirListing(path, recursive ? ItFlag::Recursive : ItFlag::Default)) {
            lists << dirEntry.fileInfo();
        }
        return lists;
    }

    // 获取给定大小内指定字体最适合的最大大小
    // 字体名称，矩形大小，绘制文本，精度
    static QFont get_suitable_font_size(const QString&font_name, const QSizeF&size, const QString&text, const qreal precision = 1) {
        if(font_name.isEmpty()) {
            return QFont();
        }
        QFont font(font_name);
        return get_suitable_font_size(font, size, text);
    }

    // 字体，矩形大小，绘制文本，精度
    static QFont get_suitable_font_size(QFont&font, const QSizeF&size, const QString&text, const qreal precision = 1) {
        QRectF rect;
        rect.setSize(size);
        if(!rect.isValid() || text.isEmpty()) {
            return QFont();
        }
        constexpr qreal font_size_min = 1;
        constexpr qreal font_size_max = 2500;

        // QFont font(font_name);
        qreal font_size_old = font.pointSizeF();
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
    static bool load_font(const QString&font_path) {
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
    static QVector<FontData> &f_member_fonts() {
        static QVector<FontData> m_vector_fonts;
        return m_vector_fonts;
    }
};
