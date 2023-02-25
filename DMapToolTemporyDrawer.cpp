#include "DMapToolTemporyDrawer.h"
#include <QVBoxLayout>
#include<QLineEdit>
#include<QPushButton>
#include<qmessagebox.h>
#include"qgscoordinatetransform.h"
#include"qgscoordinatereferencesystem.h"
#include"qgsproject.h"


DMapToolTemporyDrawer::DMapToolTemporyDrawer(QgsMapCanvas* pMapCanvas):
	QgsMapToolEdit(pMapCanvas)
{
	mRubberBands = new QMap<QString, QgsRubberBand*>();
	mCurrentDrawColor = QColor("#ff0000");

	// 初始化坐标转换transformer
	//QgsGeometry geo = mpRubberBand->asGeometry();
	QgsCoordinateReferenceSystem crs_src("EPSG:3857");
	QgsCoordinateReferenceSystem dist_src("EPSG:4326");		//description >> "WGS 84"
	tc = QgsCoordinateTransform(crs_src, dist_src, QgsProject::instance());
	//geo.transform(tc);
}

// 画布鼠标移动事件
void DMapToolTemporyDrawer::canvasMoveEvent(QgsMapMouseEvent * e)
{
	if (!mpRubberBand || !mIsDrawing)
	{
		return;
	}
	mpRubberBand->movePoint(e->mapPoint());
}

// 画布鼠标按下事件
void DMapToolTemporyDrawer::canvasPressEvent(QgsMapMouseEvent * e)
{
	// 若当前 RubberBand 为空，则创建
	if (!mpRubberBand)
	{
		mpRubberBand = createRubberBand(mCurrentGeomType);
		mpRubberBand->setColor(mCurrentDrawColor);
	}

	// 左键按下
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		switch (mCurrentGeomType)
		{
		case QgsWkbTypes::PointGeometry:
			mpRubberBand->setIcon(QgsRubberBand::IconType::ICON_FULL_DIAMOND);
			mpRubberBand->setIconSize(10);
			//mpRubberBand->setSvgIcon(":/images/Mine/location.svg", QPoint(-800, -500));
			mpRubberBand->addPoint(e->mapPoint());
			drawGeometryCallBack();
			mRubberBands->insert(currentName, mpRubberBand);
			if (toAddGeometry)
			{
				mRubberBands->insert(currentName, mpRubberBand);
				// 删除当前 RubberBand
				mpRubberBand = nullptr;
				emit succeedDrawAGeometry(currentName);
			}
			else
				clearRubberBand();
			break;
		case QgsWkbTypes::LineGeometry:
		{
			mIsDrawing = true;
			mpRubberBand->addPoint(e->mapPoint());
		}
		break;
		case QgsWkbTypes::PolygonGeometry:
		{
			if (mDrawType == DTemporyDrawerManager::DrawType::MultiPolygon)
			{
				mIsDrawing = true;
				mpRubberBand->addPoint(e->mapPoint());
			}
			else
			{
				mIsDrawing = true;
				mpRubberBand->removePoint(0);
				mpRubberBand->addPoint(e->mapPoint());
				mRectangleFirstPoint = e->mapPoint();
			}
		}
		break;
		case QgsWkbTypes::UnknownGeometry:
		case QgsWkbTypes::NullGeometry:
		default:
			break;
		}
	}
	// 右键按下
	else if (e->button() == Qt::MouseButton::RightButton)
	{
		// 停止绘制
		mIsDrawing = false;
		switch (mCurrentGeomType)
		{
		case QgsWkbTypes::LineGeometry:
			// 若顶点数不小于 2且满足可添加feature条件，则创建新的 QgsFeature，加入图层中
			if (mpRubberBand->numberOfVertices() >= 2)
			{
				drawGeometryCallBack();

				if (toAddGeometry)
				{
					mRubberBands->insert(currentName, mpRubberBand);
					// 删除当前 RubberBand
					mpRubberBand = nullptr;
					emit succeedDrawAGeometry(currentName);
				}
				else
					clearRubberBand();
			}
			break;
		case QgsWkbTypes::PolygonGeometry:
			if (mDrawType == DTemporyDrawerManager::DrawType::MultiPolygon)
			{
				// 若顶点数不小于 3且满足可添加feature条件，则创建新的 QgsFeature，加入图层中
				if (mpRubberBand->numberOfVertices() >= 3)
				{
					// 停止绘制
					mIsDrawing = false;
					drawGeometryCallBack();

					if (toAddGeometry)
					{
						mRubberBands->insert(currentName, mpRubberBand);
						// 删除当前 RubberBand
						mpRubberBand = nullptr;
						emit succeedDrawAGeometry(currentName);
					}
					else
						clearRubberBand();
				}
			}
			else
			{
				QgsPointXY leftBottom(mRectangleFirstPoint.x(), e->mapPoint().y());
				QgsPointXY rightTop(e->mapPoint().x(), mRectangleFirstPoint.y());
				mpRubberBand->removePoint(0);
				mpRubberBand->removePoint(0);
				mpRubberBand->addPoint(mRectangleFirstPoint);
				mpRubberBand->addPoint(leftBottom);
				mpRubberBand->addPoint(e->mapPoint());
				mpRubberBand->addPoint(rightTop);

				// 停止绘制
				mIsDrawing = false;
				drawGeometryCallBack();
				if (toAddGeometry)
				{
					mRubberBands->insert(currentName, mpRubberBand);
					mpRubberBand = nullptr;
					emit succeedDrawAGeometry(currentName);
				}
				else
					clearRubberBand();
			}
			break;
		case QgsWkbTypes::PointGeometry:
		case QgsWkbTypes::UnknownGeometry:
		case QgsWkbTypes::NullGeometry:
			break;
		default:
			break;
		}
	}
}

