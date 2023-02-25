#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>     //SQL语句执行相关头文件

namespace Ui {
class Dialog;
}

class DLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DLoginDialog(bool *flag, QWidget *parent = 0);
    ~DLoginDialog();

private slots:
    void on_btn_register_clicked();

    void on_btn_login_clicked();

private:
    Ui::Dialog *ui;
    QSqlDatabase m_db;
    QSqlQuery* m_sql_query;
    bool *returnFlag;
    void openDatabese();
};

#endif // DIALOG_H
