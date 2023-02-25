#pragma once
#include"qmap.h"
#include <qgsmaptooledit.h>
#include <qgsmapmouseevent.h>
#include <qgsrubberband.h>
#include <qgsmapcanvas.h>
#include<QDialog>
#include"DTemporyDrawerManager.h"

class QLineEdit;

class DMapToolTemporyDrawer :
	public QgsMapToolEdit
{
	Q_OBJECT
public:
	DMapToolTemporyDrawer(QgsMapCanvas* pMapCanvas);
	QString getJsonOfGeomByName(QString &name, QString *pyObjName = nullptr, int precison = 6);
	QList<QString> getJsonOfGeomByName(QList<QString> names, int precison = 6);
	int getCurrentDrawType();


signals:
	void succeedDrawAGeometry(QString &);

public slots:
	void changeDrawStyle(DTemporyDrawerManager::DrawType & drawType, QColor & drawColor);
	void changeItemVisible(QList<QString> &, bool);
	void deleteGeometry(QList<QString> &);

protected:
	// 清除当前的 RubberBand
	void clearRubberBand();
	// 重写 QgsMapTool 的鼠标移动事件
	void canvasMoveEvent(QgsMapMouseEvent * e) override;

	// 重写 QgsMapTool 的鼠标点击事件
	void canvasPressEvent(QgsMapMouseEvent * e) override;

	void drawGeometryCallBack();

private:
	// 当前正在工作的 RubberBand
	QgsRubberBand* mpRubberBand = nullptr;
	// 保存所有图形的画家
	QMap<QString, QgsRubberBand*>* mRubberBands = nullptr;
	// 输入信息的弹出窗口
	QDialog *inputFieldDlg = nullptr;
	// 编辑框
	QLineEdit *currentEdit = nullptr;
	// 当前正在绘制的geom类型
	QgsWkbTypes::GeometryType mCurrentGeomType;
	DTemporyDrawerManager::DrawType mDrawType;
	// 当前正在使用的颜色
	QColor mCurrentDrawColor;
	// 当前绘制geom的名字
	QString currentName;
	// 绘制矩形的第一个点
	QgsPointXY mRectangleFirstPoint;
	// 是否去添加geom
	bool toAddGeometry = false;
	// 是否已经绘制了第一个点
	bool hasAssureFirstPoint = false;
	// 记录是否正在绘制中
	bool mIsDrawing;
	// 进行坐标转换的transformer
	QgsCoordinateTransform tc;
};

