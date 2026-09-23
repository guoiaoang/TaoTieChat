#include "MediaWindow.h"
#include "ui_MediaWindow.h"

MediaWindow::MediaWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(std::make_shared<Ui::MediaWindow>()) {
    m_ui->setupUi(this);

    // new: use Qt parent ownership (raw pointers) to avoid double-delete
    m_toolbar       = new QToolBar(this);
    m_menu_save     = new QMenu(this);
    m_actions_scale = new QActionGroup(this);

    // 初始化需要的图标
    m_icons.reset();
    m_icons.color.normal   = this->palette().color(QPalette::Normal  , QPalette::WindowText);  // 常规状态图标颜色
    m_icons.color.disabled = this->palette().color(QPalette::Disabled, QPalette::WindowText);  // 禁用状态图标颜色
    m_icons.update();

    auto init_action = [this](
        QAction*& action,
        const QIcon& icon,  const QString& text,
        const QKeySequence& shortcut,
        bool visible  , bool enabled,
        bool checkable, bool checked
    ) {
        if(!action) { action = new QAction(this); }
        action->setIcon     (icon     );  // 设置动作图标
        action->setText     (text     );  // 设置动作文本
        action->setShortcut (shortcut );  // 设置动作快捷键
        action->setVisible  (visible  );  // 根据需要显示或隐藏动作
        action->setEnabled  (enabled  );  // 根据需要启用或禁用动作
        action->setCheckable(checkable);  // 设置动作是否可选中
        action->setChecked  (checked  );  // 设置动作的选中状态
    };

    init_action(m_action_media_open        , m_icons.icon.folder_open  , tr("Open"         ), QKeySequence(QKeySequence::Open      ), true, true , false, false);  // 打开
    init_action(m_action_media_save        , m_icons.icon.save         , tr("Save"         ), QKeySequence(QKeySequence::Save      ), true, true , false, false);  // 保存
    init_action(m_action_media_save_as     , m_icons.icon.save_edit    , tr("Save As"      ), QKeySequence(QKeySequence::SaveAs    ), true, true , false, false);  // 另存为
    init_action(m_action_media_prev        , m_icons.icon.arrow_left   , tr("Prev"         ), QKeySequence(QKeySequence::Back      ), true, false, false, false);  // 上一个
    init_action(m_action_media_next        , m_icons.icon.arrow_right  , tr("Next"         ), QKeySequence(QKeySequence::Forward   ), true, false, false, false);  // 下一个
    init_action(m_action_media_fit         , m_icons.icon.fit_to_screen, tr("Fit To Window"), QKeySequence(                        ), true, true , true , true );  // 适应窗口
    init_action(m_action_media_zoom_in     , m_icons.icon.zoom_in      , tr("Zoom In"      ), QKeySequence(QKeySequence::ZoomIn    ), true, false, true , false);  // 放大
    init_action(m_action_media_zoom_out    , m_icons.icon.zoom_out     , tr("Zoom Out"     ), QKeySequence(QKeySequence::ZoomOut   ), true, false, true , false);  // 缩小
    init_action(m_action_media_normal_size , m_icons.icon.restore      , tr("Normal Size"  ), QKeySequence(                        ), true, true , true , false);  // 原始大小
    init_action(m_action_window_full_screen, m_icons.icon.fullscreen   , tr("Full Screen"  ), QKeySequence(QKeySequence::FullScreen), true, true , true , false);  // 全屏显示
    init_action(m_action_window_pin        , m_icons.icon.pin          , tr("Pin"          ), QKeySequence(                        ), true, true , true , false);  // 窗口置顶

    QObject::connect(m_action_media_open        , &QAction::triggered, this, &MediaWindow::slot_media_open        );
    QObject::connect(m_action_media_save        , &QAction::triggered, this, &MediaWindow::slot_media_save        );
    QObject::connect(m_action_media_save_as     , &QAction::triggered, this, &MediaWindow::slot_media_save_as     );
    QObject::connect(m_action_media_prev        , &QAction::triggered, this, &MediaWindow::slot_media_prev        );
    QObject::connect(m_action_media_next        , &QAction::triggered, this, &MediaWindow::slot_media_next        );
    QObject::connect(m_action_media_fit         , &QAction::triggered, this, &MediaWindow::slot_media_fit         );
    QObject::connect(m_action_media_zoom_in     , &QAction::triggered, this, &MediaWindow::slot_media_zoom_in     );
    QObject::connect(m_action_media_zoom_out    , &QAction::triggered, this, &MediaWindow::slot_media_zoom_out    );
    QObject::connect(m_action_media_normal_size , &QAction::triggered, this, &MediaWindow::slot_media_normal_size );
    QObject::connect(m_action_window_full_screen, &QAction::triggered, this, &MediaWindow::slot_window_full_screen);
    QObject::connect(m_action_window_pin        , &QAction::triggered, this, &MediaWindow::slot_window_pin        );

    // static inline QMetaObject::Connection QObject::connect<...>(const QAction *sender, void (QAction::*signal)(bool checked), const MediaWindow *context, void (MediaWindow::*&&slot)(), Qt::ConnectionType type = Qt::AutoConnection)
    // connect(m_action_open.get(), &QAction::triggered, this, &MediaWindow::slot_media_open);

    auto make_separator = [this]() -> QAction * {
        QAction *action = new QAction(this);
        action->setSeparator(true);
        return action;
    };
    const QList<QAction *> actions_scale{
        m_action_media_fit        ,
        m_action_media_zoom_in    ,
        m_action_media_zoom_out   ,
        m_action_media_normal_size
    };
    for(auto&action : actions_scale) {
        m_actions_scale->addAction(action);
    }
    // add to toolbar
    const QList<QAction *> toolbar_actions{
        // m_action_open,
        // m_action_save,
        // make_separator(),
        m_action_media_save_as,
        make_separator(),
        m_action_media_prev,
        m_action_media_next,
        make_separator(),
        m_action_media_fit,
        m_action_media_zoom_in,
        m_action_media_zoom_out,
        m_action_media_normal_size,
        make_separator(),
        m_action_window_full_screen,
        m_action_window_pin
    };
    m_toolbar->addActions(toolbar_actions);

    // action saveas  // 另存为按钮作为扩展按钮 - 保存按钮的菜单
    m_menu_save        ->addAction(m_action_media_save_as);
    m_action_media_save->setMenu  (m_menu_save           );
    // m_menu_save->setStyle();
    m_action_media_open        ->setMenuRole(QAction::ApplicationSpecificRole);
    m_action_window_full_screen->setMenuRole(QAction::TextHeuristicRole      );

    // toolbar options
    m_toolbar->setWindowTitle(tr("Toolbar"));
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(32, 32)); // 这里设置显示的图标大小
    m_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);  // 不显示右键菜单
    // add toolbar to window
    this->addToolBar(m_toolbar);

    connect(this, &MediaWindow::media_changed, m_ui->label, &QLabel::setPixmap);
    m_ui->label->setAlignment(Qt::AlignCenter);
    // 设置 label 的尺寸策略，使其能够收缩以避免出现滚动条
    m_ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // 不自动缩放内容
    m_ui->label->setScaledContents(false);
    // m_ui->label->installEventFilter(this);
    // m_ui->layout_central->installEventFilter(this);
    m_ui->scrollArea->viewport()->installEventFilter(this);
    // m_ui->scrollArea->setWidgetResizable(true);

    // 窗口最大化、还原时触发图片自动拉伸
    this->installEventFilter(this);

    // TODO:功能已经支持，可以启用。但放大时滚动条功能目前暂未实现。
    m_action_media_zoom_in ->setEnabled(true);
    m_action_media_zoom_out->setEnabled(true);
}

