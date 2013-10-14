#include "tosmtosqlite.h"
#include <fstream>
#include <QVariant>
#include <QDebug>
#include <QTime>

#define NO_USELESS_DATA

bool TOsmToSqlite::head(QString &S, QString h)
{
    if (h.length() > S.length()) return false;
    return S.left(h.length()) == h;
}

bool TOsmToSqlite::tail(QString &S, QString h)
{
    if (h.length() > S.length()) return false;
    return S.right(h.length()) == h;
}

QString TOsmToSqlite::extractOption(QString &S, QString Opt)
{
    int p1 = S.indexOf(Opt + "=\"") + Opt.length() + 2;
    int p2 = S.indexOf("\"", p1 + 1);
    return S.mid(p1, p2-p1);
}

void TOsmToSqlite::readNode(QString &S, TID &id)
{
#ifndef NO_USELESS_DATA
    TID user = getId(users, extractOption(S, "user"), "t_users");
    query.prepare("insert into t_nodes values (?, ?, ?, ?, ?, ?, ?)");
#else
    query.prepare("insert into t_nodes values (?, ?, ?)");
#endif
    id = extractOption(S, "id").toLongLong();
    query.addBindValue(id);
#ifndef NO_USELESS_DATA
    query.addBindValue(extractOption(S, "version").toInt());
    query.addBindValue(extractOption(S, "timestamp"));
    query.addBindValue(extractOption(S, "uid").toLongLong());
    query.addBindValue(user);
#endif
    query.addBindValue(extractOption(S, "lat").toDouble());
    query.addBindValue(extractOption(S, "lon").toDouble());
    if (!query.exec())
    {
        qDebug() << "!!! Failed to insert node !!!";
        exit(0);
    }
}

void TOsmToSqlite::readWay(QString &S, TID &id)
{
#ifndef NO_USELESS_DATA
    TID user = getId(users, extractOption(S, "user"), "t_users");
    query.prepare("insert into t_ways values (?, ?, ?, ?, ?, ?)");
#else
    query.prepare("insert into t_ways values (?)");
#endif
    id = extractOption(S, "id").toLongLong();
    query.addBindValue(id);
#ifndef NO_USELESS_DATA
    query.addBindValue(extractOption(S, "version").toInt());
    query.addBindValue(extractOption(S, "timestamp"));
    query.addBindValue(extractOption(S, "uid").toLongLong());
    query.addBindValue(user);
    query.addBindValue(extractOption(S, "changeset"));
#endif
    if (!query.exec())
    {
        qDebug() << "!!! Failed to insert ways !!!";
        exit(0);
    }
}

void TOsmToSqlite::readRelation(QString &S, TID &id)
{
#ifndef NO_USELESS_DATA
    TID user = getId(users, extractOption(S, "user"), "t_users");
    query.prepare("insert into t_rels values (?, ?, ?, ?, ?, ?)");
#else
    query.prepare("insert into t_rels values (?)");
#endif
    id = extractOption(S, "id").toLongLong();
    query.addBindValue(id);
#ifndef NO_USELESS_DATA
    query.addBindValue(extractOption(S, "version").toInt());
    query.addBindValue(extractOption(S, "timestamp"));
    query.addBindValue(extractOption(S, "uid").toLongLong());
    query.addBindValue(user);
    query.addBindValue(extractOption(S, "changeset"));
#endif
    if (!query.exec())
    {
        qDebug() << "!!! Failed to insert relation !!!";
        exit(0);
    }
}

void TOsmToSqlite::readNd(QString &S, TID &id)
{
    query.prepare("insert into t_ways_nodes values (?, ?)");
    query.addBindValue(id);
    query.addBindValue(extractOption(S, "ref").toLongLong());
    if (!query.exec())
    {
        qDebug()  << "!!! Failed to insert way node !!!";
        exit(0);
    }
}

