#pragma once
#include<QString>
#include"DPythonUtils.h"

class DPyGeeUtils
{
public:
	DPyGeeUtils() {};
	static void setTempVarName(QString&);		// 当前工作的数据集变量名
	static bool runString(QString& );	// 执行一个字符串
	static QString getTempVarNameInfo();  //  获取当球数据集的description
	static QString getTempVarName();	// 获取当前正在工作的数据集变量名
	static QString getEEObjValue(const QString &varName, bool *res = nullptr);		// 获取一个gee对象的info
	static QString getImgCollectionDateBetween(const QString &varName, bool *res = nullptr);		// 获取介于数据集的时间范围
	static QString getImgCollectionHtmlDescription();		// 获取数据集的html描述
	static QString getImgCollectionBandsNames(QString &varName = QString());	// 获取数据集的所有波段名 eg. B1&B2&b3
	static bool releaseTempEEObj();		// 释放eeObj所占内存
	static QString getEEPaletteStr(QString &url, int);
	static QString getLatestRunOut(bool);  // when fail, return ERROR
	static QString getEEObjType(QString&);

private:
	static QString tempVarName;	// 保存运行中较大变量的变量名
	static QString mTempVarNameInfo;
};

