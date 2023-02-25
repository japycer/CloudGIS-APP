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
	// �����ǰ�� RubberBand
	void clearRubberBand();
	// ��д QgsMapTool ������ƶ��¼�
	void canvasMoveEvent(QgsMapMouseEvent * e) override;

	// ��д QgsMapTool ��������¼�
	void canvasPressEvent(QgsMapMouseEvent * e) override;

	void drawGeometryCallBack();

private:
	// ��ǰ���ڹ����� RubberBand
	QgsRubberBand* mpRubberBand = nullptr;
	// ��������ͼ�εĻ���
	QMap<QString, QgsRubberBand*>* mRubberBands = nullptr;
	// ������Ϣ�ĵ�������
	QDialog *inputFieldDlg = nullptr;
	// �༭��
	QLineEdit *currentEdit = nullptr;
	// ��ǰ���ڻ��Ƶ�geom����
	QgsWkbTypes::GeometryType mCurrentGeomType;
	DTemporyDrawerManager::DrawType mDrawType;
	// ��ǰ����ʹ�õ���ɫ
	QColor mCurrentDrawColor;
	// ��ǰ����geom������
	QString currentName;
	// ���ƾ��εĵ�һ����
	QgsPointXY mRectangleFirstPoint;
	// �Ƿ�ȥ���geom
	bool toAddGeometry = false;
	// �Ƿ��Ѿ������˵�һ����
	bool hasAssureFirstPoint = false;
	// ��¼�Ƿ����ڻ�����
	bool mIsDrawing;
	// ��������ת����transformer
	QgsCoordinateTransform tc;
};

