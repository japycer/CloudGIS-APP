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

	// �����ǰ�� RubberBand
	void clearRubberBand();

protected:

	// ��д QgsMapTool ������ƶ��¼�
	void canvasMoveEvent(QgsMapMouseEvent * e) override;

	// ��д QgsMapTool ��������¼�
	void canvasPressEvent(QgsMapMouseEvent * e) override;

	//�ж��Ƿ��������feature����
	virtual bool canAddFeature();

	void addFeatureCallback(QgsFeature &geom);

private:
	// ��ǰ���ڹ����� RubberBand
	QgsRubberBand* mpRubberBand = nullptr;
	QDialog *inputFieldDlg = nullptr;
	bool isFirstPaint = true;
	bool flagToAdd = false;
	// ��¼�Ƿ����ڻ�����
	bool mIsDrawing;
	QgsCoordinateTransform mtc;
};