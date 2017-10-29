#include "DeviceCardDav.h"





void DeviceCardDav::start()
{
	// TODO
}





void DeviceCardDav::stop()
{
	// TODO
}





bool DeviceCardDav::isOnline() const
{
	// TODO
	return false;
}





bool DeviceCardDav::load(const QJsonObject & a_Config)
{
	m_ServerUrl   = a_Config["serverUrl"].toString();
	m_UserName    = a_Config["userName"].toString();
	m_Password    = a_Config["password"].toString();
	m_DisplayName = a_Config["displayName"].toString();
	if (m_DisplayName.isEmpty())
	{
		m_DisplayName = tr("CardDAV server");
	}

	return (
		!m_ServerUrl.isEmpty() &&
		!m_UserName.isEmpty() &&
		!m_Password.isEmpty()
	);
}





QJsonObject DeviceCardDav::save() const
{
	QJsonObject res;
	res["displayName"] = m_DisplayName;
	res["serverUrl"]   = m_ServerUrl;
	res["userName"]    = m_UserName;
	res["password"]    = m_Password;
	return res;
}
