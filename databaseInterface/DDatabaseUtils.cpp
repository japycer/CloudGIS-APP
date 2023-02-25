#include <QMessageBox>
#include "DDatabaseUtils.h"
#include"qdebug.h"
#include"qsqlerror.h"

DDatabaseUtils *DDatabaseUtils::mInstance = nullptr;

DDatabaseUtils* DDatabaseUtils::Instance()
{
	if (mInstance == nullptr)
	{
		mInstance = new DDatabaseUtils();
	}
	return mInstance;
}

DDatabaseUtils::DDatabaseUtils(){}


bool DDatabaseUtils::connectDatabase(QString & dbName)
{
	// 不进行重复连接
	if (!m_db.databaseName().isEmpty()) return true;

	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_db.setDatabaseName(dbName);
	m_sql_query = new QSqlQuery();
	if (!m_db.open())
	{
		qDebug() << "Error: Failed to connect database." << m_db.lastError().text();
		return false;
	}
	else
	{
		qDebug() << "Succeed to connect database.";
		return true;
	}

	return false;
}


void DDatabaseUtils::closeDatabase()
{
	qDebug() << "disconnected with database !";
	m_db.close();
}


bool DDatabaseUtils::openDatabase()
{
	bool ok = m_db.open();
	if(ok)
		qDebug() << "Succeed to open database.";
	else
		qDebug() << "Fail to open database.";
	return ok;
}


bool DDatabaseUtils::createTable(QString command)
{
	if (command.isEmpty())
		return false;
	//创建表格
	if (!m_sql_query->exec(command))
	{
		qDebug() << "Error: Fail to create table." << m_sql_query->lastError().text();
		return false;
	}
	else
	{
		qDebug() << "Table created!";
		return true;
	}
	return false;
}


bool DDatabaseUtils::insertData(QString &command)
{
	if (command.isEmpty())
		return false;
	//插入数据
	if (!m_sql_query->exec(command))
	{
		qDebug() << m_sql_query->lastError().text();
		return false;
	}
	else
	{
		qDebug() << "inserted a row!";
		return true;
	}
	return false;
}


QSqlDatabase& DDatabaseUtils::getDatabase()
{
	return m_db;
}


bool DDatabaseUtils::excuCommand(QString& command)
{
	if (command.isEmpty())
		return false;
	//插入数据
	if (!m_sql_query->exec(command))
	{
		QMessageBox::critical(nullptr, QStringLiteral("SQL Query Error"), m_sql_query->lastError().text(), QMessageBox::Ok);
		return false;
	}
	else
		return true;
	return false; 
}


QSqlQuery* DDatabaseUtils::getSqlQuery()
{
	return m_sql_query;
}