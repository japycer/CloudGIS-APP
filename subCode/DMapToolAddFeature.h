#pragma once

//#define _USE_MATH_DEFINES
#include <cmath>

#include <qgsmaptooledit.h>
#include <qgsmapmouseevent.h>
#include <qgsrubberband.h>
#include <qgsvectorlayer.h>
#include <qgsmapcanvas.h>
#include <QDebug>


class QDialog;

class DMapToolAddFeature : public QgsMapToolEdit
{

public:

	DMapToolAddFeature(QgsMapCanvas* pMapCanvas);

	// 清除当前的 RubberBand
	void clearRubberBand();

protected:

	// 重写 QgsMapTool 的鼠标移动事件
	void canvasMoveEvent(QgsMapMouseEvent * e) override;

	// 重写 QgsMapTool 的鼠标点击事件
	void canvasPressEvent(QgsMapMouseEvent * e) override;

	//判断是否满足添加feature条件
	virtual bool canAddFeature();

	void addFeatureCallback(QgsFeature &geom);

private:
	// 当前正在工作的 RubberBand
	QgsRubberBand* mpRubberBand = nullptr;
	QDialog *inputFieldDlg = nullptr;
	bool isFirstPaint = true;
	bool flagToAdd = false;
	// 记录是否正在绘制中
	bool mIsDrawing;
	QgsCoordinateTransform mtc;
};