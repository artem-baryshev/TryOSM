#include "tosmwidget.h"
#include <QPainter>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QSqlError>
#include <math.h>
#include <map>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTime>
#include "tstat.h"
#include "troutedijkstra.h"

//#include <deque>

//TOSMWidget::TOSMWidget(QWidget *parent) :
//    QWidget(parent)
//{
//}

const double DEG_TO_RAD = M_PI / 180.0;
//const double W_INF = 1e100;

TOSMWidget::TOSMWidget(QString DbFileName, QWidget *parent) : QWidget(parent)
{
    Rect = QRectF(55, 57, 62, 66);
//    loadWays();

    dbFileName = DbFileName;
    loadNData(DbFileName);
//    fillUsage();

    mouse = new TMouseMan(this);
    routeProfile = NULL;
    mouseDragPressed = false;
}

TOSMWidget::TWayWeight::TWayWeight(TWeight f, TWeight r)
{
    forward = f;
    reverse = r;
}

TOSMWidget::TWayFlag::TWayFlag(bool f, bool r)
{
    forward = f;
    reverce = r;
}

void TOSMWidget::TNWay::init()
{
    length = NULL;
    road_class = EW_Unclassified;
    is_link = false;
    one_way = OW_no;
    is_area = false;
//    attr = TOSMWidget::TWayWeight(0, 0);
//    usage = TOSMWidget::TWayWeight(0, 0);
//    passable = TOSMWidget::TWayFlag(true, true);
//    checked = false;
}

TOSMWidget::TNWay::TNWay(TOSMWidget *Owner)
{
    init();
    owner = Owner;
}

TOSMWidget::TWeight TOSMWidget::TWayWeight::get(EDirection Dir, bool minOfBoth)
{
    switch (Dir)
    {
    case Both:
        return (minOfBoth ? std::min(forward, reverse) : std::max(forward, reverse));
    case Forward:
        return forward;
    case Reverce:
        return reverse;
    }
    return W_INF;
}

bool TOSMWidget::TWayFlag::get(EDirection Dir, bool andBoth)
{
    switch (Dir)
    {
    case Both:
        return (andBoth ? (forward && reverce) : (forward || reverce));
    case Forward:
        return forward;
    case Reverce:
        return reverce;
    }
    return false;
}

TOSMWidget::TNWay::TNWay(const TNWay &src)
{
    init();
    incopy(src);
}

TOSMWidget::TNWay & TOSMWidget::TNWay::operator = (const TOSMWidget::TNWay &src)
{
    incopy(src);
    return *this;
}

TOSMWidget::TNWay::~TNWay()
{
    if (length) delete length;
}

void TOSMWidget::TNWay::incopy(const TNWay &src)
{
    owner = src.owner;
    if (length) delete length;
    if (src.length)
    {
        length = new double;
        (*length) = (*(src.length));
    }
    else
    {
        length = NULL;
    }
    nodes = src.nodes;
    coUsedNodes = src.coUsedNodes;
}

TOSMWidget::TWeight TOSMWidget::TNWay::getLength()
{
    if (!owner) return 0.0;
//    if (!passable.get(Direction, true)) return W_INF;
    if (!length)
    {
        length = new double;
        (*length) = 0.0;
        for (int i = 1; i < nodes.size(); i++)
        {
            (*length) += owner->distance(nodes[i-1], nodes[i]);
        }
        (*length) *= 1.0;
    }
    return *length;
}

TOSMWidget::TWeight TOSMWidget::TNWay::getLength(TOSMWidget::TID fromNode)
{
    if (fromNode == nodes.first())
        return getLength(Forward);
    if (fromNode == nodes.last())
        return getLength(Reverce);
    throw(exception("!!!TOSMWidget::TNWay::Weight() exception: bad node ID to get way`s weight"));
}

int TOSMWidget::TNWay::getNodeIndex(TID node)
{
    for (int i = 0; i < nodes.size(); i++)
    {
        if (node == nodes[i]) return i;
    }
    return BAD_TID;
}

