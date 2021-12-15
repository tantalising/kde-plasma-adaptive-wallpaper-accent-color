#include "cluster.h"
#include "cmath"

double getDistance(QColor firstColor, QColor secondColor);

Cluster::Cluster(QColor cluster, qreal epsilon=1) {
    Q_ASSERT(epsilon > 0);
      cluster = cluster;
      epsilon = epsilon;
      needsRecalculating = true;
    }
Cluster::Cluster(){
      needsRecalculating = true;
      epsilon = 0.005; //If this matters, then better set this using setEpsilon
    }
QColor Cluster::getCluster() const {
        return cluster;
    }

void Cluster::setCluster(QColor cluster) {
        cluster = cluster;
    }
qreal Cluster::getEpsilon() const {
        return epsilon;
    }

void Cluster::setEpsilon(qreal epsilon) {
    if(epsilon > 0){
        epsilon = epsilon;
        }
    }

void Cluster::appendNeighbour(QColor color){
        clusterNeighbourhood.append(color);
    }

void Cluster::removeNeighbour(QColor color) {
        const auto index = clusterNeighbourhood.indexOf(color);
        if(index != -1){
            clusterNeighbourhood.removeAt(index);
        }
    }

void Cluster::recalculateCluster(void) {
      if(!needsRecalculating){
          return;
      }
      oldCluster = cluster;

      QVector<qreal> redChannel;
      QVector<qreal> greenChannel;
      QVector<qreal> blueChannel;

      for(size_t i=0; i < clusterNeighbourhood.length(); i++){
          redChannel.append(clusterNeighbourhood[i].redF());
          greenChannel.append(clusterNeighbourhood[i].greenF());
          blueChannel.append(clusterNeighbourhood[i].blueF());
      }

      qreal r = 0;
      qreal g = 0;
      qreal b = 0;
      qreal a = 1;

      for(size_t i=0; i < redChannel.length(); i++){
         r += redChannel[i];
         g += greenChannel[i];
         b += blueChannel[i];
      }

      r = r / clusterNeighbourhood.length();
      g = g / clusterNeighbourhood.length();
      b = b / clusterNeighbourhood.length();

      cluster.setRgbF(r,g,b,a);
      if(oldCluster.has_value()){
          if(getDistance(oldCluster.value(), cluster) < epsilon){
              needsRecalculating = false;
          }
      }
    }

bool Cluster::doesNeedRecalculating() const {
    return needsRecalculating;
}

size_t Cluster::size() const {
    return clusterNeighbourhood.length();
}

Cluster::~Cluster(){};



