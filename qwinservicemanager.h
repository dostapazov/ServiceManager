#ifndef QSERVICEMANAGERPRIVATE_H
#define QSERVICEMANAGERPRIVATE_H

#include <Windows.h>
#include <QObject>
#include <QList>

struct ServiceEntry
{
	QString name;
	QString display;
	SERVICE_STATUS_PROCESS status;
};

using QServiceList = QList<ServiceEntry>;

class QWinService
{
	Q_DISABLE_COPY(QWinService)
public:

	QWinService(QWinService&& src);
	~QWinService();
	bool isOpen();
	bool start(const QStringList& arguments = QStringList());
	bool stop();
	SERVICE_STATUS_PROCESS status();
	QString errorString();
	quint32 error();
	QString name();
	bool isRunning();
	bool isStartPending();
	bool isStopped();
	bool isStopPending();
	bool isPaused();
	bool isPausePending();
	uint pid();
	int  win32ExitCode();
	int  serviceExitCode();

private:
	QWinService() = default;
	void close();
	SC_HANDLE svcHandle = nullptr;
	QString svcName;
	quint32 errorCode = 0;
	friend class QWinServiceManager;
};

class QWinServiceManager : public QObject
{
	Q_OBJECT
public:
	explicit QWinServiceManager(QObject* parent = nullptr);
	~QWinServiceManager();
	bool open(bool readOnly = false);
	void close();
	bool isOpen();
	QString errorString();
	static QString errorString(quint32 errorCode);
	QServiceList list(bool svc32, bool driver = false);
	QServiceList list(quint32 flags);
	static QString serviceStateText(quint32 state);
	static QString serviceTypeText(quint32 type);
	static bool isHandleValid(SC_HANDLE handle);
	QWinService openService(QString svcName, bool allAccess);
signals:
private:
	void updateError();
	SC_HANDLE sc_handle = 0;
	quint32 errorCode = 0;

};

inline bool QWinService::isRunning()
{
	return status().dwCurrentState == SERVICE_RUNNING ;
}

inline bool QWinService::isStartPending()
{
	return status().dwCurrentState == SERVICE_START_PENDING;
}

inline bool QWinService::isStopped()
{
	return  status().dwCurrentState == SERVICE_STOPPED;
}

inline bool QWinService::isStopPending()
{
	return  status().dwCurrentState == SERVICE_STOPPED;
}

inline bool QWinService::isPaused()
{
	return status().dwCurrentState == SERVICE_PAUSED;
}

inline bool QWinService::isPausePending()
{
	return status().dwCurrentState == SERVICE_PAUSE_PENDING;
}

inline uint QWinService::pid()
{
	return status().dwProcessId;
}

inline int  QWinService::win32ExitCode()
{
	return status().dwWin32ExitCode;
}

inline int  QWinService::serviceExitCode()
{
	return status().dwServiceSpecificExitCode;
}

#endif // QSERVICEMANAGERPRIVATE_H