TID TOsmToSqlite::getId(TIDMap &idmap, QString name, QString tname)
{
    TID id;
    if (idmap.contains(name))
    {
        id = idmap[name];
    }
    else
    {
        id = idmap.size();
        idmap.insert(name, id);
        query.prepare((QString)"insert into " + tname + " values (?, ?)");
        query.addBindValue(id);
        query.addBindValue(name);
        if (!query.exec())
        {
            qDebug() << "!!! Failed to insert id to " << tname << " !!!";
            exit(0);
        }
    }
    return id;
}

void TOsmToSqlite::readMember(QString &S, TID &id)
{
    QString stype = extractOption(S, "type");

    QString sRole = extractOption(S, "role");
    TID role = getId(roles, sRole, "t_roles");

    if (stype == "node")
    {
        query.prepare("insert into t_rels_nodes values (?, ?, ?)");
    }
    else if (stype == "way")
    {
        query.prepare("insert into t_rels_ways values (?, ?, ?)");
    }
    else if (stype == "relation")
    {
        query.prepare("insert into t_rels_rels values (?, ?, ?)");
    }
    else
    {
        qDebug() << "!!! Unknown relation member type " << stype << " !!!";
        exit(0);
    }


    query.addBindValue(id);
    query.addBindValue(extractOption(S, "ref").toLongLong());
    query.addBindValue(role);
    if (!query.exec())
    {
        qDebug() << "!!! Failed to insert relation member !!!";
        exit(0);
    }

}

void TOsmToSqlite::readTag(QString &S, TID &id, TOSMReadMode mode)
{
    QString k = extractOption(S, "k"),
            v = extractOption(S, "v");

    TID tag = getId(tags, k, "t_tags"),
        tagvalue = getId(tagsvalues, v, "t_tags_values");

    switch (mode) {
    case Node:
        query.prepare("insert into t_nodes_tags values (?, ?, ?)");
        break;
    case Way:
        query.prepare("insert into t_ways_tags values (?, ?, ?)");
        break;
    case Relation:
        query.prepare("insert into t_rels_tags values (?, ?, ?)");
        break;
    default:
        qDebug() << "!!! bad mode to read attribute !!!";
        exit(0);
    }


    query.addBindValue(id);
    query.addBindValue(tag);
    query.addBindValue(tagvalue);
    if (!query.exec())
    {
        qDebug() << "!!! Failed to insert object attribute !!!";
        exit(0);
    }
}

