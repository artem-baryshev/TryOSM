#ifndef ROUTINGPROFILES_H
#define ROUTINGPROFILES_H

#include "tosmwidget.h"

class TPedestrianProfile : public TOSMWidget::TRouteProfile
{
public:
    TOSMWidget::TWeight getWayWeight(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo);
    TOSMWidget::TWeight getNodeWeight(TOSMWidget::TNNode *node, TOSMWidget::TID wayFrom, TOSMWidget::TID wayTo);
    bool isPassable(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo);
};

class TCarProfile : public TOSMWidget::TRouteProfile
{
public:
    TOSMWidget::TWeight getWayWeight(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo);
    TOSMWidget::TWeight getNodeWeight(TOSMWidget::TNNode *node, TOSMWidget::TID wayFrom, TOSMWidget::TID wayTo);
    bool isPassable(TOSMWidget::TNWay *way, TOSMWidget::TID nodeFrom, TOSMWidget::TID nodeTo);
};

#endif // ROUTINGPROFILES_H
