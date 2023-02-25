#pragma once

#include <QWidget>
#include "ui_DToolBox.h"

class DToolBox : public QWidget
{
	Q_OBJECT

public:
	DToolBox(QWidget *parent = Q_NULLPTR);
	~DToolBox();

	enum ToolType
	{
		Vector,
		Raster,
		Analysis,
		Hydrology,
		Landsat,
		Python,
		Export,
		Others
	};

public slots:
	void addOneTool(ToolType, const QString&, const QString&);


signals:
	// 工具被点击时触发的函数
	void toolClicked(ToolType, const QString&);

private:
	Ui::DToolBox ui;

private:
	void initGui();
	void bingSignalsSlots();
	void loadToolsFromConfigFile();
};
