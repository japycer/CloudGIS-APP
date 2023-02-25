#include<Python.h>
#include<QObject>
#include"DPythonUtils.h"
#include"DInterface.h"
#include "qsettings.h"
#include "qfileinfo.h"


PyThreadState *_mainState = nullptr;
DPythonUtils* DPythonUtils::mInstance = nullptr;

DPythonUtils* DPythonUtils::Instance(QObject *parent)
{
	if (mInstance == nullptr)
	{
		mInstance = new DPythonUtils(parent);
	}
	return mInstance;
}


DPythonUtils::DPythonUtils(QObject *parent):QObject(parent)
{
	mSettings = new QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
	init();
}


DPythonUtils::~DPythonUtils()
{
}

// 主要为配置一些qgis的环境
void DPythonUtils::initPython(DInterface * interface, const bool installErrorHook)
{
	if (!checkSystemImports())
	{
		exitPython();
		return;
	}

	if (interface)
	{
		// initialize 'iface' object
		runString(QStringLiteral("qgis.utils.initInterface(%1)").arg(reinterpret_cast<quint64>(interface)));
	}

	if (installErrorHook)
		DPythonUtils::installErrorHook();

	initPythonEnvs();
	finish();
}

void DPythonUtils::init()
{
	// 设置python.exe的位置
	QString qPythonPath = mSettings->value("/setting/python/PYTHON_HOMEPATH").toString();

	const wchar_t * pythonHome = reinterpret_cast<const wchar_t*>(qPythonPath.utf16());
	Py_SetPythonHome(pythonHome);
	// 初始化python解释器
	Py_Initialize();
	// 初始化threading AND acquire GIL
	PyEval_InitThreads();

	mPythonEnabled = true;

	mMainModule = PyImport_AddModule("__main__");
	mMainDict = PyModule_GetDict(mMainModule);
}

// TODO
bool DPythonUtils::checkSystemImports()
{
	QString exeRootDir = QCoreApplication::applicationDirPath();	//exe所在路径
	// 进行环境变量的初始化
	bool ok1 = runString(QStringLiteral("import sys")); // 导入sys，设置环境变量
	QString qgisInstallPath = mSettings->value("/setting/qgis/INSTALL_PATH").toString();
	QString usrName = mSettings->value("/setting/usr/USRNAME").toString();

	QString PluginPath = QString("'C:/Users/{NAME}/AppData/Roaming/QGIS/QGIS3/profiles/default/python', 'C:/Users/{NAME}/AppData/Roaming/QGIS/QGIS3/profiles/default/python/plugins'").replace("{NAME}", usrName);
	QString defaultPath = QString("'{*}/apps/qgis/./python/plugins', '{*}/apps/Python37', '{*}/apps/Python37/Scripts', '{*}/bin/python37.zip', '{*}/apps/Python37/DLLs', '{*}/apps/Python37/lib', '{*}/bin', '{*}/apps/Python37/lib/site-packages', '{*}/apps/Python37/lib/site-packages/win32', '{*}/apps/Python37/lib/site-packages/win32/lib', '{*}/apps/Python37/lib/site-packages/Pythonwin'");
	defaultPath = defaultPath.replace("{*}", qgisInstallPath);
	QString eePath = mSettings->value("/setting/python/GEE_PACKAGE").toString();

	QString prePath = QString("'{*}/apps/qgis/./python'").replace("{*}", qgisInstallPath);
	QString eePluginPath = "'" + exeRootDir + "/Dpython" + "'";

	QString path = "[" + prePath + "," + PluginPath + "," + defaultPath + "," + "'" + eePath + "'" + "," + eePluginPath + ","  + "'./'" + "]";//+ PluginPath + ","

	bool ok2 = runString(QStringLiteral("sys.path=%1").arg(path));

	if (!ok1)
	{
		occurErrMsg("cannot import sys");
		return false;
	}

	if (!ok2)
	{
		occurErrMsg("An error occur during executing sys.path=...");
		return false;
	}

	// 进行Qgis包的初始化
	// import SIP -- SIP是一个联系python和C++的包，能够对C++的函数和类进行封装，然后进行Python调用
	if (!runString(QStringLiteral("from qgis.PyQt import sip"),
		QObject::tr("Couldn't load SIP module.") + '\n' + QObject::tr("Python support will be disabled.")))
	{
		return false;
	}

	// set PyQt api versions
	// 查看sip的版本支不支持，不支持则退出
	QStringList apiV2classes;
	apiV2classes << QStringLiteral("QDate") << QStringLiteral("QDateTime") << QStringLiteral("QString") << QStringLiteral("QTextStream") << QStringLiteral("QTime") << QStringLiteral("QUrl") << QStringLiteral("QVariant");
	Q_FOREACH(const QString &clsName, apiV2classes)
	{
		if (!runString(QStringLiteral("sip.setapi('%1', 2)").arg(clsName),
			QObject::tr("Couldn't set SIP API versions.") + '\n' + QObject::tr("Python support will be disabled.")))
		{
			return false;
		}
	}

	// import Qt bindings
	if (!runString(QStringLiteral("from PyQt5 import QtCore, QtGui"),
		QObject::tr("Couldn't load PyQt.") + '\n' + QObject::tr("Python support will be disabled.")))
	{
		return false;
	}

	// import QGIS bindings
	QString error_msg = QObject::tr("Couldn't load PyQGIS.") + '\n' + QObject::tr("Python support will be disabled.");
	if (!runString(QStringLiteral("from qgis.core import *"), error_msg) || !runString(QStringLiteral("from qgis.gui import *"), error_msg))
	{
		return false;
	}

	// import QGIS utils
	error_msg = QObject::tr("Couldn't load QGIS utils.") + '\n' + QObject::tr("Python support will be disabled.");
	if (!runString(QStringLiteral("import qgis.utils"), error_msg))
	{
		return false;
	}
	return true;
}


