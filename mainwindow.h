#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qwinservicemanager.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


constexpr int ColName = 0;
constexpr int ColDisplay = 1;
constexpr int ColState = 2;
constexpr int ColPid = 3;
constexpr int ColType = 4;

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
	void updateService(int row, const QString& svcName);
	void updateServiceStatus(int row, const SERVICE_STATUS_PROCESS& status );
	void refreshServiceTable();
	void openManager();
	bool rdOnly = false;
	Ui::MainWindow* ui;
	QWinServiceManager svcManager;
};
#endif // MAINWINDOW_H
