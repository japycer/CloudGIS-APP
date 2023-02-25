#pragma once
#include"qstring"

// 这是所有GEE Tool的基类，为一个抽象类
class DGeeToolBase
{

public:
	virtual QString getToolName() { return mToolName; };
	virtual QString getToolPath() { return mToolPath; };
	virtual int getMustInputParamCount() { return mMustInputParamCount; }

protected:
	// 检查输入内容的合法性
	virtual bool checkInputValidity() = 0;
	// 根据用户输入进行命令模板的填充
	virtual QString constructCmd() = 0;
	// 开始执行算法 (主要考虑多线程)
	virtual bool startToRun() = 0;
	// 工具执行之后的回调函数,如刷新地图、缩放等
	virtual void finishCallback() = 0;


	// 工具名称
	QString mToolName;
	// 工具所在路径
	QString mToolPath;
	// 必需输入的参数的数量
	int mMustInputParamCount;
	// Python 命令的模板
	QString pythonCmdTemplate;
};

