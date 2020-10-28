#include "qwinservicemanager.h"
#include <QThread>


#pragma comment(lib, "advapi32.lib")


QWinServiceManager::QWinServiceManager(QObject* parent) : QObject(parent)
{
}

QWinServiceManager::~QWinServiceManager()
{
	close();
}

bool QWinServiceManager::open(bool readOnly)
{
	close();
	DWORD access = GENERIC_READ | (readOnly ? 0 : SC_MANAGER_ALL_ACCESS);
	sc_handle = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, access);
	updateError();
	return isOpen();

}

void QWinServiceManager::updateError()
{
	errorCode = GetLastError();
}


void QWinServiceManager::close()
{
	if (isOpen())
	{
		CloseServiceHandle(sc_handle);
	}
}

bool QWinServiceManager::isHandleValid(SC_HANDLE handle)
{
	return handle && handle != INVALID_HANDLE_VALUE;
}
bool QWinServiceManager::isOpen()
{
	return  isHandleValid(sc_handle);
}


QString QWinServiceManager::errorString()
{
	return errorString(errorCode);
}
QString QWinServiceManager::errorString(quint32 errorCode)
{
	QString errorText;
	if (errorCode)
	{
		wchar_t* messageBuffer = nullptr;
		DWORD msgSize = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
									   errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL	);

		errorText = QString::fromWCharArray(messageBuffer, msgSize);
		if (messageBuffer)
			LocalFree(messageBuffer);
	}
	return errorText;

}


QStringList QWinServiceManager::list(bool svc32,  bool driver)
{
	quint32 serviceType = (svc32 ? SERVICE_WIN32 : 0) | (driver ? SERVICE_DRIVER : 0);
	return list(serviceType );
}

QStringList QWinServiceManager::list(quint32 serviceType)
{
	QStringList svcList;
	if (isOpen())
	{

		SC_ENUM_TYPE infoLevel = SC_ENUM_PROCESS_INFO;
		DWORD state = SERVICE_STATE_ALL;
		char buffer[0x10000];
		DWORD bytesNeed = 0;

		DWORD resumeHandle = 0;
		do
		{
			DWORD svcCount = 0;
			EnumServicesStatusEx(sc_handle, infoLevel, serviceType, state,
								 LPBYTE(buffer), sizeof(buffer), &bytesNeed,
								 &svcCount, &resumeHandle, nullptr);
			updateError();
			if (svcCount )
			{
				LPENUM_SERVICE_STATUS_PROCESS begin = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(buffer);
				LPENUM_SERVICE_STATUS_PROCESS end = begin + svcCount;
				while (begin < end)
				{

					QString name = QString::fromWCharArray(begin->lpServiceName);
					svcList.append(name);
					++begin;
				}
			}
		}
		while ( resumeHandle);

	}
	return svcList;
}

QString QWinServiceManager::serviceTypeText(quint32 type)
{
	switch (type)
	{
		case SERVICE_FILE_SYSTEM_DRIVER :
			return "File system driver";
		case SERVICE_KERNEL_DRIVER :
			return "Kernel driver";
		case SERVICE_WIN32_OWN_PROCESS :
			return "Own process";
		case SERVICE_WIN32_SHARE_PROCESS :
			return "Shares a process with other services";
		default:
			return "Unknown service type";
	}
}
QString QWinServiceManager::serviceStateText(quint32 state)
{
	static const char* stateText[] =
	{
		"",
		"Stopped",
		"Start pending",
		"Stop pending",
		"Running",
		"Continue pending",
		"Pause pending",
		"Paused"
	};
	return QString(stateText[qMin(state, quint32(SERVICE_PAUSED))]);
}

QWinService QWinServiceManager::openService(QString svcName, bool allAccess)
{
	QWinService svc;

	svc.svcName   = svcName;
	SetLastError(0);
	DWORD access = GENERIC_READ;
	if (allAccess)
		access |=  GENERIC_WRITE | SERVICE_ALL_ACCESS;
	svc.svcHandle = OpenService(sc_handle, svcName.toStdWString().data(), access);
	svc.errorCode = GetLastError();
	return svc;
}


bool QWinService::isOpen()
{
	return QWinServiceManager::isHandleValid(svcHandle);
}

QWinService::QWinService(QWinService&& src)
{
	close();
	errorCode = 0;
	svcHandle = src.svcHandle;
	src.svcHandle = 0;
	svcName = std::move(src.svcName);
}

QWinService::~QWinService()
{
	close();
}

wchar_t** makeArguments(const QStringList& args)
{
	wchar_t** argv = nullptr;
	if (args.size())
	{
		argv = new wchar_t* [args.size() + 1];
		argv[args.size()] = nullptr;
		int argNumber = 0;
		for (const auto& argument : args)
		{
			argv[argNumber] = new wchar_t[argument.length() + 1];
			wcscpy_s(argv[argNumber], argument.length(), argument.toStdWString().data());
			++argNumber;
		}
	}
	return argv;
}

void freeArguments(wchar_t* argv[])
{
	if (argv)
	{
		wchar_t** current = argv;
		while (*current)
		{
			delete [] *current;
			++current;
		}
		delete [] argv;
	}
}

bool QWinService::start(const QStringList& arguments)
{
	bool ret = false;
	if (isOpen())
	{
		wchar_t** argv = makeArguments(arguments);
		ret = StartService(this->svcHandle, arguments.size(), (const wchar_t**)argv);
		errorCode = GetLastError();
		freeArguments(argv);
	}
	return ret;
}


bool QWinService::stop()
{
	bool ret = false;
	if (isOpen())
	{
		SERVICE_STATUS status;
		ret = ControlService(svcHandle, SERVICE_CONTROL_STOP, &status);
		errorCode = GetLastError();
	}
	return ret;
}

void QWinService::close()
{
	if (isOpen())
		CloseServiceHandle(svcHandle);
	svcHandle = 0;
}

QString QWinService::errorString()
{
	return QWinServiceManager::errorString(errorCode);
}

quint32 QWinService::error()
{
	return errorCode;
}
QString QWinService::name()
{
	return svcName;
}

SERVICE_STATUS_PROCESS QWinService::status()
{
	SERVICE_STATUS_PROCESS status;
	memset(&status, 0, sizeof (status));
	if (isOpen())
	{
		DWORD bytesNeed = 0;
		QueryServiceStatusEx(svcHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&status, sizeof(status), &bytesNeed);
		errorCode = GetLastError();
	}
	return status;
}

LPQUERY_SERVICE_CONFIG  QWinService::config()
{
	LPQUERY_SERVICE_CONFIG cfg = nullptr;
	DWORD bytesNeed = 0;
	QueryServiceConfig(svcHandle, nullptr, 0, &bytesNeed);
	if (bytesNeed)
	{
		cfg = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(new char [bytesNeed]);
		if (cfg)
		{
			QueryServiceConfig(svcHandle, cfg, bytesNeed, &bytesNeed);
		}
	}
	errorCode = GetLastError();
	return cfg;
}

QString QWinService::display()
{
	std::unique_ptr<QUERY_SERVICE_CONFIG> cfg(config());
	return cfg && cfg->lpDisplayName ? QString::fromWCharArray( cfg->lpDisplayName ) : QString();
}
