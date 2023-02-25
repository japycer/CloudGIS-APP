#include "qsqldatabase.h"
#include"qsqlquery.h"


// ����ִ��һЩ���ݿⳣ�ù���
class DDatabaseUtils
{
public:
	static DDatabaseUtils* Instance();
	bool connectDatabase(QString &dbName);	//����dbNameȥ�������ݿ�
	void closeDatabase();
	bool openDatabase();
	bool createTable(QString command = QString());	// ��������ڣ�����һ����
	bool insertData(QString &command);	// �����ݿ�������ݣ�һ��һ��
	QSqlDatabase& getDatabase();	// �������ݿ����
	bool excuCommand(QString&);
	QSqlQuery* getSqlQuery();

private:
	DDatabaseUtils();
	QSqlDatabase m_db;// = QSqlDatabase::addDatabase("QSQLITE");
	QSqlQuery *m_sql_query = nullptr;	// ִ�����ݿ������query����
	static DDatabaseUtils* mInstance;
};