MediaWindow::~MediaWindow() {
}

// TODO:会被多次重复调用，建议添加缓存或过滤操作
void MediaWindow::show_media(const std::shared_ptr<MediaInfo>&media_info) {
    QString str_title_right = QString("%1 %2").arg(QApplication::applicationName()).arg(tr("Media View"));
    if(!media_info) {
        // reset
        m_ui->label->setPixmap(QPixmap());
        this->setWindowTitle(str_title_right);
        return;
    }

    QPixmap media_pixmap = media_info->media_pixmap;
    if(media_pixmap.isNull()) {
        // 缓存，防止缩放时多次从磁盘读取文件。最多仅需保存最近的一张照片。
        static QMap<QString, QPixmap> s_cache_pixmap;

        // 如果有缓存就不再读取文件
        if (!media_info->media_path.isEmpty() && s_cache_pixmap.contains(media_info->media_path)) {
            media_pixmap = s_cache_pixmap.value(media_info->media_path);
        }
        else {
            s_cache_pixmap.clear();  // 清除缓存
            QFileInfo file_info(media_info->media_path);
            media_pixmap.load(media_info->media_path);
            s_cache_pixmap[media_info->media_path] = media_pixmap;
        }
    }

    // TODO:添加缓存
    if(m_fit_to_window) {
        // 更新缩放图片：使其适应 Label 大小，并保持纵横比
        // Qt::KeepAspectRatio 保证图片不变形
        // Qt::SmoothTransformation 保证缩放后的图片清晰（消除锯齿）
        QPixmap pixmap_scaled = media_pixmap.scaled(m_ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_ui->label->setPixmap(pixmap_scaled);

        // 同步更新缩放比例，防止默认在原图比例上进行缩放
        // 计算宽度和高度各自的缩放比例
        double scale_w = static_cast<double>(m_ui->label->width()) / media_pixmap.width();
        double scale_h = static_cast<double>(m_ui->label->height()) / media_pixmap.height();
        // KeepAspectRatio 模式下，实际缩放倍数是两者中的【最小值】
        m_scale_factory = std::min(scale_w, scale_h);
    }
    else {
        // 检测精度
        const double ff = 0.001;
        if(m_scale_factory > 1.0 - ff && m_scale_factory < 1.0 + ff) {
            // 如果缩放倍数为 1 附近的值，那么直接显示原图，避免不必要的缩放计算和可能的模糊
            m_ui->label->setPixmap(media_pixmap);
        }
        else {
            // 缩放倍数不为 1 时需要进行处理
            QPixmap pixmap_scaled = media_pixmap.scaled(media_pixmap.size() * m_scale_factory, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_ui->label->setPixmap(pixmap_scaled);
        }
    }

    // title
    auto play_list_count = m_media_infos.size();
    QString str_media_title = QFileInfo(media_info->media_path).fileName();  // 仅显示文件名
    if(str_media_title.isEmpty()) {
        str_media_title = media_info->media_name;
    }
    QString str_index_count;
    if(m_media_infos.size() > 1) {
        str_index_count == tr("[%1 / %2]").arg(m_media_current_index).arg(play_list_count);
    }
    QString str_title_left;
    if(!str_media_title.isEmpty()) {
        str_title_left = str_index_count + str_media_title + " - ";
    }
    this->setWindowTitle(str_title_left + str_title_right);
}

void MediaWindow::set_play_list(const std::vector<std::shared_ptr<MediaInfo>>&play_list) {
    m_media_infos = play_list;
    m_media_current_index = m_media_infos.empty() ? -1 : 0;
    std::shared_ptr<MediaInfo> media_info;
    if( m_media_current_index >= 0) {
        media_info = m_media_infos.at(m_media_current_index);
    }
    show_media(media_info);
}

const std::vector<std::shared_ptr<MediaInfo>> MediaWindow::get_play_list() const {
    return m_media_infos;
}

void MediaWindow::slot_media_open() {
    QFileDialog dialog(this, tr("Open File"));
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;

    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave) {
        dialog.setDefaultSuffix("jpg");
    }

    auto loadFile = [this](const QString &fileName) {
        QPixmap pixmap(fileName);
        if(pixmap.isNull()) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(), tr("Cannot load %1").arg(QDir::toNativeSeparators(fileName)));
            return false;
        }

        set_play_list({ std::make_shared<MediaInfo>(MediaInfo{.media_path = fileName})});

        const QString message = tr(R"(Opened "%1", %2x%3, Depth: %4)").arg(QDir::toNativeSeparators(fileName)).arg(pixmap.width()).arg(pixmap.height()).arg(pixmap.depth());
        statusBar()->showMessage(message);
        return true;
    };

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void MediaWindow::slot_media_save() {
    // QFileInfo file_info(m_media_path);
}