bool DPythonUtils::runString(const QString &command, QString msgOnError, bool single)
{
	bool res = true;
	QString traceback = runStringUnsafe(command, single);

	if(!traceback.isEmpty())
		lastRes = traceback;
	if (traceback.isEmpty())
		return true;
	else
		res = false;

	if (msgOnError.isEmpty())
	{
		// use some default message if custom hasn't been specified
		msgOnError = QObject::tr("An error occurred during execution of following code:") + "\n<tt>" + command + "</tt>";
	}

	return res;
}

void DPythonUtils::exitPython()
{
	if (mErrorHookInstalled)
		uninstallErrorHook();
	// causes segfault!
	//Py_Finalize();
	mMainModule = nullptr;
	mMainDict = nullptr;
	mPythonEnabled = false;
}

QString DPythonUtils::runStringUnsafe(const QString & command, bool single)
{
	// acquire global interpreter lock to ensure we are in a consistent state
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();
	QString ret;

	// TODO: convert special characters from unicode strings u"…" to \uXXXX
	// so that they're not mangled to utf-8
	// (non-unicode strings can be mangled)
	PyObject *obj = PyRun_String(command.toUtf8().constData(), single ? Py_single_input : Py_file_input, mMainDict, mMainDict);
	PyObject *errobj = PyErr_Occurred();
	if (nullptr != errobj)
	{
		ret = getTraceback();
	}
	Py_XDECREF(obj);

	// we are done calling python API, release global interpreter lock
	PyGILState_Release(gstate);

	return ret;
}

QString DPythonUtils::getTraceback()
{
#define TRACEBACK_FETCH_ERROR(what) {errMsg = what; goto done;}

	QString errMsg;
	QString result;

	PyObject *modStringIO = nullptr;
	PyObject *modTB = nullptr;
	PyObject *obStringIO = nullptr;
	PyObject *obResult = nullptr;

	PyObject *type = nullptr, *value = nullptr, *traceback = nullptr;

	PyErr_Fetch(&type, &value, &traceback);
	PyErr_NormalizeException(&type, &value, &traceback);

	const char *iomod = "io";

	modStringIO = PyImport_ImportModule(iomod);
	if (!modStringIO)
		TRACEBACK_FETCH_ERROR(QStringLiteral("can't import %1").arg(iomod));

	obStringIO = PyObject_CallMethod(modStringIO, reinterpret_cast<const char *>("StringIO"), nullptr);

	/* Construct a cStringIO object */
	if (!obStringIO)
		TRACEBACK_FETCH_ERROR(QStringLiteral("cStringIO.StringIO() failed"));

	modTB = PyImport_ImportModule("traceback");
	if (!modTB)
		TRACEBACK_FETCH_ERROR(QStringLiteral("can't import traceback"));

	obResult = PyObject_CallMethod(modTB, reinterpret_cast<const char *>("print_exception"),
		reinterpret_cast<const char *>("OOOOO"),
		type, value ? value : Py_None,
		traceback ? traceback : Py_None,
		Py_None,
		obStringIO);

	if (!obResult)
		TRACEBACK_FETCH_ERROR(QStringLiteral("traceback.print_exception() failed"));

	Py_DECREF(obResult);

	obResult = PyObject_CallMethod(obStringIO, reinterpret_cast<const char *>("getvalue"), nullptr);
	if (!obResult)
		TRACEBACK_FETCH_ERROR(QStringLiteral("getvalue() failed."));

	/* And it should be a string all ready to go - duplicate it. */
	if (!PyUnicode_Check(obResult))
		TRACEBACK_FETCH_ERROR(QStringLiteral("getvalue() did not return a string"));

	result = QString::fromUtf8(PyUnicode_AsUTF8(obResult));

done:

	// All finished - first see if we encountered an error
	if (result.isEmpty() && !errMsg.isEmpty())
	{
		result = errMsg;
	}

	Py_XDECREF(modStringIO);
	Py_XDECREF(modTB);
	Py_XDECREF(obStringIO);
	Py_XDECREF(obResult);
	Py_XDECREF(value);
	Py_XDECREF(traceback);
	Py_XDECREF(type);

	return result;
}

