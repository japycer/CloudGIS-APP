#include "DGisApp.h"
#include"qgsapplication.h"
#include "qbytearray.h"
#include "DEnums.h"
#include "DLoginDialog.h"

int main(int argc, char *argv[])
{
	QgsApplication a(argc, argv, true);

	// 先进行登录窗口初始化
	DEnums::User usr = DEnums::User{ false, "UnKnown", "shuangyu", "****.gmail.com" };
	DLoginDialog loginDialog(&usr, nullptr);
	loginDialog.exec();
	if(usr.strName.compare("UnKnown")==0)
		return a.exec();
	else {
		QSettings setting(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
		QByteArray  proj_lib = setting.value("/setting/qgis/PROJ_LIB").toByteArray();
		QString installPath = setting.value("/setting/qgis/INSTALL_PATH").toString();
		//QByteArray pythonHome = setting.value("/setting/python/PYTHON_HOMEPATH").toByteArray();
		qputenv("PROJ_LIB", proj_lib);
		//qputenv("PYTHONHOME", pythonHome);

		QgsApplication::setPrefixPath(installPath + "/apps/qgis", true);
		QgsApplication::setPluginPath(installPath + "/apps/qgis/plugins");
		QgsApplication::initQgis();    //初始化QGIS应用
		DGisApp w(&usr);
		w.show();
		return a.exec();
	}
	return -1;
}
