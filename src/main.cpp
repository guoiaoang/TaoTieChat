#include "MainWindow.h"
#include "Utils.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(TaoTieChat);  // `TaoTieChat.qrc` 中的资源名称
    QApplication application(argc, argv);
    Utils::InitDebugOutputFormat();
    Utils::load_translator(application);
    Utils::load_font(":/rc/fonts/InconsolataNerdFontMono-Regular.ttf");

    MainWindow main_window;
    main_window.show();

    return application.exec();
}
