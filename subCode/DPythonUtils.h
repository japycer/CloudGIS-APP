#pragma once
#include<QString>
#include"qgspythonrunner.h"
#include"qobject.h"
class QSettings;

class DInterface;
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif


// 这是一个单例模式，采用懒汉模式创建
class DPythonUtils:public QObject
{
	Q_OBJECT
public:
	static DPythonUtils* Instance(QObject *parent=0);
	~DPythonUtils();

	void initPython(DInterface *, const bool);
	bool runString(const QString &command, QString msgOnError = QString(), bool single = true);
	void exitPython();
	bool evalString(const QString &command, QString &result);

	QString getLastRes() { return lastRes; };
	QString getOutPut();
	//QList<QList<QString>*> * getList(QString &keyword);

signals:
	void occurErrMsg(QString msg);

private:
	DPythonUtils(QObject *parent = 0);
	void init();
	bool initPythonEnvs();
	bool checkSystemImports();
	void installErrorHook();
	void uninstallErrorHook();
	QString runStringUnsafe(const QString &command, bool single = true);
	QString getTraceback();
	void finish();

	//! convert Python object to QString. If the object isn't unicode/str, it will be converted
	QString PyObjectToQString(PyObject *obj);

	//! reference to module __main__
	PyObject *mMainModule = nullptr;
	PyObject *pModuleRedirection = nullptr;
	PyObject *pFunRedirection = nullptr;

	PyObject *pModuleSearchGEE = nullptr;
	PyObject *pFunPopData = nullptr;
	PyObject *pFunsetSearcherKeyWords = nullptr;
	//! dictionary of module __main__
	PyObject *mMainDict = nullptr;

	//! flag determining that Python support is enabled
	bool mPythonEnabled = false;
	bool isFirstGetOutput = true;
	bool isFirstSearchGEE = true;
	bool mErrorHookInstalled = false;

	QSettings *mSettings = nullptr;
	QString lastRes;

	static DPythonUtils* mInstance;
};
