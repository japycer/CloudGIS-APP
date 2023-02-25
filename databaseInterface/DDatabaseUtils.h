#include "qsqldatabase.h"
#include"qsqlquery.h"


// 用于执行一些数据库常用功能
class DDatabaseUtils
{
public:
	static DDatabaseUtils* Instance();
	bool connectDatabase(QString &dbName);	//根据dbName去连接数据库
	void closeDatabase();
	bool openDatabase();
	bool createTable(QString command = QString());	// 如果表不存在，创建一个表
	bool insertData(QString &command);	// 向数据库插入数据，一次一条
	QSqlDatabase& getDatabase();	// 返回数据库对象
	bool excuCommand(QString&);
	QSqlQuery* getSqlQuery();

private:
	DDatabaseUtils();
	QSqlDatabase m_db;// = QSqlDatabase::addDatabase("QSQLITE");
	QSqlQuery *m_sql_query = nullptr;	// 执行数据库操作的query变量
	static DDatabaseUtils* mInstance;
};
