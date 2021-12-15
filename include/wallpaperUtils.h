#ifndef WALLPAPERUTILS_H
#define WALLPAPERUTILS_H

#include <QColor>
#include <QFileSystemWatcher>
#include <KConfigGroup>
#include <KSharedConfig>
#include "cluster.h"

const QString getWallpaperConfigFilePath(void);
const QString getGlobalConfigFilePath(void);
const QString getCurrentColorScheme(KSharedConfigPtr& globalConfig);
const bool fileExists(QString);
void showNotification(QString appName="", QString appIcon="", QString summary="", QString body="", int timeout=1000);
void showNonExistentConfigFileError(void);
void startMonitoringForWallpaperChange(QFileSystemWatcher&, QString file, std::function<void(void)>& onWallpaperChanged);
void setAccentColor(KSharedConfigPtr& globalConfig, QString accentColor);
double getDistance(QColor, QColor);
QVector<QColor> getClusters(QVector<QColor>& pixelVector, size_t numberOfCluster);
void assignPixelsToClusters(QVector<QColor>& pixelVector, QVector<QColor>& clusterVector, Cluster clusters[]);
QVector<QColor> kmean(QVector<QColor>& pixelVector, QVector<QColor>& clusterVector, size_t iteration=2);
#endif // WALLPAPERUTILS_H
