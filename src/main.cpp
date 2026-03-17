#include "MainWindow.h"
#include "Utils.h"

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE_EXPAND(PROJECT_NAME);
    QApplication application(argc, argv);
    application.setApplicationName(TOSTRING(PROJECT_NAME));
    Utils::init_debug_output_format();
    Utils::load_translator(application);
    Utils::load_font(":/rc/fonts/InconsolataNerdFontMono-Regular.ttf");

    MainWindow main_window;
    main_window.show();

    return application.exec();
}