void MediaWindow::slot_media_save_as() {
    // 源文件信息
    std::shared_ptr<MediaInfo> media_info;
    if(!m_media_infos.empty() && m_media_current_index >= 0 && m_media_current_index < int64_t(m_media_infos.size())) {
        media_info = m_media_infos.at(m_media_current_index);
    }
    QString media_path   = media_info ? media_info->media_path : "";
    QFileInfo file_info(media_path);
    QString media_suffix = file_info.suffix().toLower();
    bool is_file_exists  = !media_path.isEmpty() && file_info.exists();

    // 默认文件名和后缀
    // 如果源文件存在，默认使用源文件的名字，否则给一个默认名称
    QString default_suffix    = !media_suffix.isEmpty() ? media_suffix : "jpg";
    // `TaotieChat_ + 时间戳 + .后缀` --> `TaotieChat_20260918T001423M976Z+0800.jpg`
    QString time_str  = QDateTime::currentDateTimeUtc().toLocalTime().toString("yyyyMMdd'T'hhmmss'M'zzz'Z'tt");
    QString time_name = QString("TaotieChat_%1.%2").arg(time_str).arg(default_suffix);
    QString default_file_name = is_file_exists ? file_info.fileName() : time_name;

    // 弹窗
    QFileDialog dialog(this, tr("Save File As"));
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptSave;

    // 设置默认目录
    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());

    // 添加类型过滤
    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);

    // 设置当前的类型过滤
    // 动态根据源文件后缀设置默认文件名、后缀与 MIME 筛选器
    // 根据后缀匹配默认的 MIME 类型（如 png -> image/png, jpg -> image/jpeg）
    QMimeDatabase mime_database;
    QMimeType mime_type = mime_database.mimeTypeForFile(media_path, QMimeDatabase::MatchExtension);
    QString select_filter = mime_type.isValid() && mimeTypeFilters.contains(mime_type.name()) ? mime_type.name() : "image/jpeg";
    dialog.selectMimeTypeFilter(select_filter);

    // 设置默认弹出的文件名
    dialog.selectFile(default_file_name);
    // 设置用户如果不输入后缀时的自动补全后缀
    dialog.setDefaultSuffix(default_suffix);

    if (dialog.exec() == QDialog::Accepted) {
        // 保存文件的信息
        const QString save_path         = dialog.selectedFiles().first();
        const QString save_suffix       = QFileInfo(save_path).suffix().toLower();
        const QString natinve_save_path = QDir::toNativeSeparators(save_path);
        const QString message_successed = tr(R"(Wrote Success : "%1")").arg(QDir::toNativeSeparators(natinve_save_path));
        const QString message_failed    = tr(R"(Cannot write : %1)"   ).arg(QDir::toNativeSeparators(natinve_save_path));
        bool is_save_ok = false;
        do {
            if (save_path.isEmpty() || save_suffix.isEmpty()) { break; }  // 实际不会被触发

            // 保存时需要再次校验源文件信息
            std::shared_ptr<MediaInfo> media_info;
            if(!m_media_infos.empty() && m_media_current_index >= 0 && m_media_current_index < int64_t(m_media_infos.size())) {
                media_info = m_media_infos.at(m_media_current_index);
            }
            QString media_path   = media_info ? media_info->media_path : "";
            QFileInfo file_info(media_path);
            QString media_suffix = file_info.suffix().toLower();
            bool is_file_exists  = !media_path.isEmpty() && file_info.exists();

            // 文件存在且同后缀时，直接复制文件即可
            if (is_file_exists && save_suffix == media_suffix) {
                is_save_ok = QFile::copy(media_path, save_path);
            }
            // 文件后缀不一致时需要加载转码保存
            else if (is_file_exists && save_suffix == media_suffix) {
                QImage image(media_path);
                is_save_ok = image.save(save_path);
            }
            // 传入的是图像时(文件不存在时)
            else if (media_info && !media_info->media_pixmap.isNull()) {
                is_save_ok = media_info->media_pixmap.save(save_path);
            }
        } while (false);
        // 界面提示
        const QString message = is_save_ok ? message_successed : message_failed;
        statusBar()->showMessage(message);
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(), message);
    }
}