// 清除之前的rubber
void DMapToolTemporyDrawer::clearRubberBand()
{
	if (!mpRubberBand)
	{
		return;
	}
	delete mpRubberBand;
	mpRubberBand = nullptr;
}


// 绘制geom的回调函数
void DMapToolTemporyDrawer::drawGeometryCallBack()
{
	toAddGeometry = false;
	inputFieldDlg = new QDialog();
	QVBoxLayout *vLayot = new QVBoxLayout();
	vLayot->setSpacing(8);

	currentEdit = new QLineEdit();
	QRegExp regVar("[a-zA-Z_]+\\w*");
	currentEdit->setValidator(new QRegExpValidator(regVar, this));

	vLayot->addWidget(currentEdit);
	QLabel *tip = new QLabel("ok?");
	vLayot->addWidget(tip);
	
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
		bool hasExisted = mRubberBands->contains(currentEdit->text());
		if (hasExisted)
		{
			tip->setText("Error, has Existed !");
			return;
		}
		currentName = currentEdit->text().simplified();
		if (currentName.isEmpty())
		{
			tip->setText("Error, name is Empty !");
			return;
		}
		toAddGeometry = true;
		currentEdit = nullptr;
		inputFieldDlg->close();
		delete inputFieldDlg;
		inputFieldDlg = nullptr;
		return;
	});

	connect(cancelBtn, &QPushButton::clicked, inputFieldDlg, &QDialog::close);
	connect(cancelBtn, &QPushButton::clicked, inputFieldDlg, &QDialog::deleteLater);

	inputFieldDlg->setLayout(vLayot);
	inputFieldDlg->setFixedSize(inputFieldDlg->sizeHint());
	inputFieldDlg->exec();
}


