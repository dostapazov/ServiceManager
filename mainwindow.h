#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qwinservicemanager.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private slots:
	void on_bRefresh_clicked();

	void on_svcTable_itemSelectionChanged();

	void on_bStart_clicked();

	void on_bStop_clicked();

private:
	void refreshServiceTable();
	void openManager();
	bool rdOnly = false;
	Ui::MainWindow* ui;
	QWinServiceManager svcManager;
};
#endif // MAINWINDOW_H