void MediaWindow::slot_media_prev() {
    if(m_media_current_index > 0) {
        m_media_current_index--;
        show_media(m_media_infos.at(m_media_current_index));
    }
}

void MediaWindow::slot_media_next() {
    if(m_media_current_index < int64_t(m_media_infos.size()) - 1) {
        m_media_current_index++;
        show_media(m_media_infos.at(m_media_current_index));
    }
}

void MediaWindow::slot_media_fit(bool checked) {
    m_fit_to_window = true;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::slot_media_zoom_in() {
    m_fit_to_window = false;
    m_scale_factory *= 1.25;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::slot_media_zoom_out() {
    m_fit_to_window = false;
    m_scale_factory /= 1.25;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::slot_media_normal_size () {
    m_fit_to_window = false;
    m_scale_factory = 1.0;
    show_media(m_media_infos.at(m_media_current_index));
}

void MediaWindow::slot_window_full_screen(bool checked) {
    // 用于恢复之前的窗口大小状态
    static auto window_state_old = this->windowState();
    if(m_action_window_full_screen->isChecked()) {
        // 更新保存之前的窗口大小状态
        window_state_old = this->windowState();
        // 执行命令
        this->showFullScreen();
        m_action_window_full_screen ->setIcon(m_icons.icon.window_restore);
    }
    else {
        // 还原窗口状态
        if(window_state_old == Qt::WindowNoState) {
            this->showNormal();
        }
        else if(window_state_old == Qt::WindowMinimized) {
            this->showMinimized();
        }
        else if(window_state_old == Qt::WindowMaximized) {
            // TODO: QT 的 showFullScreen() 状态到 showMaximized() 状态，会先恢复常规大小后再最大化
            this->showMaximized();
        }
        else if(window_state_old == Qt::WindowFullScreen) {
            this->showFullScreen();
        }
        m_action_window_full_screen ->setIcon(m_icons.icon.fullscreen);
    }
}

void MediaWindow::slot_window_pin(bool checked) {
    if(m_action_window_pin->isChecked()) {
        this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        m_action_window_pin ->setIcon(m_icons.icon.pin_off);
    }
    else {
        this->setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        m_action_window_pin ->setIcon(m_icons.icon.pin);
    }
    show(); // 必须调用 show() 重新刷新窗口状态
}

bool MediaWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::Resize) {
        if(watched == m_ui->label || watched == m_ui->scrollArea->viewport()) {
            show_media(m_media_infos.at(m_media_current_index));
        }
    }
    else if(event->type() == QEvent::WindowStateChange) {
        // 窗口最大化与还原时可能需要手动触发图片缩放的更新
        show_media(m_media_infos.at(m_media_current_index));
    }
    return QMainWindow::eventFilter(watched, event);
}
