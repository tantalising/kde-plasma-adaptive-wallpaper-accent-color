#include <QCoreApplication>
#include <QDebug>
#include <KSharedConfig>
#include <QCommandLineParser>
#include "wallpaperUtils.h"
#include "wallpaper.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("app.tantalising");
    QCoreApplication::setApplicationName("plasma adaptive accent color");

    QCommandLineParser *parser = new QCommandLineParser;
    parser->addHelpOption();
    parser->setApplicationDescription(
        "This tool automatically sets an accent color generated from wallpaper.");
    parser->addOption(
        QCommandLineOption(QStringLiteral("version"), "Shows the application version"));

    parser->process(app);

    if(parser->isSet("version")){
        qInfo() << "1.0.3";
        exit(0);
    }

    const auto wallpaperConfigFilePath = getWallpaperConfigFilePath();
    if(!fileExists(wallpaperConfigFilePath)) {
        showNonExistentConfigFileError();
        app.quit();
    }

    const auto globalConfigFilePath = getGlobalConfigFilePath();
    if(!fileExists(globalConfigFilePath)) {
        showNonExistentConfigFileError();
        app.quit();
    }

    Wallpaper wallpaper(wallpaperConfigFilePath);
    auto globalConfig = KSharedConfig::openConfig(globalConfigFilePath);
    auto callback = [&wallpaper, &globalConfig](){
        wallpaper.updateWallpaperInfo();
        wallpaper.applyAccentColor(globalConfig);
    };

    std::function<void(void)> onWallpaperChange = callback;

        // Don't wait for wallpaper change on start. Just set it!
        wallpaper.updateWallpaperInfo();
        wallpaper.applyAccentColor(globalConfig);

        // Start monitoring for wallpaper change and change the accent color accordingly
        QFileSystemWatcher watcher;
        startMonitoringForWallpaperChange(watcher, wallpaperConfigFilePath, onWallpaperChange);

    return app.exec();
}