// 一个改变绘制颜色和绘制geom类型的槽函数
void DMapToolTemporyDrawer::changeDrawStyle(DTemporyDrawerManager::DrawType &drawType, QColor & drawColor)
{
	switch (drawType)
	{
	case DTemporyDrawerManager::Point:
		mCurrentGeomType = QgsWkbTypes::GeometryType::PointGeometry;
		break;
	case DTemporyDrawerManager::LineString:
		mCurrentGeomType = QgsWkbTypes::GeometryType::LineGeometry;
		break;
	case DTemporyDrawerManager::MultiPolygon:
		mCurrentGeomType = QgsWkbTypes::GeometryType::PolygonGeometry;
		break;
	case DTemporyDrawerManager::Rectangle:
		mCurrentGeomType = QgsWkbTypes::GeometryType::PolygonGeometry;
		break;
	case DTemporyDrawerManager::UnKnown:
		mCurrentGeomType = QgsWkbTypes::GeometryType::PolygonGeometry;
		break;
	default:
		break;
	}
	mDrawType = drawType;
	mCurrentDrawColor = drawColor;
}


void DMapToolTemporyDrawer::changeItemVisible(QList<QString> & names, bool isVisible)
{
	for (int i = 0; i < names.count(); i++)
	{
		QString name = names.at(i);
		mRubberBands->value(name)->setVisible(isVisible);
	}
}


void DMapToolTemporyDrawer::deleteGeometry(QList<QString> & names)
{
	for (int i = 0; i < names.count(); i++)
	{
		QString name = names.at(i);
		delete mRubberBands->value(name);
		mRubberBands->remove(name);
	}
}

// 获得某个geom的json字符串
QString DMapToolTemporyDrawer::getJsonOfGeomByName(QString &name, QString *pyObjName,int precison)
{
	if(!(mRubberBands->contains(name))) return QString();
	QgsGeometry geom = mRubberBands->value(name)->asGeometry();
	
	//QString descrip = tc.sourceCrs().description();
	//if (descrip.compare("WGS 84", Qt::CaseInsensitive) == 0)
	//	return geom.asJson(precison);
	geom.transform(tc);
	geom.convertToSingleType();

	auto geoType = geom.type();
	QString geomType;
	QString varPrefix;
	switch (geoType)
	{
	case QgsWkbTypes::PointGeometry:
		geomType = QString("Point");
		varPrefix = QString("P");
		break;
	case QgsWkbTypes::LineGeometry:
		geomType = QString("LineString");
		varPrefix = QString("L");
		break;
	case QgsWkbTypes::PolygonGeometry:
		geomType = QString("Polygon");
		varPrefix = QString("Poly");
		break;
	case QgsWkbTypes::UnknownGeometry:
	case QgsWkbTypes::NullGeometry:
	default:
		return QString();
		break;
	}

	QString varName = QString("%1_%2").arg(varPrefix, name.simplified());
	QString res = geom.asJson(precison);
	int indexStart = res.indexOf('[');
	int endStart = res.lastIndexOf(']');
	res = res.mid(indexStart, endStart - indexStart + 1);
	
	if(pyObjName)
		*pyObjName = varName;
	return QString("%1=ee.Geometry.%2(%3)").arg(varName,geomType,res);
	//QMessageBox::warning(nullptr, QString("JSON"), geo.asJson(6), QMessageBox::Ok);
}


// 获得某些geom的json字符串
QList<QString> DMapToolTemporyDrawer::getJsonOfGeomByName(QList<QString> names, int precison)
{
	QList<QString> jsons;
	for (int i = 0; i < names.count(); i++)
	{
		QString name = names.at(i);
		if (mRubberBands->contains(name))
		{
			jsons.append(getJsonOfGeomByName(name));
		}
	}
	return jsons;
}

int DMapToolTemporyDrawer::getCurrentDrawType()
{
	int res = -1;
	switch (mCurrentGeomType)
	{
	case QgsWkbTypes::PointGeometry:
		res = 1;
		break;
	case QgsWkbTypes::LineGeometry:
		res = 2;
		break;
	case QgsWkbTypes::PolygonGeometry:
		res = 3;
		break;
	case QgsWkbTypes::UnknownGeometry:
		res = 4;
		break;
	case QgsWkbTypes::NullGeometry:
		res = 5;
		break;
	default:
		res = 0;
		break;
	}
	return res;
}