//double TOSMWidget::TNNode::orthodrom(TOSMWidget::TNNode &n)
//{
//    return 6371 * acos(sin(n.lat * DEG_TO_RAD) * sin(lat * DEG_TO_RAD) + cos(n.lat * DEG_TO_RAD) * cos(lat * DEG_TO_RAD) * cos((lon - n.lon) * DEG_TO_RAD));
//}

double TOSMWidget::distance(QPointF n1, QPointF n2)
{
    return 6371 * acos(sin(n1.y() * DEG_TO_RAD) * sin(n2.y() * DEG_TO_RAD) + cos(n1.y() * DEG_TO_RAD) * cos(n2.y() * DEG_TO_RAD) * cos((n1.x() - n2.x()) * DEG_TO_RAD));
}

double TOSMWidget::w2s_x(double coord)
{
    return (coord - Rect.left()) / Rect.width() * this->width();
}

double TOSMWidget::w2s_y(double coord)
{
    return this->height() - (coord - Rect.top()) / Rect.height() * this->height();
}

double TOSMWidget::s2w_x(double coord)
{
    return coord / this->width() * Rect.width() + Rect.left();
}

double TOSMWidget::s2w_y(double coord)
{
    return (this->height() - coord) * Rect.height() / this->height() + Rect.top();
}

TOSMWidget::~TOSMWidget()
{
//    qDebug() << "~TOSMWidget() {";
//    db.close();
//    qDebug() << "}";
}

void TOSMWidget::drawLine(QPainter &painter, QLineF line)
{
    painter.drawLine(w2s_x(line.x1()), w2s_y(line.y1()), w2s_x(line.x2()), w2s_y(line.y2()));
}

QString TOSMWidget::TNWay::toString()
{
    QString s;
    for (QList <TID> ::Iterator it = nodes.begin(); it != nodes.end(); it++)
        s+= " " + (*it);
    return s;
}

QPointF TOSMWidget::w2s_p(QPointF p)
{
    return QPointF(w2s_x(p.x()), w2s_y(p.y()));
}

QPointF TOSMWidget::s2w_p(QPointF p)
{
    return QPointF(s2w_x(p.x()), s2w_y(p.y()));
}

