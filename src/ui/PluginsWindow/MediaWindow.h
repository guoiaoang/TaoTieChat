#pragma once

#include "Utils.h"

namespace Ui {
class MediaWindow;
}

struct MediaInfo {
    QString media_path  ;
    QPixmap media_pixmap;
    QString media_name  ;
};

class MediaWindow : public QMainWindow {
    Q_OBJECT
private:
    explicit MediaWindow(QWidget *parent = nullptr);
    MediaWindow (const MediaWindow&) = delete;
    MediaWindow& operator=(const MediaWindow&) = delete;
public:
    ~MediaWindow();
    static MediaWindow &instance() {
        static MediaWindow s_media_window;
        return s_media_window;
    }

public:
    void set_play_list(const std::vector<std::shared_ptr<MediaInfo>>&play_list);
    const std::vector<std::shared_ptr<MediaInfo>> get_play_list() const;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void show_media(const std::shared_ptr<MediaInfo>&media_info);

signals:
    void media_changed(const QPixmap&pixmap);

public slots:
    void on_media_open        (            );
    void on_media_save        (            );
    void on_media_save_as     (            );
    void on_media_prev        (            );
    void on_media_next        (            );
    void on_media_fit         (bool checked);
    void on_media_zoom_in     (            );
    void on_media_zoom_out    (            );
    void on_media_normal_size (            );
    void on_window_full_screen(bool checked);
    void on_window_pin        (bool checked);

private:
    // UI 组件
    std::shared_ptr<Ui::MediaWindow> m_ui           ;
    std::shared_ptr<QToolBar       > m_toolbar      ;
    std::shared_ptr<QMenu          > m_menu_save    ;  // 另存为
    std::shared_ptr<QActionGroup   > m_actions_scale;
    // 菜单和工具栏的动作
    std::shared_ptr<QAction> m_action_media_open        ;  // 打开
    std::shared_ptr<QAction> m_action_media_save        ;  // 保存
    std::shared_ptr<QAction> m_action_media_save_as     ;  // 另存为
    std::shared_ptr<QAction> m_action_media_prev        ;  // 上一个
    std::shared_ptr<QAction> m_action_media_next        ;  // 下一个
    std::shared_ptr<QAction> m_action_media_fit         ;  // 适应窗口
    std::shared_ptr<QAction> m_action_media_zoom_in     ;  // 放大
    std::shared_ptr<QAction> m_action_media_zoom_out    ;  // 缩小
    std::shared_ptr<QAction> m_action_media_normal_size ;  // 原始大小
    std::shared_ptr<QAction> m_action_window_full_screen;  // 全屏显示
    std::shared_ptr<QAction> m_action_window_pin        ;  // 窗口置顶
    Utils::FontIcons m_icons = Utils::FontIcons().reset().update();  // 图标工具
    // 状态记录
    bool   m_fit_to_window = true;  // 图片自适应窗口大小
    double m_scale_factory = 1.0;   // 图片非自适应窗口大小时的缩放倍数
    // 数据
    // 播放列表
    std::vector<std::shared_ptr<MediaInfo>> m_media_infos;
    int64_t m_media_current_index = -1;
};

// 可参考项目：
//   * Qt/Examples/Qt-5.15.2/widgets/widgets/imageviewer/imageviewer.cpp
//   * Qt/Examples/Qt-5.15.2/widgets/widgets/movie/movieplayer.cpp
