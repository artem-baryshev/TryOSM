#ifndef TOSMTOSQLITE_H
#define TOSMTOSQLITE_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>

typedef long long TID;
const TID unknownID = -1;

class TOsmToSqlite
{
public:
protected:
    enum TOSMReadMode
    {
        Unknown,
        Node,
        Way,
        Relation
    };
    typedef QMap <QString, TID> TIDMap;
    TIDMap tags, roles, users, tagsvalues;
    QSqlQuery query;
    void selectIDs(TIDMap &idmap, QString tname);
    TID getId(TIDMap &idmap, QString name, QString tname);
    QString readLine(std::ifstream & ifs);
    bool head(QString &S, QString h);
    bool tail(QString &S, QString h);
    QString extractOption(QString &S, QString Opt);
    void readNode(QString &S, TID &id);
    void readWay(QString &S, TID &id);
    void readRelation(QString &S, TID &id);
    void readNd(QString &S, TID &id);
    void readMember(QString &S, TID &id);
    void readTag(QString &S, TID &id, TOSMReadMode mode);
    void createOSMTables();
public:
    TOsmToSqlite(QString osmFile, QString sqliteFile, bool createTables = false);
};

#endif // TOSMTOSQLITE_H
