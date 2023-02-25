#pragma once
#include <qmap.h>
#include <qgsmaptooledit.h>
#include <qgsmapmouseevent.h>
#include <qgsrubberband.h>
#include <qgsmapcanvas.h>

class DMapToolDrawSample :
	public QgsMapToolEdit
{
	Q_OBJECT
public:
	DMapToolDrawSample(QgsMapCanvas* pMapCanvas);

signals:
	void succeedDrawAGeometry(int);
	void toSaveAsFile(QgsMapLayer *);
	//void toRunACommand(QString &);

public slots:
	void addANewClass(QString &className, QColor &color);
	void changeCurrentDrawClass(QString&);
	void changeDrawStyle(QString &className, QColor & drawColor);
	void removeAClass(QString &className);
	void removeAGeom(QString &, int);
	// 可见性设置
	void changeItemsVisible(QList<QString> &, bool);
	void changeItemVisible(QString&, bool);
	void changeAllItemVisible(QString &clsName, bool isVisible);
	// 导出样本
	void exportSamples(QString& exportTo, QString& exportName);

protected:
	// 清除当前的 RubberBand
	void clearRubberBand();
	// 重写 QgsMapTool 的鼠标移动事件
	void canvasMoveEvent(QgsMapMouseEvent * e) override;
	// 重写 QgsMapTool 的鼠标点击事件
	void canvasPressEvent(QgsMapMouseEvent * e) override;
	//	重写双击事件，用于退出绘制
	void canvasDoubleClickEvent(QgsMapMouseEvent *e) override;

	struct DSample
	{
		QgsRubberBand * pRubberhand = nullptr;
		int classID = -1;
		int ID = -1;
	};

private:
	// 当前正在工作的 RubberBand
	QgsCoordinateTransform mTc;
	QgsRubberBand* mpRubberBand = nullptr;
	// 保存所有分类的画家
	QMap<QString, QList<DSample *>*>* mRubberBands = nullptr;
	QMap<QString, QColor> mDrawColors;
	QMap<QString, int> mClassIds;
	// 当前正在绘制的geom类型
	QString currentClassName;
	QList<int> mClassIdStore;
	// 当前正在使用的颜色
	QColor mCurrentDrawColor;
	// 记录是否正在绘制中
	bool mIsDrawing;
	// 记录距离上次导出是否改变
	bool mHasModified = false;
	QgsVectorLayer* mCurrentExportLayer = nullptr;
};
