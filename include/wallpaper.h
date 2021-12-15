#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QString>
#include <KConfigGroup>
#include <KSharedConfig>

class Wallpaper
{

public:
    explicit Wallpaper(QString);
    ~Wallpaper();
    void updateWallpaperInfo(void);
    void applyAccentColor(KSharedConfigPtr& globalConfig);

private:
    const KSharedConfigPtr wallpaperConfig;
    KConfigGroup highestResolutionMontiorConfig;
    KConfigGroup imagePlugin;
    KConfigGroup slideshowPlugin;
    QString currentPlugin;
    QString lastUsedWallpaper;

    QString imagePath(void) const;
    QString slidePath(void) const;
    QString wallpaperPath(void) const; //path according to current plugin
    static int getResolutionOfMonitorConfig(KConfigGroup monitorConfig);
    static KConfigGroup getConfigOfHighestResolution(QVector<KConfigGroup> monitorConfigList);
    QVector<KConfigGroup> getMonitorConfigList(void) const;
    void init(void);
};

#endif // WALLPAPER_H
