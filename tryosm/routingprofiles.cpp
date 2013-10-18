#include "routingprofiles.h"

TOSMWidget::TWeight TPedestrianProfile::getWayWeight(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo)
{
    TOSMWidget::TWeight spd = 6;
    switch(way->roadClass)
    {
        case TOSMWidget::TNWay::EW_Motorway:
        case TOSMWidget::TNWay::EW_Trunk:
        case TOSMWidget::TNWay::EW_Primary:
        case TOSMWidget::TNWay::EW_Secondary:
        case TOSMWidget::TNWay::EW_Tertiary:
        case TOSMWidget::TNWay::EW_LivingStreet:
        case TOSMWidget::TNWay::EW_Residental:
        case TOSMWidget::TNWay::EW_Unclassified:
        case TOSMWidget::TNWay::EW_Service:
        case TOSMWidget::TNWay::EW_Track:
        case TOSMWidget::TNWay::EW_Raceway:
        case TOSMWidget::TNWay::EW_Road:
        case TOSMWidget::TNWay::EW_BusGuideway:
        case TOSMWidget::TNWay::EW_Construction:  spd = 4; break;
        default: spd = 6;
    }
    return way->getLength(nodeFrom, nodeTo)/spd;
}

TOSMWidget::TWeight TPedestrianProfile::getNodeWeight(TOSMWidget::TNNode *node, TOSMWidget::TID wayFrom, TOSMWidget::TID wayTo)
{
    return 0;
}

bool TPedestrianProfile::isPassable(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo)
{
    return true;
}

TOSMWidget::TWeight TCarProfile::getWayWeight(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo)
{
    if (!isPassable(way, nodeFrom, nodeTo)) return W_INF;
    TOSMWidget::TWeight spd;
    switch(way->roadClass)
    {
        case TOSMWidget::TNWay::EW_Motorway: spd = 110; break;
        case TOSMWidget::TNWay::EW_Trunk: spd = 70; break;
        case TOSMWidget::TNWay::EW_Primary: spd = 90; break;
        case TOSMWidget::TNWay::EW_Secondary: spd = 80; break;
        case TOSMWidget::TNWay::EW_Tertiary: spd = 80; break;
        case TOSMWidget::TNWay::EW_LivingStreet: spd = 40; break;
        case TOSMWidget::TNWay::EW_Residental: spd = 60; break;
        case TOSMWidget::TNWay::EW_Unclassified: spd = 40; break;
        case TOSMWidget::TNWay::EW_Service: spd = 20; break;
        case TOSMWidget::TNWay::EW_Track: spd = 10; break;
        case TOSMWidget::TNWay::EW_Raceway: spd = 180; break;
        case TOSMWidget::TNWay::EW_Road: spd = 40; break;
        default: spd = 5;
    }
    return way->getLength(nodeFrom, nodeTo)/spd;
}

TOSMWidget::TWeight TCarProfile::getNodeWeight(TOSMWidget::TNNode *node, TOSMWidget::TID wayFrom, TOSMWidget::TID wayTo)
{
    return 0;
}

TOSMWidget::TWeight TPedestrianProfile::getMaxWaySpeed()
{
    return 6.0;
}

TOSMWidget::TWeight TCarProfile::getMaxWaySpeed()
{
    return 110.0;
}

bool TCarProfile::isPassable(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo)
{
    switch (way->roadClass)
    {
        case TOSMWidget::TNWay::EW_Pedestrian: return false;
        case TOSMWidget::TNWay::EW_BusGuideway: return false;
        case TOSMWidget::TNWay::EW_Path: return false;
        case TOSMWidget::TNWay::EW_Footway: return false;
        case TOSMWidget::TNWay::EW_Bridleway: return false;
        case TOSMWidget::TNWay::EW_Steps: return false;
        case TOSMWidget::TNWay::EW_Cycleway: return false;
        case TOSMWidget::TNWay::EW_Proposed: return false;
        case TOSMWidget::TNWay::EW_Construction: return false;
        default:;
    }
    switch (way->oneWay)
    {
    case TOSMWidget::TNWay::OW_no: return true;
    case TOSMWidget::TNWay::OW_yes_forward: return (way->getNodeIndex(nodeFrom) <= way->getNodeIndex(nodeTo));
    case TOSMWidget::TNWay::OW_yes_reverce: return (way->getNodeIndex(nodeFrom) >= way->getNodeIndex(nodeTo));
    }
    return false;
}