bool DPythonUtils::evalString(const QString & command, QString & result)
{
	// acquire global interpreter lock to ensure we are in a consistent state
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();

	PyObject *res = PyRun_String(command.toUtf8().constData(), Py_eval_input, mMainDict, mMainDict);
	bool success = nullptr != res;

	// TODO: error handling

	if (success)
		result = PyObjectToQString(res);

	Py_XDECREF(res);

	// we are done calling python API, release global interpreter lock
	PyGILState_Release(gstate);

	return success;
}

QString DPythonUtils::PyObjectToQString(PyObject * obj)
{
	QString result;

	// is it None?
	if (obj == Py_None)
	{
		return QString();
	}

	// check whether the object is already a unicode string
	if (PyUnicode_Check(obj))
	{
		result = QString::fromUtf8(PyUnicode_AsUTF8(obj));
		return result;
	}

	// if conversion to Unicode failed, try to convert it to classic string, i.e. str(obj)
	PyObject *obj_str = PyObject_Str(obj); // new reference
	if (obj_str)
	{
		result = QString::fromUtf8(PyUnicode_AsUTF8(obj_str));
		Py_XDECREF(obj_str);
		return result;
	}

	// some problem with conversion to Unicode string
	//dxl QgsDebugMsg(QStringLiteral("unable to convert PyObject to a QString!"));
	return QStringLiteral("(qgis error)");
}

void DPythonUtils::installErrorHook()
{
	runString(QStringLiteral("qgis.utils.installErrorHook()"));
	mErrorHookInstalled = true;
}

void DPythonUtils::uninstallErrorHook()
{
	runString(QStringLiteral("qgis.utils.uninstallErrorHook()"));
}

void DPythonUtils::finish()
{
	// release GIL!
	// Later on, we acquire GIL just before doing some Python calls and
	// release GIL again when the work with Python API is done.
	// (i.e. there must be PyGILState_Ensure + PyGILState_Release pair
	// around any calls to Python API, otherwise we may segfault!)
	_mainState = PyEval_SaveThread();
}

bool DPythonUtils::initPythonEnvs()
{
	QString exeRootDir = QCoreApplication::applicationDirPath();	//exe所在路径

	// 进行网络配置
	QString completePath2 = exeRootDir + "/init/default/initEnv.py";
	QString qCommand2 = QStringLiteral("exec(open('{*}'.encode('utf-8')).read())").replace("{*}", completePath2);
	bool ok4 = runString(qCommand2);
	
	// 进行GEE初始化	包括导入 iface, 
	QString completePath3 = exeRootDir + "/init/default/initEE.py";
	QString qCommand3 = QStringLiteral("exec(open('{*}'.encode('utf-8')).read())").replace("{*}", completePath3);
	bool ok5 = runString(qCommand3);

	// 进行标准输出重定向，这个对于本程序的python命令行制作很重要
	// 获得输出重定向文件所在路径
	QString completePath1 = exeRootDir + "/Dpython/redirection.py";
	QString qCommand1 = QStringLiteral("exec(open('{*}'.encode('utf-8')).read())").replace("{*}", completePath1);
	bool ok3 = runString(qCommand1);

	if (!ok3)
		occurErrMsg("cannot initiate redirection");
	if (!ok4)
		occurErrMsg("cannot initiate network env");
	if (!ok5)
		occurErrMsg("cannot initiate ee");

	// 进行用户自定义初始化
	// 判断自定义文件目录下是否存在文件
	QString completePath4 = exeRootDir + "/init/custom/__init__.py";
	QFileInfo fileInfo(completePath4);
	if (!fileInfo.isFile())
		return true;
	QString qCommand4 = QStringLiteral("exec(open('{*}'.encode('utf-8')).read())").replace("{*}", completePath4);
	runString(qCommand4);
	return true;
}


