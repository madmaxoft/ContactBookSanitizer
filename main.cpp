#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtDebug>
#include "MainWindow.h"
#include "Session.h"
#include "ContactBook.h"
#include "Device.h"
#include "ExampleDevice.h"
#include "DeviceVcfFile.h"





/** Creates a new session and fills it with example data. */
std::unique_ptr<Session> makeExampleSession()
{
	std::unique_ptr<Session> session(new Session);
	session->addDevice(std::unique_ptr<Device>(new ExampleDevice));

	// Test: add a device based on a VCF file
	QJsonObject cfg;
	cfg["type"] = QString::fromUtf8("VcfFile");
	cfg["fileName"] = QString::fromUtf8("Contacts.vcf");
	session->addDevice(Device::createFromConfig(cfg));

	return session;
}





/** Loads the session from the specified filename.
Returns the loaded session, or nullptr if the loading failed. */
std::unique_ptr<Session> loadSessionFromFile(const QString & a_FileName)
{
	// Read the JSON from the file:
	QFile file(a_FileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		return nullptr;
	}
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(file.readAll(), &err);
	if (err.error == QJsonParseError::NoError)
	{
		qWarning() << "Cannot parse file " << a_FileName << ": " << err.errorString();
		return nullptr;
	}

	// Create devices based on the JSON:
	std::unique_ptr<Session> session(new Session);
	const auto & devices = doc.object()["devices"];
	for (const auto & device: devices.toArray())
	{
		auto dev = Device::createFromConfig(device.toObject());
		if (dev != nullptr)
		{
			session->addDevice(std::move(dev));
		}
	}
	if (session->getDevices().empty())
	{
		return nullptr;
	}

	return session;
}





/** Loads the session, either from the current folder ("portable app") or from the user's home.
If neither contain any reasonable data, creates an empty data file. */
std::unique_ptr<Session> loadSession()
{
	// First try loading from the current folder ("portable app"):
	auto session = loadSessionFromFile("ContactBookSanitizer.cbsSession");
	if (session != nullptr)
	{
		return session;
	}

	// No current folder data, load from user's home:
	// QDir::
	session = loadSessionFromFile(/* TODO */ "ContactBookSanitizer.cbsSession");
	if (session != nullptr)
	{
		return session;
	}

	// No data could be read, create a new example session file
	// TODO: First try writing it to the current folder; if it fails, we're installed, if it succeeds, we're portable
	session = makeExampleSession();
	return session;
}





int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w(loadSession());
	w.show();

	return a.exec();
}
