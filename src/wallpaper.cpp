#include "wallpaper.h"
#include <QCoreApplication>
#include <QDebug>
#include <QImage>
#include <KSharedConfig>
#include <KConfigGroup>
#include "wallpaperUtils.h"

Wallpaper::Wallpaper(QString wallpaperConfigFilePath) : wallpaperConfig(KSharedConfig::openConfig(wallpaperConfigFilePath))
{
    init(); //Don't unpack init here, it is used elsewhere also.
}

int Wallpaper::getResolutionOfMonitorConfig(KConfigGroup monitorConfig){
    auto const keys = monitorConfig.keyList();
    for(const auto &key: keys){
        if(key.contains("ItemGeometries-")){
            QString resolution;
            for(size_t j=15; j < key.length(); j++){ // 15 => removig the part "ItemGeometries-"
                auto currentChar = key.at(j);
                if(currentChar == 'x') break;
                resolution.append(currentChar);
            }
            return resolution.toInt();
        }
    }
    return 0; // This should not occur
}

KConfigGroup Wallpaper::getConfigOfHighestResolution(QVector<KConfigGroup> monitorConfigList){
    if(monitorConfigList.length() == 1) {
        return monitorConfigList.first();
    } else {
        auto highestResolutionConfig = monitorConfigList[0];
        auto currentResolution = 0;
        for(auto &&currentConfig: monitorConfigList){
            const auto resolutionOfcurrentConfig = getResolutionOfMonitorConfig(currentConfig);
            if(resolutionOfcurrentConfig > currentResolution){
                currentResolution = resolutionOfcurrentConfig;
                highestResolutionConfig = currentConfig;
            }
        }
        qDebug() << "Highest resolution is" << currentResolution;
        return highestResolutionConfig;
    }
}

QString Wallpaper::imagePath(void) const {
    auto path = imagePlugin.readPathEntry("Image", "I/know/fallback/path/is/not/needed");
    if(path.contains("file://")){
        return path.mid(7);
    }
    return path;
}

QString Wallpaper::slidePath(void) const {
    auto path = slideshowPlugin.readPathEntry("Image", "I/know/fallback/path/is/not/needed");
    if(path.contains("file://")){
        return path.mid(7);
    }
    return path;
}

void Wallpaper::updateWallpaperInfo(void) {
    wallpaperConfig->reparseConfiguration();
    init();
    const QString wallpaperplugin = highestResolutionMontiorConfig.readEntry("wallpaperplugin");
    if(wallpaperplugin != currentPlugin) {
        currentPlugin = wallpaperplugin;
    }
}

void Wallpaper::init(void) {
    const auto monitorConfigList = getMonitorConfigList();
    if(monitorConfigList.empty()) { // There should be at least one monitor, otherwise what you will do with accent color?
        showNonExistentConfigFileError();
        QCoreApplication::quit();
    }

    highestResolutionMontiorConfig = getConfigOfHighestResolution(monitorConfigList);

    const KConfigGroup intermediateGroup(&highestResolutionMontiorConfig, "Wallpaper");
    const KConfigGroup imageGroup(&intermediateGroup, "org.kde.image");
    const KConfigGroup slideGroup(&intermediateGroup, "org.kde.slideshow");

    imagePlugin = KConfigGroup(&imageGroup, "General");
    slideshowPlugin = KConfigGroup(&slideGroup, "General");
    currentPlugin = highestResolutionMontiorConfig.readEntry("wallpaperplugin");

    if(!lastUsedWallpaper.has_value()){
        lastUsedWallpaper = "";
    }
}

QVector<KConfigGroup> Wallpaper::getMonitorConfigList(void) const {
    const KConfigGroup containmentGroup(wallpaperConfig, QStringLiteral("Containments"));
    const QStringList containmentSubGroups = containmentGroup.groupList();

    QVector<KConfigGroup> monitorConfigList;
    for(const auto& subGroup: containmentSubGroups){
        const KConfigGroup currentSubGroup(&containmentGroup, subGroup);
        if(currentSubGroup.hasKey("ItemGeometriesHorizontal")) {
            monitorConfigList.append(currentSubGroup);
        }
    }
    return monitorConfigList;
}

QString Wallpaper::wallpaperPath(void) const {
    if(currentPlugin == "org.kde.image") {
        return imagePath();
    }
    return slidePath();
}

void Wallpaper::applyAccentColor(KSharedConfigPtr& globalConfig) {
    const auto currentWallpaper = wallpaperPath();
    if(lastUsedWallpaper != currentWallpaper) {
        lastUsedWallpaper = currentWallpaper;
        qInfo() << "applying accent color";
        QImage image = QImage(currentWallpaper);

        if(image.isNull()){
            qInfo() << "Invalid image. The wallpaper path is" << currentWallpaper;
            return;
        }

        image = image.scaled(200, 200);

        QVector<QColor> pixelVector;
        for(size_t x=0; x < image.width(); x++){
            for(size_t y=0; y < image.height(); y++){
                pixelVector.append(image.pixelColor(x,y));
            }
        }

        auto clusterVector = getClusters(pixelVector, 2);
        QVector<QColor> accentColors = kmean(pixelVector, clusterVector);
        QColor color = accentColors[0];

        // Most of the time saturation is bad, so let just increase it.
        // Also the value should be increased a bit to look better
        color.setHsv(color.hsvHue(), 255,std::min(int(color.value() * 1.5), 255));
        const auto r = QVariant(color.red()).toString();
        const auto g = QVariant(color.green()).toString();
        const auto b = QVariant(color.green()).toString();
        const QString accentColor = r+","+g+","+b;
        setAccentColor(globalConfig, accentColor);

    } else {
        qInfo() << "Got same wallpaper. No accent will be applied";
    }
}

Wallpaper::~Wallpaper(){
    wallpaperConfig->~KSharedConfig();
}
