#ifndef TDIJKSTRAROUTE_H
#define TDIJKSTRAROUTE_H

#include "tosmwidget.h"



class TRouteDijkstra : public TOSMWidget::TRoutingEngine
{
    typedef QMap <TOSMWidget::TID, TOSMWidget::TWeight> TDistanceMap;
    typedef std::multimap <TOSMWidget::TWeight, TOSMWidget::TID> TSortedDistances;
    typedef TOSMWidget::TID TID;
    typedef TOSMWidget::TIDs TIDs;
    typedef TOSMWidget::TRouteProfile TRouteProfile;
    typedef TOSMWidget::TWeight TWeight;
    struct TKnotNeighbour
    {
        TOSMWidget::TID knot;
        TOSMWidget::TID way;
        TKnotNeighbour(TOSMWidget::TID, TOSMWidget::TID Way = BAD_TID);
    };
    QMap <TID, QList <TKnotNeighbour> > knotsNear;
    QSet <TID> knots;
    void nearKnots(TID node1, TID node2, TID way);
    void initSearch(TID nodeId, TRouteProfile &profile, TDistanceMap &distances, TSortedDistances &D, TID dest = BAD_TID);
    TID getNextKnot(TSortedDistances &D, TIDs &U);
    void updateDistances(TID node, TDistanceMap &distances, TSortedDistances &D, TOSMWidget::TRouteProfile &profile, bool reverce = false, TID dest = BAD_TID, TID source = BAD_TID);
    void buildRoute(TOSMWidget::TRoute &route, TID start, TDistanceMap &distances, TOSMWidget::TRouteProfile &profile, bool reverce = false);
public:
    TRouteDijkstra(TOSMWidget * Owner);
    TOSMWidget::TRoute findPath(TID nodeIdFrom, TID nodeIdTo, TRouteProfile &profile);
    TOSMWidget::TRoute findPath_DDijkstra(TID nodeIdFrom, TID nodeIdTo, TRouteProfile &profile);
    TOSMWidget::TRoute findPath_AStar(TID nodeIdFrom, TID nodeIdTo, TRouteProfile &profile);
};

#endif // TDIJKSTRAROUTE_H
