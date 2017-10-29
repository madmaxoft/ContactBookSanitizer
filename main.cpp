#include <QApplication>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtDebug>
#include <QMessageBox>
#include "MainWindow.h"
#include "Session.h"
#include "ContactBook.h"
#include "Device.h"
#include "ExampleDevice.h"
#include "DeviceVcfFile.h"
#include "Exceptions.h"





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





/** Loads the session, either from the current folder ("portable app") or from the user's home.
If neither contain any reasonable data, creates an empty data file. */
std::unique_ptr<Session> loadSession()
{
	// First try loading from the current folder ("portable app"):
	auto session = Session::loadFromFile("ContactBookSanitizer.cbsSession");
	if (session != nullptr)
	{
		return session;
	}

	// No current folder data, load from user's home:
	auto homePath = QDir::homePath() + "/";
	session = Session::loadFromFile(homePath + "ContactBookSanitizer.cbsSession");
	if (session != nullptr)
	{
		return session;
	}

	// No data could be read, create a new example session file
	qDebug() << "No session could be loaded. Creating a new session with example data.";
	session = makeExampleSession();

	// First try writing session to the current folder; if it fails, we're installed, if it succeeds, we're portable
	try
	{
		session->saveToFile("ContactBookSanitizer.cbsSession");
		qDebug() << "Detected a portable installation, session will be stored in current folder.";
		return session;
	}
	catch (const EFileError & exc)
	{
		qDebug()
			<< "Failed to write the session file to current folder: " << exc.m_Message
			<< "; this usually means we're running in installed mode. Will retry the home path.";
	}

	// We're installed, save to local home:
	try
	{
		session->saveToFile(homePath + "ContactBookSanitizer.cbsSession");
		qDebug() << "Detected an installed environment, session will be stored in home path: " << homePath;
		return session;
	}
	catch (const EFileError & exc)
	{
		QString msg("Failed to write the session file to both current folder and the home path: %1; session data will be lost after you close the app.");
		msg.arg(exc.m_Message);
		qWarning() << msg;
		QMessageBox::warning(nullptr, QString::fromUtf8("ContactBookSanitizer"), msg);
	}
	return session;
}





int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w(loadSession());
	w.show();

	return a.exec();
}
