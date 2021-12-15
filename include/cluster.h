#ifndef CLUSTER_H
#define CLUSTER_H

#include <QColor>

class Cluster {
    public:
        explicit Cluster(QColor cluster, qreal epsilon);
        Cluster();
        QColor getCluster() const;
        qreal getEpsilon() const;
        size_t size() const;
        bool doesNeedRecalculating() const;

        void setCluster(QColor cluster);
        void setEpsilon(qreal epsilon);

        void appendNeighbour(QColor color);

        void removeNeighbour(QColor color);

        void recalculateCluster();
        friend inline bool operator < (const Cluster& first, const Cluster& second){return first.size() < second.size();}

       ~Cluster();

    private:
       bool needsRecalculating;
       qreal epsilon;
       std::optional<QColor> oldCluster;
       QColor cluster;
       QVector<QColor> clusterNeighbourhood;
};

#endif // CLUSTER_H