void TOsmToSqlite::createOSMTables()
{
    query.exec("CREATE TABLE \"t_tags\" ("
               "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
               "\"value\" TEXT NOT NULL"
               ")");
#ifndef NO_USELESS_DATA
    query.exec("CREATE TABLE \"t_users\" ("
               "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
               "\"value\" TEXT NOT NULL"
               ")");
#endif
    query.exec("CREATE TABLE \"t_roles\" ("
               "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
               "\"value\" TEXT NOT NULL"
               ")");
    query.exec("CREATE TABLE \"t_nodes\" ("
               "\"id\" INTEGER PRIMARY KEY NOT NULL,"
           #ifndef NO_USELESS_DATA
               "\"version\" INTEGER,"
               "\"timestamp\" TEXT,"
               "\"uid\" INTEGER,"
               "\"user\" INTEGER,"
           #endif
               "\"lat\" REAL NOT NULL,"
               "\"lon\" REAL NOT NULL"
           #ifndef NO_USELESS_DATA
               ",foreign key(\"user\")references t_users(id)"
           #endif
               ")");
    query.exec("CREATE TABLE \"t_nodes_tags\" ("
               "\"node\" INTEGER NOT NULL,"
               "\"tag\" INTEGER NOT NULL,"
               "\"value\" INTEGER NOT NULL,"
               "foreign key(node)references t_nodes(id),"
               "foreign key(tag)references t_tags(id),"
               "foreign key(value)references t_tags_values(id)"
               ")");
    query.exec("CREATE TABLE \"t_ways\" ("
               "\"id\" INTEGER PRIMARY KEY NOT NULL"
           #ifndef NO_USELESS_DATA
               ",\"version\" INTEGER,"
               "\"timestamp\" TEXT,"
               "\"uid\" INTEGER,"
               "\"user\" INTEGER,"
               "\"changeset\" INTEGER,"
               "foreign key(user)references t_users(id)"
           #endif
               ")");
    query.exec("CREATE TABLE \"t_ways_nodes\" ("
               "\"way\" INTEGER NOT NULL,"
               "\"node\" INTEGER NOT NULL,"
               "FOREIGN KEY (way) references t_ways(id),"
               "FOREIGN KEY (node) references t_nodes(id)"
               ")");
    query.exec("CREATE TABLE \"t_ways_tags\" ("
               "\"way\" INTEGER NOT NULL,"
               "\"tag\" INTEGER NOT NULL,"
               "\"value\" INTEGER NOT NULL,"
               "foreign key(way)references t_ways(id),"
               "foreign key(tag)references t_tags(id),"
               "foreign key(value)references t_tags_values(id)"
               ")");
    query.exec("CREATE TABLE \"t_rels\" ("
               "\"id\" INTEGER PRIMARY KEY NOT NULL"
           #ifndef NO_USELESS_DATA
               ",\"version\" INTEGER,"
               "\"timestamp\" TEXT,"
               "\"uid\" INTEGER,"
               "\"user\" integer,"
               "\"changeset\" INTEGER,"
               "foreign key(user)references t_users(id)"
           #endif
               ")");
    query.exec("CREATE TABLE \"t_rels_nodes\" ("
               "\"rel\" INTEGER NOT NULL,"
               "\"node\" INTEGER NOT NULL,"
               "\"role\" INTEGER NOT NULL,"
               "foreign key(rel)references t_rels(id),"
               "foreign key(node)references t_nodes(id),"
               "foreign key(role)references t_roles(id)"
               ")");
    query.exec("CREATE TABLE \"t_rels_rels\" ("
               "\"rel\" INTEGER NOT NULL,"
               "\"subrel\" INTEGER NOT NULL,"
               "\"role\" INTEGER NOT NULL,"
               "foreign key(rel)references t_rels(id),"
               "foreign key(subrel)references t_rels(id),"
               "foreign key(role)references t_roles(id)"
               ")");
    query.exec("CREATE TABLE \"t_rels_tags\" ("
               "\"rel\" INTEGER NOT NULL,"
               "\"tag\" INTEGER NOT NULL,"
               "\"value\" INTEGER NOT NULL,"
               "foreign key(rel)references t_rels(id),"
               "foreign key(tag)references t_tags(id),"
               "foreign key(value)references t_tags_values(id)"
               ")");
    query.exec("CREATE TABLE \"t_rels_ways\" ("
               "\"rel\" INTEGER NOT NULL,"
               "\"way\" INTEGER NOT NULL,"
               "\"role\" INTEGER NOT NULL,"
               "foreign key(rel)references t_rels(id),"
               "foreign key(way)references t_ways(id),"
               "foreign key(role)references t_roles(id)"
               ")");
    query.exec("CREATE TABLE \"t_tags_values\" ("
               "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
               "\"value\" TEXT NOT NULL"
               ")");

    query.exec("CREATE INDEX \"i_r_i_a\" on t_rels (id ASC)");
    query.exec("CREATE INDEX \"i_ro_i_a\" on t_roles (id ASC)");
    query.exec("CREATE INDEX \"i_n_i_a\" on t_nodes (id ASC)");
    query.exec("CREATE INDEX \"i_t_i_a\" on t_tags (id ASC)");
    query.exec("CREATE INDEX \"i_t_v_a\" on t_tags (value ASC)");
    query.exec("CREATE INDEX \"i_tv_i_a\" on t_tags_values (id ASC)");
    query.exec("CREATE INDEX \"i_tv_v_a\" on t_tags_values (value ASC)");
    query.exec("CREATE INDEX \"i_w_i_a\" on t_ways (id ASC)");
#ifndef NO_USELESS_DATA
    query.exec("CREATE INDEX \"i_u_i_a\" on t_users (id ASC)");
#endif

    query.exec("CREATE INDEX \"i_wt_t_a\" on t_ways_tags (tag ASC)");
    query.exec("CREATE INDEX \"i_wt_w_a\" on t_ways_tags (way ASC)");

    query.exec("CREATE INDEX \"i_wn_n_a\" on t_ways_nodes (node ASC)");
    query.exec("CREATE INDEX \"i_wn_w_a\" on t_ways_nodes (way ASC)");

    query.exec("CREATE INDEX \"i_nt_n_a\" on t_nodes_tags (node ASC)");
    query.exec("CREATE INDEX \"i_nt_t_a\" on t_nodes_tags (tag ASC)");

    query.exec("CREATE INDEX \"i_rn_r_a\" on t_rels_nodes (rel ASC)");
    query.exec("CREATE INDEX \"i_rn_n_a\" on t_rels_nodes (node ASC)");

    query.exec("CREATE INDEX \"i_rr_r_a\" on t_rels_rels (rel ASC)");
    query.exec("CREATE INDEX \"i_rr_s_a\" on t_rels_rels (subrel ASC)");

    query.exec("CREATE INDEX \"i_rt_r_a\" on t_rels_tags (rel ASC)");
    query.exec("CREATE INDEX \"i_rt_t_a\" on t_rels_tags (tag ASC)");

    query.exec("CREATE INDEX \"i_rw_r_a\" on t_rels_ways (rel ASC)");
    query.exec("CREATE INDEX \"i_rw_w_a\" on t_rels_ways (way ASC)");

}

