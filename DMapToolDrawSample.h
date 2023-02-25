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
	// �ɼ�������
	void changeItemsVisible(QList<QString> &, bool);
	void changeItemVisible(QString&, bool);
	void changeAllItemVisible(QString &clsName, bool isVisible);
	// ��������
	void exportSamples(QString& exportTo, QString& exportName);

protected:
	// �����ǰ�� RubberBand
	void clearRubberBand();
	// ��д QgsMapTool ������ƶ��¼�
	void canvasMoveEvent(QgsMapMouseEvent * e) override;
	// ��д QgsMapTool ��������¼�
	void canvasPressEvent(QgsMapMouseEvent * e) override;
	//	��д˫���¼��������˳�����
	void canvasDoubleClickEvent(QgsMapMouseEvent *e) override;

	struct DSample
	{
		QgsRubberBand * pRubberhand = nullptr;
		int classID = -1;
		int ID = -1;
	};

private:
	// ��ǰ���ڹ����� RubberBand
	QgsCoordinateTransform mTc;
	QgsRubberBand* mpRubberBand = nullptr;
	// �������з���Ļ���
	QMap<QString, QList<DSample *>*>* mRubberBands = nullptr;
	QMap<QString, QColor> mDrawColors;
	QMap<QString, int> mClassIds;
	// ��ǰ���ڻ��Ƶ�geom����
	QString currentClassName;
	QList<int> mClassIdStore;
	// ��ǰ����ʹ�õ���ɫ
	QColor mCurrentDrawColor;
	// ��¼�Ƿ����ڻ�����
	bool mIsDrawing;
	// ��¼�����ϴε����Ƿ�ı�
	bool mHasModified = false;
	QgsVectorLayer* mCurrentExportLayer = nullptr;
};