//获取重定向后的输出
QString DPythonUtils::getOutPut()
{
	PyGILState_STATE gstate;
	gstate = PyGILState_Ensure();
	if (isFirstGetOutput) {
		pModuleRedirection = PyImport_ImportModule("redirection");
		if (!pModuleRedirection)
			return QString("");
		pFunRedirection = PyObject_GetAttrString(pModuleRedirection, "getContent");
		isFirstGetOutput = false;
	}
	char *ret, *temp;

	PyObject *outP = PyEval_CallObject(pFunRedirection, NULL);
	PyArg_ParseTuple(outP, "s|s", &ret, &temp);
	QString res = QString::fromUtf8(ret);
	PyGILState_Release(gstate);
	return res;
}

// 过时的方法
//QList<QList<QString>*> * DPythonUtils::getList(QString &keyword)
//{
//	PyGILState_STATE gstate;
//	gstate = PyGILState_Ensure();
//	if (isFirstSearchGEE) {
//		QString exeRootDir = QCoreApplication::applicationDirPath();	//exe所在路径
//
//		QString pyLocation = exeRootDir + "/Dpython/searchGEEData.py";
//		QString qCommand = QString("exec(open('%1'.encode('utf-8')).read())").arg(pyLocation);
//		PyRun_SimpleString(qCommand.toLatin1().data());
//		//PyRun_SimpleString("exec(open('D:/myProjects/QGIS-GEE/DGisApp/DGisApp/x64/Release/Dpython/searchGEEData.py'.encode('utf-8')).read())");
//							//exec(open('D:/myProjects/QGIS-GEE/DGisApp/DGisApp/x64/Release/Dpython/searchGEEData.py'.encode('utf-8')).read())
//		pModuleSearchGEE = PyImport_ImportModule("searchGEEData");
//		if (!pModuleSearchGEE)
//			return nullptr;
//
//		pFunPopData = PyObject_GetAttrString(pModuleSearchGEE, "popData");
//		pFunsetSearcherKeyWords = PyObject_GetAttrString(pModuleSearchGEE, "setSearcherKeyWords");
//		isFirstSearchGEE = false;
//	}
//
//	QByteArray byteArg = keyword.toLatin1();
//	PyObject *pArg = Py_BuildValue("(s)", byteArg.data());
//
//	if (!pFunPopData || !pFunsetSearcherKeyWords)
//		return nullptr;
//	char *str1, *str2, *str3, *str4, *str5, *str6;
//
//	PyEval_CallObject(pFunsetSearcherKeyWords, pArg);
//	PyObject *outP;
//	try
//	{
//		while (true)
//		{
//			QList<QString>* mLists = new QList<QString>();
//			outP = PyEval_CallObject(pFunPopData, NULL);
//			PyArg_ParseTuple(outP, "s|s|s|s|s|s", &str1, &str2, &str3, &str4, &str5, &str6);
//
//			QString s1 = QString::fromUtf8(str1);
//			QString s2 = QString::fromUtf8(str2);
//			QString s3 = QString::fromUtf8(str3);
//			QString s4 = QString::fromUtf8(str4);
//			QString s5 = QString::fromUtf8(str5);
//			QString s6 = QString::fromUtf8(str6);
//			if (s1.isEmpty())
//				break;
//			mLists->append(s1);
//			mLists->append(s2);
//			mLists->append(s3);
//			mLists->append(s4);
//			mLists->append(s5);
//			mLists->append(s6);
//			searchLists->append(mLists);
//		}
//	}
//	catch (const std::exception& ex)
//	{
//		qDebug() << QString(ex.what());
//	}
//
//	PyGILState_Release(gstate);
//	return searchLists;
//}
