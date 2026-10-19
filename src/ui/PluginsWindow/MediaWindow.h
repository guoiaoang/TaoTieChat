#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

namespace Ui {
class MediaWindow;
}

struct MediaInfo {
    QString media_path ;
    QImage  media_image;
    QString media_name ;
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
    void show_media(std::shared_ptr<MediaInfo> media_info);

// signals:
//     void media_changed(const QPixmap&pixmap);

public slots:
    void slot_media_open        (            );
    void slot_media_save        (            );
    void slot_media_save_as     (            );
    void slot_media_prev        (            );
    void slot_media_next        (            );
    void slot_media_fit         (bool checked);
    void slot_media_zoom_in     (            );
    void slot_media_zoom_out    (            );
    void slot_media_normal_size (            );
    void slot_window_full_screen(bool checked);
    void slot_window_pin        (bool checked);

private:
    // UI 组件
    std::shared_ptr<Ui::MediaWindow> m_ui           ;
    QToolBar*                        m_toolbar      = nullptr;
    QMenu*                           m_menu_save    = nullptr;  // 另存为
    QActionGroup*                    m_actions_scale= nullptr;
    // 菜单和工具栏的动作（使用 Qt 父对象管理内存，避免 shared_ptr 与 QObject 的双重释放）
    QAction* m_action_media_open        = nullptr;  // 打开
    QAction* m_action_media_save        = nullptr;  // 保存
    QAction* m_action_media_save_as     = nullptr;  // 另存为
    QAction* m_action_media_prev        = nullptr;  // 上一个
    QAction* m_action_media_next        = nullptr;  // 下一个
    QAction* m_action_media_fit         = nullptr;  // 适应窗口
    QAction* m_action_media_zoom_in     = nullptr;  // 放大
    QAction* m_action_media_zoom_out    = nullptr;  // 缩小
    QAction* m_action_media_normal_size = nullptr;  // 原始大小
    QAction* m_action_window_full_screen= nullptr;  // 全屏显示
    QAction* m_action_window_pin        = nullptr;  // 窗口置顶
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