void TOSMWidget::loadNData(QString DbFileName)
{
//    TIDs multiusedNodes;

    {
        QSqlDatabase db;
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(DbFileName);
        if (!db.open())
        {
            qDebug() << "!!! Failed to open database !!!";
            exit(0);
        }
        QSqlQuery q(db);
        qDebug() << QTime::currentTime().toString() << " " << "requesting ways...";
        if (!q.exec("select "
                    "w.id way, "
                    "t.value tag, "
                    "tv.value value "
                    "from "
                    "t_ways w "
                    "inner join t_ways_tags wt on w.id = wt.way "
                    "inner join t_tags t on wt.tag = t.id and (t.value in ('highway', 'oneway', 'junction')) "
                    "inner join t_tags_values tv on wt.value = tv.id"
                    ))
        {
            qDebug() << "!!! failed to recieve ways!!" << q.lastError().text();
            return;
        }
        qDebug() << QTime::currentTime().toString() << "receiving ways...";
        while (q.next())
        {
            TID w = q.value(q.record().indexOf("way")).toLongLong();
            QString t = q.value(q.record().indexOf("tag")).toString();
            QString v = q.value(q.record().indexOf("value")).toString();
            if (!nways.contains(w)) nways.insert(w, TNWay(this));
            TNWay * way = &(nways[w]);

            if (v == "motorway") { way->setRoadClass(TNWay::EW_Motorway); }
            if (v == "motorway_link") { way->setRoadClass(TNWay::EW_Motorway); way->setIsLink(true);}
            if (v == "trunk") { way->setRoadClass(TNWay::EW_Trunk); }
            if (v == "trunk_link") { way->setRoadClass(TNWay::EW_Trunk); way->setIsLink(true);}
            if (v == "primary") { way->setRoadClass(TNWay::EW_Primary); }
            if (v == "primary_link") { way->setRoadClass(TNWay::EW_Primary); way->setIsLink(true);}
            if (v == "secondary") { way->setRoadClass(TNWay::EW_Secondary); }
            if (v == "secondary_link") { way->setRoadClass(TNWay::EW_Secondary); way->setIsLink(true);}
            if (v == "tertiary") { way->setRoadClass(TNWay::EW_Tertiary); }
            if (v == "tertiary_link") { way->setRoadClass(TNWay::EW_Tertiary); way->setIsLink(true);}
            if (v == "living_street") { way->setRoadClass(TNWay::EW_LivingStreet); }
            if (v == "pedestrian") { way->setRoadClass(TNWay::EW_Pedestrian); }
            if (v == "residential") { way->setRoadClass(TNWay::EW_Residental); }
            if (v == "unclassified") { way->setRoadClass(TNWay::EW_Unclassified); }
            if (v == "service") { way->setRoadClass(TNWay::EW_Service); }
            if (v == "track") { way->setRoadClass(TNWay::EW_Track); }
            if (v == "bus_guideway") { way->setRoadClass(TNWay::EW_BusGuideway); }
            if (v == "raceway") { way->setRoadClass(TNWay::EW_Raceway); }
            if (v == "road") { way->setRoadClass(TNWay::EW_Road); }
            if (v == "path") { way->setRoadClass(TNWay::EW_Path); }
            if (v == "footway") { way->setRoadClass(TNWay::EW_Footway); }
            if (v == "bridleway") { way->setRoadClass(TNWay::EW_Bridleway); }
            if (v == "steps") { way->setRoadClass(TNWay::EW_Steps); }
            if (v == "cycleway") { way->setRoadClass(TNWay::EW_Cycleway); }
            if (v == "proposed") { way->setRoadClass(TNWay::EW_Proposed); }
            if (v == "construction") { way->setRoadClass(TNWay::EW_Construction); }
            if (v == "escape") { way->setRoadClass(TNWay::EW_Escape); }
            if (t == "oneway")
            {
                if ((v == "yes") || (v == "true") || (v == "true"))
                {
                    way->setOneWay(TNWay::OW_yes_forward);
                }
                if ((v == "-1") || (v == "reverse"))
                {
                    way->setOneWay(TNWay::OW_yes_reverce);
                }
                if (v == "no")
                {
                    way->setOneWay(TNWay::OW_no);
                }
            }
            if (t == "area")
            {
                if ((v == "yes") || (v == "true") || (v == "true"))
                {
                    way->setIsArea(true);
                }
                if ((v == "-1") || (v == "reverse"))
                {
                    way->setIsArea(true);
                }
                if (v == "no")
                {
                    way->setIsArea(false);
                }
            }
        }

        qDebug() << QTime::currentTime().toString() << "requesting nodes...";

        if (!q.exec("select "
                       "w.id way, "
                       "n.lat lat, "
                       "n.lon lon, "
                       "n.id node "
                   "from "
                       "t_ways w "
                       "inner join t_ways_tags wt on w.id = wt.way "
                       "inner join t_tags t on wt.tag = t.id and (t.value = 'highway') "
                       "inner join t_ways_nodes wn on w.id = wn.way "
                       "inner join t_nodes n on n.id = wn.node "
                       "inner join t_tags_values tv on wt.value = tv.id "
//                   "where "
//                    "tv.value not in ("
//                                    "'pedestrian',"
//                                    "'footway',"
//                                    "'path',"
//                                    "'steps',"
//                                    "'construction',"
//                                    "'cycleway',"
//                                    "'proposed',"
//                                    "'platform',"
//                                    "'bridleway',"
//                                    "'piste'"
//                                ") "
//                    "tv.value in ("
    //                "'residental',"
    //                "'service',"
    //                "'living_street',"
    //                "'road',"
//                                    "'motorway',"
//                                    "'motorway_link',"
//                                    "'trunk', "
//                                    "'trunk_link',"
//                                    "'primary',"
//                                    "'primary_link',"
//                                    "'secondary',"
//                                    "'secondary_link',"
//                                    "'tertiary_link',"
//                                    "'tertiary'"
//                                ") "

    //                   "and n.lat > 61.1 and n.lat < 61.2 and n.lon > 62.75 "
                   "order by 1"))

        {
            qDebug() << "!!! Failed to recieve nodes !!! " << q.lastError().driverText() ;
            return;
        }
        bool first = true;
        qDebug() << QTime::currentTime().toString() << " " << "receiving nodes...";
        while (q.next())
        {
    //        qDebug() << "q.next()";
            TID w = q.value(q.record().indexOf("way")).toLongLong();
            TID n = q.value(q.record().indexOf("node")).toLongLong();
            double lat = q.value(q.record().indexOf("lat")).toDouble();
            double lon = q.value(q.record().indexOf("lon")).toDouble();
            QPointF point(lon, lat);
            if (first)
            {
                first = false;
                Rect.setLeft(point.x());
                Rect.setRight(point.x());
                Rect.setTop(point.y());
                Rect.setBottom(point.y());

            }
            else
            {
    //            qDebug() << point;
                Rect.setLeft(std::min(Rect.left(), point.x()));
                Rect.setRight(std::max(Rect.right(), point.x()));
                Rect.setTop(std::min(Rect.top(), point.y()));
                Rect.setBottom(std::max(Rect.bottom(), point.y()));
            }
            if (!nways.contains(w)) nways.insert(w, TNWay(this));
            nways[w].nodes.append(n);
            if (!nnodes.contains(n))
            {
                TNNode NNode(lat, lon);
                nnodes.insert(n, NNode);
            }
            nnodes[n].lat = lat;
            nnodes[n].lon = lon;
            nnodes[n].ownedBy.insert(w);
//            if (nnodes[n].containedBy.size() > 1)
//            {
//                MyCrosses.insert(n);
//            }
        }
        db.close();
    }

//    for (TIDs::Iterator it = MyCrosses.begin(); it != MyCrosses.end(); it++)
//    {
//        for (TIDs::Iterator it_w = nnodes[*it].containedBy.begin(); it_w != nnodes[*it].containedBy.end(); it_w++)
//        {
//            nways[*it_w].
//            nways[*it_w]. (*it);
//        }
//    }



//    Rect.setBottom((Rect.top() + Rect.bottom()) / 2);
//    qDebug() << "rect " << Rect;

//    TIDs checkedNodes;
////    QMap <TID, TID> nodesToMyNodes;

//    qDebug() << QTime::currentTime().toString() << " " << "converting data...";

//    for (TIDsIter it_n = multiusedNodes.begin(); it_n != multiusedNodes.end(); it_n++)
//    {
//        TID nodeId = (*it_n);
//        TNNode *node = &nnodes[nodeId];
//        if (node->containedBy.size() > 1)
//        {
//            TID newMyNodeId = 0;
//            if (!nodesToMyNodes.contains(nodeId))
//            {
//                newMyNodeId = MyNodes.size();
//                nodesToMyNodes.insert(nodeId, newMyNodeId);
//                TNNode newMyNode(node->lat, node->lon);
//                MyNodes.push_back(newMyNode);

//            }
//            else
//            {
//                newMyNodeId = nodesToMyNodes[nodeId];
//            }
////            MyNodes.append(TNNode(node->lat, node->lon));
////            qDebug() << "!!!  NEW multinode (" << node->containedBy.size() << ") " << newMyNodeId;

//            for (TIDsIter it_w = node->containedBy.begin(); it_w != node->containedBy.end(); it_w++)
//            {
//                TNWay *way = &nways[*it_w];
////                qDebug() << " ! next way " << way->nodes << " we need " << nodeId;
////                qDebug() << "<<<";
//                int inode = way->getNodeIndex(nodeId);
////                QList <TID> newWayNodes;
//                bool addThisWay = true;
//                TNWay newMyWay(this);
//                newMyWay.nodes.append(newMyNodeId);
//                for (int i = inode - 1; i >= 0; i--)
//                {
//                    TNNode *waynode = &nnodes[way->nodes[i]];
//                    if (!nodesToMyNodes.contains(way->nodes[i]))
//                    {
//                        nodesToMyNodes.insert(way->nodes[i], MyNodes.size());
//                        TNNode newMyNode(waynode->lat, waynode->lon);
////                        qDebug() << newMyNode.lat << ", " << newMyNode.lon;
//                        MyNodes.push_back(newMyNode);
//                    }
//                    newMyWay.nodes.push_back(nodesToMyNodes[way->nodes[i]]);
//                    if (checkedNodes.contains(way->nodes[i]))
//                    {
//                        addThisWay = false;
////                        qDebug() << "already checked " << nodesToMyNodes[way->nodes[i]];
//                        break;
//                    }
//                    if (waynode->containedBy.size() > 1) break;
//                }
//                if (addThisWay && (newMyWay.nodes.size() > 1))
//                {
//                    MyWays.push_back(newMyWay);
////                    qDebug() << "++ way " << newMyWay.nodes;
//                }
//                else
//                {
////                    qDebug() << "-- way " << newMyWay.nodes << " " << (!addThisWay?"checked":"");
//                }

////                qDebug() << ">>>";
//                addThisWay = true;
//                newMyWay.nodes.clear();
//                newMyWay.nodes.append(newMyNodeId);
//                for (int i = inode + 1; i < way->nodes.size(); i++)
//                {
//                    TNNode *waynode = &nnodes[way->nodes[i]];
//                    if (!nodesToMyNodes.contains(way->nodes[i]))
//                    {
//                        nodesToMyNodes.insert(way->nodes[i], MyNodes.size());
//                        TNNode newMyNode(waynode->lat, waynode->lon);
////                        qDebug() << newMyNode.lat << ", " << newMyNode.lon;
//                        MyNodes.push_back(newMyNode);
//                    }
//                    newMyWay.nodes.push_back(nodesToMyNodes[way->nodes[i]]);
//                    if (checkedNodes.contains(way->nodes[i]))
//                    {
//                        addThisWay = false;
////                        qDebug() << "already checked " << nodesToMyNodes[way->nodes[i]];
//                        break;
//                    }
//                    if (waynode->containedBy.size() > 1) break;
//                }
//                if (addThisWay && (newMyWay.nodes.size() > 1))
//                {
//                    MyWays.push_back(newMyWay);
////                    qDebug() << "++ way " << newMyWay.nodes;
//                }
//                else
//                {
////                    qDebug() << "-- way " << newMyWay.nodes << " " << (!addThisWay?"checked":"");
//                }
//            }
//            checkedNodes.insert(nodeId);
//        }

//    }

//    for (TID i = 0; i < MyWays.size(); i++)
////    for (QList <TNWay>::iterator it_w = MyWays.begin(); it_w != MyWays.end(); it_w++)
//    {
//        TNWay *way = &MyWays[i];
//        for (QList <TID> ::Iterator it = way->nodes.begin(); it != way->nodes.end(); it++)
//        {
//            MyNodes[*it].containedBy.insert(i);
//        }
////        MyNodes[way->nodes.first()].containedBy.insert(i);
////        MyNodes[way->nodes.last()].containedBy.insert(i);
//        MyCrosses.insert(way->nodes.first());
//        MyCrosses.insert(way->nodes.last());
//    }



}

