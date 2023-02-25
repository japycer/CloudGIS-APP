#include "DMapToolAddFeature.h"
#include<QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include<QLabel>
#include<QLineEdit>
#include<QPushButton>
#include"qgsvectorlayerutils.h"
#include"qgscoordinatetransform.h"
#include"qgscoordinatereferencesystem.h"

DMapToolAddFeature::DMapToolAddFeature(QgsMapCanvas * pMapCanvas) :
	QgsMapToolEdit(pMapCanvas),
	mIsDrawing(false)
{
	QgsCoordinateReferenceSystem crs_src("EPSG:3857");
	QgsCoordinateReferenceSystem dist_src("EPSG:4326");		//description >> "WGS 84"
	mtc = QgsCoordinateTransform(crs_src, dist_src, QgsProject::instance());
}

// 画布鼠标移动事件
void DMapToolAddFeature::canvasMoveEvent(QgsMapMouseEvent * e)
{
	if (!mpRubberBand || !mIsDrawing)
	{
		return;
	}
	mpRubberBand->movePoint(e->mapPoint());
}

// 画布鼠标按下事件
void DMapToolAddFeature::canvasPressEvent(QgsMapMouseEvent * e)
{
	QgsVectorLayer *layer = currentVectorLayer();
	if (layer == nullptr)
		return;
	
	QgsWkbTypes::GeometryType layerType = layer->geometryType();
	// 若当前 RubberBand 为空，则创建
	if (!mpRubberBand)
	{
		switch (layerType)
		{
		case QgsWkbTypes::PointGeometry:
			mpRubberBand = createRubberBand(QgsWkbTypes::GeometryType::PointGeometry);
			break;
		case QgsWkbTypes::LineGeometry:
			mpRubberBand = createRubberBand(QgsWkbTypes::GeometryType::LineGeometry);
			break;
		case QgsWkbTypes::PolygonGeometry:
			mpRubberBand = createRubberBand(QgsWkbTypes::GeometryType::PolygonGeometry);
			break;
		case QgsWkbTypes::UnknownGeometry:
			break;
		case QgsWkbTypes::NullGeometry:
			break;
		default:
			break;
		}
		if (!mpRubberBand) return;
		//mpRubberBand = createRubberBand(QgsWkbTypes::GeometryType::PolygonGeometry);
	}

	// 左键按下
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		switch (layerType)
		{
		case QgsWkbTypes::PointGeometry:
			if (canAddFeature()) {
				mpRubberBand->addPoint(e->mapPoint());
				QgsFeature f;
				QgsGeometry geom = mpRubberBand->asGeometry();
				geom.transform(mtc);
				f.setGeometry(geom);
				currentVectorLayer()->addFeature(f);
				canvas()->refresh();
				mIsDrawing = true;
			}
			break;
		case QgsWkbTypes::LineGeometry:
		case QgsWkbTypes::PolygonGeometry:
		{
			mIsDrawing = true;
			mpRubberBand->addPoint(e->mapPoint());
		}
			break;
		case QgsWkbTypes::UnknownGeometry:
			break;
		case QgsWkbTypes::NullGeometry:
			break;
		default:
			break;
		}
	}
	// 右键按下
	else if (e->button() == Qt::MouseButton::RightButton)
	{
		// 停止绘制
		mIsDrawing = false;
		switch (layerType)
		{
		case QgsWkbTypes::LineGeometry:
			// 若顶点数不小于 2且满足可添加feature条件，则创建新的 QgsFeature，加入图层中
			if (mpRubberBand->numberOfVertices() >= 2 && canAddFeature())
			{
				QgsFields fields = layer->fields();
				QgsGeometry geom = mpRubberBand->asGeometry();
				geom.transform(mtc);
				geom.convertToSingleType();
				QgsFeature feature = QgsVectorLayerUtils::createFeature(layer, geom);;
				currentVectorLayer()->addFeature(feature);
				if (isFirstPaint)
				{
					//canvas()->zoomToFullExtent();
					isFirstPaint = false;
				}
				else
					canvas()->refresh();
				//addFeatureCallback(geo);
			}
			break;
		case QgsWkbTypes::PolygonGeometry:
			// 若顶点数不小于 3且满足可添加feature条件，则创建新的 QgsFeature，加入图层中
			if (mpRubberBand->numberOfVertices() >= 3 && canAddFeature())
			{
				flagToAdd = false;
				QgsGeometry geom = mpRubberBand->asGeometry();
				geom.transform(mtc);
				geom.convertToSingleType();
				QgsFeature feature = QgsVectorLayerUtils::createFeature(layer, geom);
				addFeatureCallback(feature);
				if (flagToAdd)
				{
					layer->addFeature(feature);
					if (isFirstPaint)
					{
						//canvas()->zoomToFullExtent();
						isFirstPaint = false;
					}
					//else
					canvas()->refresh();
				}
				//addFeatureCallback(geo);
			}
			break;
		case QgsWkbTypes::PointGeometry:
		case QgsWkbTypes::UnknownGeometry:
		case QgsWkbTypes::NullGeometry:
			break;
		default:
			break;
		}
		// 删除当前 RubberBand
		clearRubberBand();
	}
}


