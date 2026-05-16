#pragma once

#include "Utils.h"

namespace Ui {
class MediaWindow;
}

class MediaWindow : public QMainWindow {
    Q_OBJECT
public:
    static MediaWindow &instance() {
        static MediaWindow s_media_window;
        return s_media_window;
    }
private:
    explicit MediaWindow(QWidget *parent = nullptr);
    MediaWindow              (const MediaWindow & ) = delete;
    MediaWindow              (      MediaWindow &&) = delete;
    MediaWindow & operator = (const MediaWindow & ) = delete;
    MediaWindow & operator = (      MediaWindow &&) = delete;
public:
    ~MediaWindow();

public:
    void clean();
    bool load(const QString&media_path);
    bool load(const QPixmap&media_pixmap);
    void set_media_name(const QString&media_name);
    bool update_media();
    bool update_title();

signals:
    void media_changed(const QPixmap&pixmap);

public slots:
    void on_media_open();
    void on_media_save();
    void on_media_save_as();

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    std::shared_ptr<Ui::MediaWindow> m_ui          ;
    std::shared_ptr<QToolBar       > m_toolbar     ;
    std::shared_ptr<QMenu          > m_menu_save   ;  // 另存为
    std::shared_ptr<QActionGroup   > m_action_group;

    std::shared_ptr<QAction> m_action_open       ;  // 打开
    std::shared_ptr<QAction> m_action_save       ;  // 保存
    std::shared_ptr<QAction> m_action_save_as    ;  // 另存为
    std::shared_ptr<QAction> m_action_prev       ;  // 上一个
    std::shared_ptr<QAction> m_action_next       ;  // 下一个
    std::shared_ptr<QAction> m_action_fit        ;  // 适应窗口
    std::shared_ptr<QAction> m_action_zoom_in    ;  // 放大
    std::shared_ptr<QAction> m_action_zoom_out   ;  // 缩小
    std::shared_ptr<QAction> m_action_normal_size;  // 原始大小
    std::shared_ptr<QAction> m_action_full_screen;  // 全屏显示
    std::shared_ptr<QAction> m_action_pin        ;  // 窗口置顶

    QString  m_media_path  ;
    QPixmap  m_media_pixmap;
    QString  m_media_name  ;

    bool   m_fit_to_window = true;  // 图片自适应窗口大小
    double m_scale_factory = 1.0;   // 图片非自适应窗口大小时的缩放倍数
};

// 可参考项目：
//   * Qt/Examples/Qt-5.15.2/widgets/widgets/imageviewer/imageviewer.cpp
//   * Qt/Examples/Qt-5.15.2/widgets/widgets/movie/movieplayer.cpp
