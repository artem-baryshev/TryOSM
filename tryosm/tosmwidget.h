#ifndef TOSMWIDGET_H
#define TOSMWIDGET_H

#include <QWidget>
#include <QSqlDatabase>
#include <QPainter>
#include <QList>
#include <QLinkedList>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QPointF>
#include <QDebug>
#include <QRunnable>
#include <QReadWriteLock>
#include "tstat.h"
#include "tmouseman.h"

const long long BAD_TID = -1;
const double W_INF = 1e100;

class TOSMWidget : public QWidget
{

    Q_OBJECT

    QImage Image;

    class TRender : public QRunnable
    {
        void run();
    };

public:

    QReadWriteLock datalocker, imagelocker;
    typedef long long TID;
    typedef QSet <TID> TIDs;
    typedef QList <TID> TIDList;
    typedef double TWeight;


//protected:

    double w2s_x(double coord);
    double w2s_y(double coord);
    QPointF w2s_p(QPointF p);
    double s2w_x(double coord);
    double s2w_y(double coord);
    QPointF s2w_p(QPointF p);

    enum EDirection
    {
        Both,
        Forward,
        Reverce
    };

    struct TWayWeight
    {
        TWeight forward, reverse;
        TWeight get(EDirection Dir = Both, bool minOfBoth = true);
        TWayWeight(TWeight f = 0.0, TWeight r = 0.0);
    };

    struct TWayFlag
    {
        bool forward, reverce;
        bool get(EDirection Dir = Both, bool andBoth = true);
        TWayFlag(bool f = true, bool r = true);
    };

    QString dbFileName;

    TMouseMan *mouse;

    double maxWayAttr;
    double midWayAttr;

    struct TNNode
    {
//        long usage;
        double lat, lon, metrica;
//        double attr, usefullness, matter;
        TIDs ownedBy;
//        double orthodrom(TNNode &nd);
        QPointF toPointF();
        TNNode(double Lat = 0, double Lon = 0);
        bool isKnot()
        {
            return (ownedBy.size() > 1);
        }
    };

    double distance(TNNode n1, TNNode n2);
    double distance(TID n1, TID n2);
    double distance(QPointF n1, QPointF n2);

    struct TNWay
    {
        enum EWayClass
        {
            EW_Unclassified,
            EW_Primary,
            EW_Secondary,
            EW_Motorway,
            EW_Trunk,
            EW_Tertiary,
            EW_LivingStreet,
            EW_Pedestrian,
            EW_Residental,
            EW_Service,
            EW_Track,
            EW_BusGuideway,
            EW_Raceway,
            EW_Road,
            EW_Path,
            EW_Footway,
            EW_Bridleway,
            EW_Steps,
            EW_Cycleway,
            EW_Proposed,
            EW_Construction,
            EW_Escape
        };
        enum EOneway
        {
            OW_no,
            OW_yes_forward,
            OW_yes_reverce
        };

        class exception
        {
        public:
            exception(QString S){qDebug() << S;}
        };
        EWayClass roadClass;
        EOneway oneWay;
        bool isLink;

        QMap <TID, QMap <TID, TWeight> > lengthes;

        QList <TID> nodes;
        QList <TID> coUsedNodes;
//        QMap <TID, QVector <TNeightbourInfo> > neighbours;
        int getNodeIndex(TID node);
        QString toString();
        TWeight getLength();
        TWeight getLength(TID fromNode);
        TWeight getLength(TID toNode, EDirection Direction);
        TWeight getLength(TID fromNode, TID toNode);
        TNWay(const TNWay &src);
        TNWay(TOSMWidget * Owner = NULL);
        TNWay & operator = (const TNWay & src);
        TID getOtherEnd(TID);
        EDirection getDirectionFrom(TID);
        ~TNWay();
    private:
        TOSMWidget * owner;
        TWeight *length;
//        TWeight *weight;
        void incopy(const TNWay &src);
        void init();
    };



//    struct TWay
//    {
//        QList <QPointF> nodes;
//    };

//    QList <TNWay> MyWays;
//    QList <TNNode> MyNodes;
public:
//    TIDs MyCrosses;


    typedef QMap <TID, TNWay> TNWays;
    typedef QMap <TID, TNNode> TNNodes;
    typedef TNWays::Iterator TNWaysIter;
    typedef TNNodes::Iterator TNNodesIter;


    TNWays nways;
    TNNodes nnodes;


//    QList <TWay> ways;
//    QMap <TID, TID> nodesToMyNodes;
//    void loadWays();
//    QString dbFileName;
    void loadNData(QString);
    void drawLine(QPainter &painter, QLineF line);
    bool mouseDragPressed;
    QPointF mouseDragPos;
//    QList <TID> path;
    TStatistic stWA;
    QList <QPointF> markers;
    TID nearestNode(QPointF);

//    QList <TID> findPath(TID nodeIdFrom, TID nodeIdTo);
//    QList <TID> findPathBrute(TID nodeFrom, TID nodeTo);
//    QList <TID> * findPathBrute_sub(TID nodeFrom, QSet<TID> nodesTo, QList <TID> *curPath = NULL, QSet <TID> *curPathNodes = NULL);
//    TIDs getNeighbours(TID node);
public:
    bool useMetric;
    struct TRoute
    {
        QList <TID> nodes;
        TWeight Length;
    };
    class TRouteProfile
    {
    public:
        virtual TWeight getWayWeight(
                                     TNWay *way,
                                     TID nodeFrom,
                                     TID nodeTo
                                     ) = 0;
        virtual bool isPassable(
                                     TNWay *way,
                                     TID nodeFrom,
                                     TID nodeTo
                                     ) = 0;
        virtual TWeight getNodeWeight(TNNode *node, TID wayFrom, TID wayTo) = 0;
    };

    TRouteProfile * routeProfile;

    class TRoutingEngine
    {
    protected:
        TOSMWidget * owner;
//        TNWays * Ways() {return &owner->nways;}
//        TNNodes * Nodes() {return &owner->nnodes;}
    public:
        TRoutingEngine(TOSMWidget * Owner) {owner = Owner;}
        virtual TRoute findPath(TID nodeIdFrom, TID nodeIdTo, TRouteProfile&) = 0;
    };

    friend class TRoutingEngine;
    double drawProp;
//    explicit TOSMWidget(QWidget *parent = 0);
    void fillUsage();
    explicit TOSMWidget(QString DbFileName, QWidget *parent);
    QRectF Rect;
    TRoute path;
    ~TOSMWidget();
signals:
    
public slots:
    void mouseDoubleClickEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
//    void resizeEvent(QResizeEvent *);
};



#endif // TOSMWIDGET_H