TOsmToSqlite::TOsmToSqlite(QString osmFile, QString sqliteFile, bool createTables)
{
    qDebug() << QTime::currentTime().toString() << " start converting";

    std::ifstream ifs(osmFile.toStdString().c_str());

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(sqliteFile.toStdString().c_str());

    if (!db.open())
    {
        qDebug() << "!!! Failed to open database !!!";
        exit(0);
    }

    TOSMReadMode mode = Unknown;

    TID id;

    query = QSqlQuery(db);

    if (createTables)
    {
        createOSMTables();
    }

    selectIDs(tags, "t_tags");
    selectIDs(roles, "t_roles");
    selectIDs(tagsvalues, "t_tags_values");
#ifndef NO_USELESS_DATA
    selectIDs(users, "t_users");
#endif

    query.exec("begin;");

    QString line;

    line = readLine(ifs);

    while (!line.isEmpty()) {

        line = line.trimmed();

//        qDebug() << line;

        if (head(line, "</"))
        {
            mode = Unknown;
        }
        else
        {
            if (head(line, "<"))
            {
                if (head(line, "<node "))
                {
                    if (!tail(line, "/>")) mode = Node;
                    readNode(line, id);
                }
                else if (head(line, "<way "))
                {
                    if (!tail(line, "/>")) mode = Way;
                    readWay(line, id);

                }
                else if (head(line, "<relation "))
                {
                    if (!tail(line, "/>")) mode = Relation;
                    readRelation(line, id);

                }
                else if (head(line, "<tag "))
                {
                    readTag(line, id, mode);
                }
                else if (head(line, "<nd "))
                {
                    readNd(line, id);
                }
                else if (head(line, "<member "))
                {
                    readMember(line, id);
                }
            }
        }

        line = readLine(ifs);
    }

    query.exec("commit;");

    ifs.close();

    db.close();

    qDebug() << QTime::currentTime().toString() << " finish converting";

}

void TOsmToSqlite::selectIDs(TIDMap &idmap, QString tname)
{
    if (query.exec(QString("select id, value from ") + tname))
    {
        while (query.next()) {
            idmap.insert(query.value(1).toString(), query.value(0).toLongLong());
        }
    }
}

QString TOsmToSqlite::readLine(std::ifstream & ifs)
{
    QString curLine;
    if(!ifs.good()) return "";
    const size_t max_read_line = 32757;
    char buf[max_read_line];
    ifs.getline(buf, max_read_line, '\n');
    curLine.clear();
    curLine.append(buf);
    return curLine;
}

