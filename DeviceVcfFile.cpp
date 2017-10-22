#include "DeviceVcfFile.h"
#include <assert.h>
#include <QFileInfo>
#include "VCardParser.h"
#include "Exceptions.h"





DeviceVcfFile::DeviceVcfFile():
	m_ContactBook(new ContactBook(tr("Contacts")))
{
}





QString DeviceVcfFile::displayName() const
{
	return m_DisplayName;
}





void DeviceVcfFile::start()
{
	if (m_VcfFileName.isEmpty())
	{
		// Invalid settings
		return;
	}

	// Load the VCF data:
	QFile f(m_VcfFileName);
	if (!f.open(QFile::ReadOnly | QFile::Text))
	{
		m_DisplayName = tr("%1 (Cannot open file)").arg(m_VcfFileNameBase);
		return;
	}
	try
	{
		VCardParser::parse(f, m_ContactBook);
	}
	catch (const EParseError & exc)
	{
		m_DisplayName = tr("%1 (Cannot parse file: %2)")
			.arg(m_VcfFileNameBase)
			.arg(QString::fromStdString(exc.m_Message));
	}
	catch (const EException &)
	{
		m_DisplayName = tr("%1 (Cannot parse file)").arg(m_VcfFileNameBase);
		return;
	}
}





void DeviceVcfFile::stop()
{
	// Nothing needed
}





bool DeviceVcfFile::isOnline() const
{
	return !m_VcfFileName.isEmpty();
}





bool DeviceVcfFile::load(const QJsonObject & a_Config)
{
	m_VcfFileName = a_Config["fileName"].toString();

	// Strip away the path:
	QFileInfo fi(m_VcfFileName);
	m_VcfFileNameBase = fi.fileName();
	m_DisplayName = m_VcfFileNameBase;

	return true;
}





QJsonObject DeviceVcfFile::save() const
{
	QJsonObject res;
	res["fileName"] = m_VcfFileName;
	return res;
}