TOSMWidget::TID TOSMWidget::nearestNode(QPointF p)
{
    TID best = 0;
    TWeight bestd = W_INF;
    for (TNNodes::Iterator it = nnodes.begin(); it != nnodes.end(); it++)
    {

        TNNode *node = &it.value();
//                &(nnodes[i]);
//        nnodes[i];
        TWeight d = distance(node->toPointF(), p);
        if (d < bestd)
        {
            bestd = d;
            best = it.key();
        }
    }
    return best;
}

QPointF TOSMWidget::TNNode::toPointF()
{
    return QPointF(lon, lat);
}

double TOSMWidget::distance(TNNode n1, TNNode n2)
{
    return distance(n1.toPointF(), n2.toPointF());
}

double TOSMWidget::distance(TID n1, TID n2)
{
    return distance(nnodes[n1], nnodes[n2]);
}

void TOSMWidget::paintEvent(QPaintEvent *)
{
    qDebug() << "Paint event";
    QImage newImage(QSize(rect().width(), rect().height()), QImage::Format_ARGB32);
    QPainter Painter(&newImage);
//    Painter.set
//    QPaintDevice t;

//    Painter.begin(&newImage);


    Painter.setPen(QPen(QColor(0, 0, 0)));

    midWayAttr = stWA.Median(drawProp);

    long long i = 0;
    for (TNWaysIter it = nways.begin(); it != nways.end(); it++)
    {
        TNWay *way = &it.value();
//        qDebug() << "F " << way->attrF << " R" << way->attrR << " / " << maxWayAttr;
//        bool deadEndF = MyNodes[way->nodes.last()].containedBy.size() == 1, deadEndR = MyNodes[way->nodes.first()].containedBy.size() == 1;



//        double v = way->Usage();

//        if ((way->usageF > 0) && (way->usageR > 0))
//        {
//            Painter.setPen(QPen(QColor(0, 255, 0)));
//            Painter.drawEllipse(w2s_p(MyNodes[way->nodes.first()].toPointF()), 25, 25);
//        }
//        else

//        if (v > midWayAttr)
//        {
//            Painter.setPen(QPen(QColor(255, 0, 0)));
//        }
//        else
//        {
//            Painter.setPen(QPen(QColor(0, 0, 255)));
//        }

//        / maxWayAttr;
//        Painter.setPen(QPen(QColor(long(v * 255) % 256, 0, 255 - long(v * 255) % 256)));

        bool first = true;
        QPointF prevP;
//        double dist1 = MyNodes[(*it).nodes.first()].attr,
//                dist2 = MyNodes[(*it).nodes.last()].attr,
//                wlen = (*it).Weight(),
//                wdist = 0.0;
        i++;
//        Painter.setPen(QPen(QColor((i*i)%200, ((i+11)*(i+11))%200, ((i+19)*(i+19))%200)));
        switch (way->roadClass())
        {
            case TOSMWidget::TNWay::EW_Trunk:
            case TOSMWidget::TNWay::EW_Motorway:
            case TOSMWidget::TNWay::EW_Raceway:
            case TOSMWidget::TNWay::EW_Primary: Painter.setPen(QPen(QColor(63, 200, 63))); break;
            case TOSMWidget::TNWay::EW_Tertiary:
            case TOSMWidget::TNWay::EW_Road:
            case TOSMWidget::TNWay::EW_Unclassified:
            case TOSMWidget::TNWay::EW_Secondary: Painter.setPen(QPen(QColor(100, 150, 63))); break;
            case TOSMWidget::TNWay::EW_LivingStreet:
            case TOSMWidget::TNWay::EW_Service: Painter.setPen(QPen(QColor(130, 100, 63))); break;
            case TOSMWidget::TNWay::EW_Residental: Painter.setPen(QPen(QColor(170, 90, 63))); break;
            case TOSMWidget::TNWay::EW_BusGuideway:
            case TOSMWidget::TNWay::EW_Track: Painter.setPen(QPen(QColor(200, 60, 63))); break;
            case TOSMWidget::TNWay::EW_Path:
            case TOSMWidget::TNWay::EW_Footway:
            case TOSMWidget::TNWay::EW_Steps:
            case TOSMWidget::TNWay::EW_Bridleway:
            case TOSMWidget::TNWay::EW_Construction:
            case TOSMWidget::TNWay::EW_Proposed:
            case TOSMWidget::TNWay::EW_Pedestrian: Painter.setPen(QPen(QColor(200, 200, 200))); break;
        default:
            Painter.setPen(QPen(QColor(0, 0, 0)));
        }

//        Painter.setPen(QPen(QColor((i*i)%200, ((i+11)*(i+11))%200, ((i+19)*(i+19))%200)));
        for (QList <TID> ::Iterator it_n = way->nodes.begin(); it_n != way->nodes.end(); it_n++)
        {
//            double dist = std::min(dist1 + wdist, dist2 + wlen - wdist);
//            Painter.setPen(QPen(QColor(long((dist*256/10)) % 256, 128, 128)));
            QPointF curP = nnodes[*it_n].toPointF();
            if ((nnodes[*it_n].metrica > 0.0) && (nnodes[*it_n].metrica < W_INF))
            {
                Painter.drawText(w2s_p(curP),QString::number(nnodes[*it_n].metrica));
            }
            if (!first)
            {
                drawLine(Painter, QLineF(prevP, curP));
//                wdist += fabs(distance(curP, prevP));
            }
            first = false;
            prevP = curP;
        }
    }

//    Painter.setPen(QPen(QColor(0, 0, 0)));
//    Painter.drawEllipse(w2s_p(MyNodes[*MyCrosses.begin()].toPointF()), 5, 5);

    Painter.setPen(QPen(QColor(0, 200, 0)));

    for (int i = 0; i < markers.size(); i++)
    {
//        qDebug() << "drawing marker " << markers[i];
//        QRect pieRect = QRect(
//                            w2s_x(markers[i].x())-20,
//                            w2s_y(markers[i].y())-20,
//                            w2s_x(markers[i].x())+20,
//                            w2s_y(markers[i].y())+20);
//        Painter.drawPie(pieRect, (90-10)*16, (90+10)*16);
        Painter.drawEllipse(w2s_p(markers[i]), 15, 15);
        Painter.drawEllipse(w2s_p(nnodes[nearestNode(markers[i])].toPointF()), 10, 10);
    }

    Painter.setPen(QPen(QColor(0, 255, 0)));
    QPointF p;
    bool first = true;
    for (QList <TID> ::Iterator it = path.nodes.begin(); it != path.nodes.end(); it++)
    {
        Painter.drawEllipse(w2s_p(nnodes[*it].toPointF()), 5, 5);
        first = false;
    }

//    for (QList <TWay> ::Iterator it = MyWays.begin(); it != MyWays.end(); it++)
//    {
//        TNWay *way = &(*it);
//        for (QList <TID> ::Iterator itn = way->nodes.begin(); itn != way->nodes.end(); itn++)
//        {

//        }
//    }

//    this->
//    Painter.drawImage();
//    Painter.end();

    QPainter newPainter(this);
    qDebug() << "rect " << this->rect();
    newPainter.drawRect(this->rect());
//    qDebug() << "rect " << this->rect();
//    newPainter.fillRect(this->Rect, QBrush(Qt::white));
    newPainter.drawImage(QPoint(0,0), newImage);

}

void TOSMWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        mouseDragPressed = false;
    }
//    else if (e->button() == Qt::RightButton)
//    {
////        if (e->)
//    }

}



void TOSMWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        markers.append(s2w_p(e->posF()));
        while (markers.size() > 2)
        {
            markers.removeAt(0);
        }
        if (markers.size() == 2)
        {
            path.nodes.clear();

//            path.append(nearestNode(markers[0]));
//            path.append(nearestNode(markers[1]));
            TRouteDijkstra route(this);
            if (!routeProfile)
            {
                qDebug() << "!!!routing profile not set";
                return;
            }
//            TPedestrianProfile pp;
            qDebug() << QTime::currentTime().toString() << "path find start";
            path = route.findPath(nearestNode(markers[0]), nearestNode(markers[1]), *routeProfile);
            qDebug() << QTime::currentTime().toString() << "path find end";
//            path =

//            path = findPath(nearestNode(markers[0]), nearestNode(markers[1]));

        }
//        qDebug() << "markers: " << markers;
        update();
    }
}

void TOSMWidget::mousePressEvent(QMouseEvent *e)
{
//    mouse
    if (e->button() == Qt::LeftButton)
    {
        mouseDragPressed = true;
        mouseDragPos = s2w_p(e->posF());
    }
}

void TOSMWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (mouseDragPressed)
    {
        QPointF newMousePos = s2w_p(e->posF());

        Rect.moveTo(Rect.left() + (mouseDragPos.x() - newMousePos.x()), Rect.top() + (mouseDragPos.y() - newMousePos.y()));
        update();
    }
}

