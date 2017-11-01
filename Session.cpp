#include "Session.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include "Device.h"
#include "Exceptions.h"





Session::Session():
	Super(nullptr)
{

}





void Session::addDevice(std::unique_ptr<Device> a_Device)
{
	auto dev = a_Device.get();
	m_Devices.push_back(std::move(a_Device));
	emit addedDevice(dev);
}





void Session::delDevice(const Device * a_Device)
{
	for (auto itr = m_Devices.begin(), end = m_Devices.end(); itr != end; ++itr)
	{
		if (itr->get() == a_Device)
		{
			emit removingDevice(a_Device);
			m_Devices.erase(itr);
			return;
		}
	}
}





void Session::startDevices()
{
	for (auto & dev: m_Devices)
	{
		dev->start();
	}
}





void Session::stopDevices()
{
	for (auto & dev: m_Devices)
	{
		dev->stop();
	}
}





void Session::setFileName(const QString & a_FileName)
{
	m_FileName = a_FileName;
}





std::unique_ptr<Session> Session::loadFromFile(const QString & a_FileName)
{
	// Read the JSON from the file:
	QFile file(a_FileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		return nullptr;
	}
	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(file.readAll(), &err);
	if (err.error != QJsonParseError::NoError)
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
		qWarning() << "The session in file " << a_FileName << " contains no devices, aborting load.";
		return nullptr;
	}

	session->setFileName(a_FileName);
	return session;
}





void Session::saveToFile()
{
	// Open the file for writing:
	if (m_FileName.isEmpty())
	{
		throw EFileError(
			__FILE__, __LINE__,
			m_FileName,
			tr("Cannot save to an empty file name")
		);
	}
	QFile f(m_FileName);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		throw EFileError(
			__FILE__, __LINE__,
			m_FileName,
			tr("Cannot open file for writing: %1").arg(f.errorString())
		);
	}

	// Serialize the session data:
	QJsonObject json;
	QJsonArray devices;
	for (const auto & dev: m_Devices)
	{
		devices.append(dev->save());
	}
	json["devices"] = devices;
	auto baFileData = QJsonDocument(json).toJson(QJsonDocument::Compact);

	// Write the data to the file:
	f.write(baFileData.constData(), baFileData.size());
	qDebug() << "Session has been saved to file " << m_FileName;
}





void Session::saveToFile(const QString & a_FileName)
{
	setFileName(a_FileName);
	saveToFile();
}




