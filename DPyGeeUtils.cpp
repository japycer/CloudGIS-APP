#include "DPyGeeUtils.h"

QString DPyGeeUtils::tempVarName = QString();	// 保存运行中较大变量的变量名
QString DPyGeeUtils::mTempVarNameInfo = QString();

void DPyGeeUtils::setTempVarName(QString& name)
{
	tempVarName = name;
	bool res = DPythonUtils::Instance()->runString(QString("%1_info=%1.toDictionary().getInfo()").arg(tempVarName));
}


bool DPyGeeUtils::releaseTempEEObj()
{
	bool res = DPythonUtils::Instance()->runString(QString("del %1").arg(tempVarName));
	return res;
}

bool DPyGeeUtils::runString(QString& command)
{
	bool res = DPythonUtils::Instance()->runString(command, QString(), true);
	return res;
}

QString DPyGeeUtils::getEEObjValue(const QString &varName, bool *res)
{
	*res = DPythonUtils::Instance()->runString(QString("print(%1.getInfo())").arg(varName), QString(), true);
	QString outP;
	if (res)
		outP = DPythonUtils::Instance()->getOutPut();
	else
		outP = QString("ERROR").append(DPythonUtils::Instance()->getLastRes());
	return outP;
}


QString DPyGeeUtils::getImgCollectionDateBetween(const QString &varName, bool *res)
{
	//bool ok;
	QString command1 = QString("print(ee.Date(%1_info['date_range'][0]).format('yyyy-MM-dd').getInfo())").arg(tempVarName);
	QString command2 = QString("print(ee.Date(%1_info['date_range'][1]).format('yyyy-MM-dd').getInfo())").arg(tempVarName);

	*res = DPythonUtils::Instance()->runString(command1, QString(), true);
	QString timeStart;
	if (*res)
		timeStart = DPythonUtils::Instance()->getOutPut();

	*res = DPythonUtils::Instance()->runString(command2, QString(), true);
	QString timeEnd;
	if (*res)
		timeEnd = DPythonUtils::Instance()->getOutPut();

	return timeStart.simplified() + "," + timeEnd.simplified();
}

QString DPyGeeUtils::getTempVarNameInfo(){return mTempVarNameInfo;}

QString DPyGeeUtils::getTempVarName(){return tempVarName;}


QString DPyGeeUtils::getImgCollectionHtmlDescription()
{
	QString command = QString("print(%1_info['description'])").arg(tempVarName);

	bool ok = DPythonUtils::Instance()->runString(command, QString(), true);
	QString outP;
	if (ok)
		outP = DPythonUtils::Instance()->getOutPut();
	else
		outP = QString("ERROR").append(DPythonUtils::Instance()->getLastRes());
	return outP;
}

QString DPyGeeUtils::getImgCollectionBandsNames(QString &varName)
{
	QString name;
	if (varName.isEmpty())
		name = tempVarName;
	else
		name = varName;

	QString command = QString("print(%1.first().bandNames().join('&').getInfo())").arg(name);
	bool ok = DPythonUtils::Instance()->runString(command, QString(), true);
	QString outP;
	if (ok)
		outP = DPythonUtils::Instance()->getOutPut();
	else
		outP = QString("ERROR");
	return outP;
}

// 发生错误，python 暂时未找到require函数的实现
QString DPyGeeUtils::getEEPaletteStr(QString &url, int level)
{
	// require('users/gena/packages:palettes').colorbrewer.RdYlGn[9]
	QString command = QString("print((require('users/gena/packages:palettes').%1[%2]).getInfo())").arg(url).arg(level);
	bool ok = DPythonUtils::Instance()->runString(command, QString(), true);
	QString outP;
	if (ok)
		outP = DPythonUtils::Instance()->getOutPut();
	else
		outP = QString("[]");
	return outP;
}

QString DPyGeeUtils::getLatestRunOut(bool ok)
{
	if (ok)
		return DPythonUtils::Instance()->getOutPut().simplified();
	else
		return QString("ERROR").append(DPythonUtils::Instance()->getLastRes());
}

QString DPyGeeUtils::getEEObjType(QString& varName)
{
	bool resImport = runString(QString("from EEPlugin import DUtils"));
	if (!resImport) return QString("ERROR");

	QString command("print(DUtils.getObjType(%1))");
	command = command.arg(varName);
	bool resBool = runString(command);
	if (!resBool)
	{
		getLatestRunOut(false);
		return QString("ERROR");
	}
	else
		return getLatestRunOut(true);
}