// 判断图层是否满足插入条件
bool DMapToolAddFeature::canAddFeature()
{
	if(currentVectorLayer()==nullptr)
		return false;
	if (!currentVectorLayer()->isEditable())
		return false;
	QgsVectorDataProvider* provider = currentVectorLayer()->dataProvider();
	if (!(provider->capabilities() & QgsVectorDataProvider::AddFeatures))
		return false;
	return true;
}


// 添加feature的回调函数，负责录入一些属性信息及撤销操作
void DMapToolAddFeature::addFeatureCallback(QgsFeature &geom)
{
	QgsFields fields = currentVectorLayer()->fields();
	int fieldsCount = fields.count();
	if (fieldsCount < 0)
		return;

	inputFieldDlg = new QDialog();
	QVBoxLayout *vLayot = new QVBoxLayout();
	vLayot->setSpacing(8);
	QList<QLineEdit*> lineEditList;
	for (size_t i = 0; i < fieldsCount; i++)
	{
		QHBoxLayout *hLayot = new QHBoxLayout();
		QLabel *label = new QLabel();
		label->setText(fields.at(i).name());
		QLineEdit *lineEdit = new QLineEdit();
		lineEditList.append(lineEdit);
		hLayot->addWidget(label);
		hLayot->addWidget(lineEdit);
		vLayot->addLayout(hLayot);
	}
	QHBoxLayout *hLayotB = new QHBoxLayout();
	QPushButton *okBtn = new QPushButton();
	okBtn->setText("OK");
	QPushButton *cancelBtn = new QPushButton();
	cancelBtn->setText("Cancel");
	hLayotB->addStretch();
	hLayotB->addWidget(okBtn);
	hLayotB->addWidget(cancelBtn);
	hLayotB->addStretch();
	vLayot->addLayout(hLayotB);

	connect(okBtn, &QPushButton::clicked, [&] {
		for (size_t i = 0; i < fieldsCount; i++)
		{
			geom.setAttribute(fields.at(i).name(), lineEditList.at(i)->text());
		}
		flagToAdd = true;
		inputFieldDlg->close();
		delete inputFieldDlg;
		lineEditList.clear();
	});
	connect(cancelBtn, &QPushButton::clicked, inputFieldDlg, &QDialog::close);
	connect(cancelBtn, &QPushButton::clicked, inputFieldDlg, &QDialog::deleteLater);

	inputFieldDlg->setLayout(vLayot);
	inputFieldDlg->setFixedSize(inputFieldDlg->sizeHint());
	inputFieldDlg->exec();
}


// 清除之前的rubber
void DMapToolAddFeature::clearRubberBand()
{
	if (!mpRubberBand)
	{
		return;
	}
	delete mpRubberBand;
	mpRubberBand = nullptr;
}
