#include "DeviceCardDav.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamWriter>





static const int PERIODIC_CHECK_SECONDS = 30;





enum
{
	reqDetectAddressBookSupport = 1,
	reqCurrentUserPrincipal,
};





DeviceCardDav::DeviceCardDav():
	m_IsOnline(false)
{
	connect(&m_PeriodicCheck, &QTimer::timeout, this, &DeviceCardDav::periodicCheck);
}





QString DeviceCardDav::displayName() const
{
	if (m_Status.isEmpty())
	{
		return m_DisplayName;
	}
	return QString::fromUtf8("%1 (%2)").arg(m_DisplayName, m_Status);
}





void DeviceCardDav::start()
{
	// Perform the initial check now:
	periodicCheck();

	// Start checking periodically:
	m_PeriodicCheck.start(1000 * PERIODIC_CHECK_SECONDS);
}





void DeviceCardDav::stop()
{
	m_PeriodicCheck.stop();
}





bool DeviceCardDav::isOnline() const
{
	return m_IsOnline;
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

	if (
		m_ServerUrl.isEmpty() ||
		m_UserName.isEmpty() ||
		m_Password.isEmpty()
	)
	{
		return false;
	}

	// Create a WebDAV property container and parser:
	m_DavPropertyTree.reset(new DavPropertyTree(m_ServerUrl, m_UserName, m_Password));
	connect(m_DavPropertyTree.get(), &DavPropertyTree::requestFinished, this, &DeviceCardDav::replyFinished);

	return true;
}





QJsonObject DeviceCardDav::save() const
{
	QJsonObject res;
	res["type"]        = QString::fromUtf8("CardDav");
	res["displayName"] = m_DisplayName;
	res["serverUrl"]   = m_ServerUrl.toString();
	res["userName"]    = m_UserName;
	res["password"]    = m_Password;
	return res;
}





void DeviceCardDav::respDetectAddressBookSupport(const QNetworkReply & a_Reply)
{
	// Check if the addressbook protocol extension is supported:
	auto support = a_Reply.rawHeader("DAV").split(',');
	bool hasAddressBookSupport = false;
	for (const auto & s: support)
	{
		if (s.trimmed() == "addressbook")
		{
			hasAddressBookSupport = true;
			break;
		}
	}
	if (!hasAddressBookSupport)
	{
		qDebug() << __FUNCTION__
			<< ": Server doesn't report addressbook support. The reported features are: "
			<< a_Reply.rawHeader("DAV");
		m_Status = tr("Server doesn't report addressbook support");
		setOffline();
		return;
	}
	qDebug() << __FUNCTION__ << ": Server supports addressbook on URL " << m_ServerUrl;

	// Find the current user principal URL:
	QByteArray baReq;
	QXmlStreamWriter w(&baReq);
	w.writeStartDocument();
	w.writeNamespace("DAV:", "d");
	w.writeNamespace("urn:ietf:params:xml:ns:carddav", "c");
	w.writeStartElement("d:propfind");
		w.writeStartElement("d:prop");
			w.writeEmptyElement("d:current-user-principal");
		w.writeEndElement();
	w.writeEndElement();
	w.writeEndDocument();
	qDebug() << __FUNCTION__ << ": Requesting user principal: " << baReq;
	m_DavPropertyTree->sendRequest(m_ServerUrl, "PROPFIND", 0, baReq, reqCurrentUserPrincipal);
}





void DeviceCardDav::respCurrentUserPrincipal(const QNetworkReply & a_Reply)
{
	// Check that the user principal was in fact returned:
	const auto & node = m_DavPropertyTree->node(m_ServerUrl);
	auto prop = node.findProp<DavPropertyTree::HrefProperty>("DAV:", "current-user-principal");
	if (prop == nullptr)
	{
		qDebug() << __FUNCTION__ << ": current user principal not found";
		m_Status = tr("Server doesn't accept username / password");
		setOffline();
		return;
	}
	qDebug()
		<< __FUNCTION__ << ": Current user principal detected as "
		<< prop->m_Href;
	// TODO
}





void DeviceCardDav::setOffline()
{
	m_IsOnline = false;
	// TODO: Trigger the online-state-change signals
}





void DeviceCardDav::replyFinished(
	const QNetworkReply & a_Reply,
	const QByteArray & a_ResponseBody,
	QVariant a_UserData
)
{
	switch (a_UserData.toInt())
	{
		case reqDetectAddressBookSupport: return respDetectAddressBookSupport(a_Reply);
		case reqCurrentUserPrincipal:     return respCurrentUserPrincipal(a_Reply);
	}
	qWarning() << __FUNCTION__ << ": Unhandled request type: " << a_UserData;
}





void DeviceCardDav::davResponseError(const QString & a_Error)
{
	Q_UNUSED(a_Error);
	m_Status = tr("Cannot query user principal");
	setOffline();
}





void DeviceCardDav::periodicCheck()
{
	// Start by detecting address-book support on the URL:
	qDebug() << __FUNCTION__ << ": Initiating a periodic check";
	m_DavPropertyTree->sendRequest(m_ServerUrl, "OPTIONS", 0, QByteArray(), reqDetectAddressBookSupport);
}




