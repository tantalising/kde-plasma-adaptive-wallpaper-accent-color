#include <QtDBus/QtDBus>
#include <QFileInfo>
#include <random>
#include "wallpaperUtils.h"

const QString getWallpaperConfigFilePath(void) {
    auto const homedir = qgetenv("HOME");
    auto const wallpaperConfigFile = homedir + "/.config/plasma-org.kde.plasma.desktop-appletsrc";
    return wallpaperConfigFile;
}

const QString getGlobalConfigFilePath(void) {
    auto const homedir = qgetenv("HOME");
    auto const globalConfigFile = homedir + "/.config/kdeglobals";
    return globalConfigFile;
}


const bool fileExists(QString filePath){
    return QFileInfo::exists(filePath) && QFileInfo(filePath).isFile();
}


const QString getCurrentColorScheme(KSharedConfigPtr& globalConfig){
    KConfigGroup generalGroup(globalConfig, QStringLiteral("General"));
    generalGroup.sync();
    auto colorscheme = generalGroup.readEntry("ColorScheme", "");
    if(colorscheme == "") {
        colorscheme = QStringLiteral("BreezeLight"); // at least let me assume breeze is there
    }
    generalGroup.writeEntry("ColorScheme", colorscheme+".temp");
    generalGroup.sync();
    return colorscheme;
}

void setAccentColor(KSharedConfigPtr& globalConfig, QString accentColor){
    KConfigGroup generalGroup(globalConfig, QStringLiteral("General"));
    generalGroup.writeEntry(QStringLiteral("AccentColor"), accentColor);
    auto currentColorScheme = getCurrentColorScheme(globalConfig);

    std::system(QString("plasma-apply-colorscheme %1").arg(currentColorScheme).toLatin1());
    generalGroup.writeEntry(QStringLiteral("ColorScheme"), currentColorScheme);
    generalGroup.sync();

}

void showNotification(QString appName, QString appIcon, QString summary, QString body, int timeout) {
    QDBusMessage errorMessage = QDBusMessage::createMethodCall("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  "Notify");

    const uint id = QRandomGenerator::global()->generate();

    const QStringList actionList = {};
    QVariantMap hints = {};
    hints["urgency"] = QVariant(1);


    QList<QVariant> args = {appName, id, appIcon, summary, body, actionList, hints, timeout};
    errorMessage.setArguments(args);
    QDBusConnection::sessionBus().send(errorMessage);
}

void showNonExistentConfigFileError(void) {
       const QString appName = "Plasma Adaptive Accent Color";
       const QString appIcon = "error";
       const QString summary = "Invalid wallpaper config file";
       const QString body = "The config file for wallpaper has not been found. This is most probably a bug in this app. \
                              Thank you for your support.";
       int timeout = 10000; //20 seconds; enough to read the message and become disappointed

       showNotification(appName, appIcon, summary, body, timeout);
}

void startMonitoringForWallpaperChange(QFileSystemWatcher& watcher, QString file, std::function<void(void)>& onWallpaperChanged){
    watcher.addPath(file);
    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged,  [&watcher, file, onWallpaperChanged]() {
        watcher.addPath(file); // Something is removing the watch so let just re add this
        onWallpaperChanged();
        }
    );
}

double getDistance(QColor firstColor, QColor secondColor){
    const auto redDifference = std::pow(firstColor.red() - secondColor.red(), 2.0);
    const auto greenDifference = std::pow(firstColor.green() - secondColor.green(), 2.0);
    const auto blueDifference = std::pow(firstColor.blue() - secondColor.blue(), 2.0);
    return std::sqrt(redDifference + greenDifference + blueDifference);
}

QVector<QColor> getClusters(QVector<QColor>& pixelVector, size_t numberOfCluster){
    if(numberOfCluster == 0 || numberOfCluster == 1){
        return {QColor(0,0,0)};
    }
    QVector<QColor> clusterVector;


    std::sample(pixelVector.begin(), pixelVector.end(), std::back_inserter(clusterVector),
                numberOfCluster, std::mt19937{std::random_device{}()});

    return clusterVector;
}

void assignPixelsToClusters(QVector<QColor>& pixelVector, QVector<QColor>& clusterVector, Cluster clusters[])
{
    double distance = std::numeric_limits<double>::infinity();
    size_t nearestClusterIndex = -1;

    for(size_t i=0; i < pixelVector.length(); i++) {
          for(size_t j=0; j < clusterVector.length(); j++){
              auto current_distance = getDistance(pixelVector[i], clusterVector[j]);
              if(current_distance < distance) {
                  distance = current_distance;
                  nearestClusterIndex = j;
              }
          }

          clusters[nearestClusterIndex].appendNeighbour(pixelVector[i]);

          // remove the assigned pixel from other clusters in case that cluster contains that pixel
          for(size_t k=0; k < clusterVector.length(); k++){
              if(k != nearestClusterIndex){
                  clusters[k].removeNeighbour(pixelVector[i]);
              }
          }
    }
}


QVector<QColor> kmean(QVector<QColor>& pixelVector, QVector<QColor>& clusterVector, size_t iteration){
    QVector<QColor> accentColorVector;

    // make and init clusterVector.length() number of cluster objects
    Cluster clusters[clusterVector.length()];
    for(size_t i=0; i < clusterVector.length(); i++){
        clusters[i].setCluster(clusterVector[i]);
    }

    // find the nearest cluster for each pixel and
    // then append that pixel to that vector
    // until the clusters converge or iteration ends
    for(size_t i=0; i < iteration; i++){
        assignPixelsToClusters(pixelVector, clusterVector, clusters);
        //Recalculate cluster value for more accurate cluster
        for(size_t i=0; i < clusterVector.length(); i++){
            clusters[i].recalculateCluster();
        }

        // check for convergence
        bool isConverged = false;
        for(size_t i=0; i < clusterVector.length(); i++){
            if(clusters[i].doesNeedRecalculating()){
                break;
            }
           isConverged = true;
        }
        if(isConverged) {
            break;
        }
    }

    // sort the vectors in descending order(first one is regarded here as primary accent color)
    const auto sorter = [](const Cluster& first, const Cluster& second){return second < first;};
    std::sort(clusters, clusters+clusterVector.length());
    // make the final cluster vector and then return it
    for(size_t i=0; i < clusterVector.length(); i++){
        accentColorVector.append(clusters[i].getCluster());
    }
    return clusterVector;
}
