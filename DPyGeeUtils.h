#pragma once
#include<QString>
#include"DPythonUtils.h"

class DPyGeeUtils
{
public:
	DPyGeeUtils() {};
	static void setTempVarName(QString&);		// ��ǰ���������ݼ�������
	static bool runString(QString& );	// ִ��һ���ַ���
	static QString getTempVarNameInfo();  //  ��ȡ�������ݼ���description
	static QString getTempVarName();	// ��ȡ��ǰ���ڹ��������ݼ�������
	static QString getEEObjValue(const QString &varName, bool *res = nullptr);		// ��ȡһ��gee�����info
	static QString getImgCollectionDateBetween(const QString &varName, bool *res = nullptr);		// ��ȡ�������ݼ���ʱ�䷶Χ
	static QString getImgCollectionHtmlDescription();		// ��ȡ���ݼ���html����
	static QString getImgCollectionBandsNames(QString &varName = QString());	// ��ȡ���ݼ������в����� eg. B1&B2&b3
	static bool releaseTempEEObj();		// �ͷ�eeObj��ռ�ڴ�
	static QString getEEPaletteStr(QString &url, int);
	static QString getLatestRunOut(bool);  // when fail, return ERROR
	static QString getEEObjType(QString&);

private:
	static QString tempVarName;	// ���������нϴ�����ı�����
	static QString mTempVarNameInfo;
};

