#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	openManager();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openManager()
{
	if (!svcManager.open(false))
		rdOnly = svcManager.open(true);
	else
		rdOnly = false;
	QString titleStr;
	if (svcManager.isOpen())
		titleStr = QString("Service manager open in %1 mode").arg(rdOnly ? "read only" : "normal");
	else
		titleStr = "Error open service manager" + svcManager.errorString();
	setWindowTitle(titleStr);
}

void ensureRowColumns(QTableWidget* tw, int row, int columns)
{
	if (tw->rowCount() <= row)
		tw->setRowCount(row + 1);

	for (int i = 0; i < columns; i++)
	{
		if (!tw->item(row, i))
			tw->setItem(row, i, new QTableWidgetItem);
	}
}

void MainWindow::updateServiceStatus(int row, const SERVICE_STATUS_PROCESS& status )
{
	auto iState = ui->svcTable->item(row, ColState);
	auto iPid = ui->svcTable->item(row, ColPid);
	auto iType = ui->svcTable->item(row, ColType);
	iState->setText(QWinServiceManager::serviceStateText(status.dwCurrentState));
	iPid->setText(status.dwProcessId ? QString::number(status.dwProcessId, 16) : QString());
	iType->setText(QWinServiceManager::serviceTypeText(status.dwServiceType));
}

void MainWindow::updateService(int row, const QString& svcName)
{
	auto iDisplay = ui->svcTable->item(row, ColDisplay);
	QWinService svc = this->svcManager.openService(svcName, false);
	if (svc.isOpen())
	{
		iDisplay->setText(svc.display());
		const auto status = svc.status();
		updateServiceStatus(row, status);
	}
}

void MainWindow::refreshServiceTable()
{
	ui->svcTable->clear();
	auto list = svcManager.list(true);
	ui->svcTable->setRowCount(list.size());
	int row = 0;
	for (const auto& name : list )
	{
		ensureRowColumns(ui->svcTable, row, 5);
		auto iName  = ui->svcTable->item(row, ColName);
		iName->setText(name);
		updateService(row, name);
		++row;
	}
}

void MainWindow::on_bRefresh_clicked()
{
	refreshServiceTable();
}

void MainWindow::on_svcTable_itemSelectionChanged()
{
	auto item = ui->svcTable->item(ui->svcTable->currentRow(), 0);
	if (item)
	{
		QString svcName = item->text();
		QWinService svc = svcManager.openService(svcName, !rdOnly);
		ui->bStart->setDisabled(svc.isRunning());
		ui->bStop->setEnabled(!svc.isStopped());
		updateServiceStatus(ui->svcTable->currentRow(), svc.status());
	}
}

void MainWindow::on_bStart_clicked()
{
	auto item = ui->svcTable->item(ui->svcTable->currentRow(), 0);
	if (item)
	{
		QString svcName = item->text();
		QWinService svc = svcManager.openService(svcName, !rdOnly);
		svc.start();
		QStatusBar* statusBar = this->statusBar();
		statusBar->showMessage(svc.errorString(), 5000);
		on_svcTable_itemSelectionChanged();
	}
}

void MainWindow::on_bStop_clicked()
{
	auto item = ui->svcTable->item(ui->svcTable->currentRow(), 0);
	if (item)
	{
		QString svcName = item->text();
		QWinService svc = svcManager.openService(svcName, !rdOnly);
		svc.stop();
		QStatusBar* statusBar = this->statusBar();
		statusBar->showMessage(svc.errorString(), 5000);
		on_svcTable_itemSelectionChanged();

	}
}
