#pragma once
#include"qstring"

// ��������GEE Tool�Ļ��࣬Ϊһ��������
class DGeeToolBase
{

public:
	virtual QString getToolName() { return mToolName; };
	virtual QString getToolPath() { return mToolPath; };
	virtual int getMustInputParamCount() { return mMustInputParamCount; }

protected:
	// ����������ݵĺϷ���
	virtual bool checkInputValidity() = 0;
	// �����û������������ģ������
	virtual QString constructCmd() = 0;
	// ��ʼִ���㷨 (��Ҫ���Ƕ��߳�)
	virtual bool startToRun() = 0;
	// ����ִ��֮��Ļص�����,��ˢ�µ�ͼ�����ŵ�
	virtual void finishCallback() = 0;


	// ��������
	QString mToolName;
	// ��������·��
	QString mToolPath;
	// ��������Ĳ���������
	int mMustInputParamCount;
	// Python �����ģ��
	QString pythonCmdTemplate;
};