void TOSMWidget::wheelEvent(QWheelEvent *e)
{
//    qDebug() << "delta " << e->delta();
    double delta = e->delta(), scale = 1, step = 0.9;
    if (delta > 0)
    {
        while (delta > 0)
        {
            delta -= 120;
            scale *= step;
        }
    }
    else
    {
        while(delta < 0)
        {
            delta += 120;
            scale /= step;
        }
    }
    QPointF clickPoint((double)e->pos().x() / width() * Rect.width() + Rect.left(), (double)(height() - e->pos().y()) / height() * Rect.height() + Rect.top());
    QPointF newSizes(Rect.width() * scale, Rect.height() * scale);
    Rect = QRectF(clickPoint.x() - newSizes.x() * ((double)e->pos().x() / width()), clickPoint.y() - newSizes.y() * ((double)(height() - e->pos().y()) / height()), newSizes.x(), newSizes.y());
    update();
}

TOSMWidget::TNNode::TNNode(double Lat, double Lon)
{
     lat = Lat;
     lon = Lon;
     metrica = 0.0;
//     attr = W_INF;
}



TOSMWidget::TWeight TOSMWidget::TNWay::getLength(TID fromNode, TID toNode)
{
    if (lengthes.contains(fromNode))
    {
        if (lengthes[fromNode].contains(toNode))
        {
            return lengthes[fromNode][toNode];
        }
    }
    else
    {
        lengthes.insert(fromNode, QMap <TID, TWeight> ());
    }
    if (lengthes.contains(toNode))
    {
        if (lengthes[toNode].contains(fromNode))
        {
            return lengthes[toNode][fromNode];
        }
    }
    TWeight w = 0;
    EDirection d = Both;
    TWayFlag calc(false, false);
    if (fromNode == toNode) return 0.0;
    bool has1 = false, has2 = false;
    if (isArea())
    {
        for (QList <TID>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it) == fromNode)
            {
                has1 = true;
            }
            if ((*it) == toNode)
            {
                has2 = true;
            }
        }
        w = owner->distance(fromNode, toNode);
    }
    else
    {
        for (int i = 0; i < nodes.size(); i++)
        {
            if (nodes[i] == fromNode)
            {
                has1 = true;
                calc.forward = true;
                if (!calc.reverce)
                {
                    d = Forward;
                    continue;
                }
            }
            if (nodes[i] == toNode)
            {
                has2 = true;
                calc.reverce = true;
                if (!calc.forward)
                {
                    d = Reverce;
                    continue;
                }
            }
            if (calc.forward || calc.reverce) w += owner->distance(nodes[i], nodes[i-1]);
            if (calc.forward && calc.reverce) break;
        }
    }
    if (!(has1 && has2))
    {
        qDebug() << "in " << nodes << "\n no nodes " << fromNode << " or " << toNode;
        lengthes[fromNode].insert(toNode, W_INF);
        return W_INF;
    }
    lengthes[fromNode].insert(toNode, w);
    return w;
}

TOSMWidget::TID TOSMWidget::TNWay::getOtherEnd(TID node)
{
    qDebug() << node << " " << nodes;
    if (node == nodes.first())
    {
        return nodes.last();
    }
    if (node == nodes.last())
    {
        return nodes.first();
    }
    return BAD_TID;
}

TOSMWidget::EDirection TOSMWidget::TNWay::getDirectionFrom(TID node)
{
    if (node == nodes.first())
    {
        return Forward;
    }
    if (node == nodes.last())
    {
        return Reverce;
    }
    return Both;
}

void TOSMWidget::fillUsage()
{



}


