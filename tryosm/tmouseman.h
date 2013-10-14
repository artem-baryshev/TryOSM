#ifndef TMOUSEMAN_H
#define TMOUSEMAN_H

#include <QObject>
#include <QMouseEvent>
#include <QWheelEvent>

class TMouseMan : public QObject
{
    Q_OBJECT
    bool leftPressed, righPressed, midPressed;
    QPoint leftOldPos, rightOldPos, midOldPos;

public:
    explicit TMouseMan(QObject *parent = 0);
//    QPoint getOldPos
//    void pushEvent
signals:
    
public slots:
//    void mouseMoveEvent(QMouseEvent *e);
//    void mousePressEvent(QMouseEvent *e);
//    void mouseReleaseEvent(QMouseEvent *e);
//    void mouseDoubleClickEvent(QMouseEvent *e);
//    void wheelEvent(QWheelEvent *e);
};

#endif // TMOUSEMAN_H
