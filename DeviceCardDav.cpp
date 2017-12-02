#include "DeviceCardDav.h"
#include <assert.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamWriter>
#include <QBuffer>
#include <QFile>
#include "VCardParser.h"
#include "Exceptions.h"





static const int PERIODIC_CHECK_SECONDS = 5 * 60;




/** The various kinds of requests that can be sent / received answers for. */
enum
{
	reqDetectAddressBookSupport = 1,
	reqCurrentUserPrincipal,
	reqAddressbookRoot,
	reqListAddressbooks,
	reqCheckAddressbookEtags,
	reqAddressbookData,
};





////////////////////////////////////////////////////////////////////////////////
// DeviceCardDav:DavContactBook:

ContactPtr DeviceCardDav::DavContactBook::createNewContact()
{
	DavContactPtr res(new DavContact);
	addContact(res);
	return res;
}





DeviceCardDav::DavContactPtr DeviceCardDav::DavContactBook::contactFromUrl(const QUrl & a_Url)
{
	for (const auto & c: m_Contacts)
	{
		const auto dc = std::dynamic_pointer_cast<DavContact>(c);
		if (dc == nullptr)
		{
			qWarning() << __FUNCTION__ << "DAV ContactBook contains a non-DAV contact!";
			assert(!"DAV ContactBook contains a non-DAV contact!");
			continue;
		}
		if (dc->url() == a_Url)
		{
			return dc;
		}
	}
	return nullptr;
}





////////////////////////////////////////////////////////////////////////////////
// DeviceCardDav:

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
	if (a_Reply.error() != QNetworkReply::NoError)
	{
		qDebug() << __FUNCTION__ << ": Received an error reply: " << a_Reply.errorString();
		m_Status = tr("Failed to connect to server");
		setOffline();
		return;
	}

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
	w.writeNamespace(NS_DAV, "d");
	w.writeNamespace(NS_CARDDAV, "c");
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
	Q_UNUSED(a_Reply);

	// Check that the user principal was in fact returned:
	const auto & node = m_DavPropertyTree->node(m_ServerUrl);
	auto prop = node.findProp<DavPropertyTree::HrefProperty>(NS_DAV, "current-user-principal");
	if (prop == nullptr)
	{
		qDebug() << __FUNCTION__ << ": current user principal not found";
		m_Status = tr("Server doesn't accept username / password");
		setOffline();
		return;
	}
	m_PrincipalUrl = m_DavPropertyTree->urlFromHref(prop->m_Href);
	qDebug()
		<< __FUNCTION__ << ": Current user principal detected as "
		<< m_PrincipalUrl.toString();

	// Ask the current user principal about the addressbook root:
	QByteArray baReq;
	QXmlStreamWriter w(&baReq);
	w.writeStartDocument();
	w.writeNamespace(NS_DAV, "d");
	w.writeNamespace(NS_CARDDAV, "c");
	w.writeStartElement("d:propfind");
		w.writeStartElement("d:prop");
			w.writeEmptyElement("c:addressbook-home-set");
		w.writeEndElement();
	w.writeEndElement();
	w.writeEndDocument();
	qDebug() << __FUNCTION__ << ": Requesting addressbook home set: " << baReq;
	m_DavPropertyTree->sendRequest(m_PrincipalUrl, "PROPFIND", 0, baReq, reqAddressbookRoot);
}





void DeviceCardDav::respAddressbookRoot(const QNetworkReply & a_Reply)
{
	Q_UNUSED(a_Reply);

	// Check that the addressbook home was in fact returned:
	const auto & node = m_DavPropertyTree->node(m_PrincipalUrl);
	auto prop = node.findProp<DavPropertyTree::HrefProperty>(NS_CARDDAV, "addressbook-home-set");
	if (prop == nullptr)
	{
		qDebug() << __FUNCTION__ << ": current user principal not found";
		m_Status = tr("Server doesn't accept username / password");
		setOffline();
		return;
	}
	qDebug()
		<< __FUNCTION__ << ": Addressbook home detected as "
		<< prop->m_Href;

	m_AddressbookHomeUrl = m_DavPropertyTree->urlFromHref(prop->m_Href);

	// List all addressbooks:
	QByteArray baReq;
	QXmlStreamWriter w(&baReq);
	w.writeStartDocument();
	w.writeNamespace(NS_DAV, "d");
	w.writeNamespace(NS_CARDDAV, "c");
	w.writeStartElement("d:propfind");
		w.writeStartElement("d:prop");
			w.writeEmptyElement("d:resourcetype");
			w.writeEmptyElement("d:displayname");
		w.writeEndElement();
	w.writeEndElement();
	w.writeEndDocument();
	qDebug() << __FUNCTION__ << ": Requesting addressbook list";
	m_DavPropertyTree->sendRequest(m_AddressbookHomeUrl, "PROPFIND", 1, baReq, reqListAddressbooks);
}





void DeviceCardDav::respListAddressbooks(const QNetworkReply & a_Reply)
{
	Q_UNUSED(a_Reply);

	// Pick addressbooks from the m_AddressbookHomeUrl's children:
	QList<QUrl> addressBookUrls;
	auto children = m_DavPropertyTree->nodeChildren(m_AddressbookHomeUrl);
	for (const auto & child: children)
	{
		const auto & node = m_DavPropertyTree->node(child);
		auto restype = node.findProp<DavPropertyTree::ResourceTypeProperty>(NS_DAV, "resourcetype");
		if (restype != nullptr)
		{
			if (restype->hasResourceType(NS_CARDDAV, "addressbook"))
			{
				qDebug() << __FUNCTION__ << ": Got an addressbook: " << child.toString();
				addressBookUrls.push_back(child);
			}
		}
	}

	// Remove the m_ContactBooks that are no longer present:
	for (auto itr = m_ContactBooks.begin(); itr != m_ContactBooks.end();)
	{
		const auto & baseUrl = (*itr)->m_BaseUrl;
		bool isPresent = false;
		for (const auto & abu: addressBookUrls)
		{
			if (baseUrl == abu)
			{
				isPresent = true;
				break;
			}
		}
		if (!isPresent)
		{
			qDebug() << __FUNCTION__ << ": Removing contactbook " << (*itr)->displayName();
			emit delContactBook(this, itr->get());
			itr = m_ContactBooks.erase(itr);
		}
		else
		{
			++itr;
		}
	}

	// Add the new addressbooks not yet present in m_ContactBooks:
	for (const auto & abu: addressBookUrls)
	{
		bool isPresent = false;
		for (const auto & cb: m_ContactBooks)
		{
			if (cb->m_BaseUrl == abu)
			{
				isPresent = true;
				break;
			}
		}
		if (!isPresent)
		{
			auto displayName = displayNameForAdressbook(abu);
			qDebug() << __FUNCTION__ << ": Adding contactbook " << displayName;
			auto cb = std::make_shared<DavContactBook>(abu, displayName);
			m_ContactBooks.push_back(cb);
			emit addContactBook(this, cb);
			loadContactBook(cb.get());
		}
	}

	// The server has replied successfully to all our queries, mark it as online:
	setOnline();
}





void DeviceCardDav::respCheckAddressbookEtags(const QNetworkReply & a_Reply)
{
	auto cb = contactBookFromUrl(a_Reply.url());
	if (cb == nullptr)
	{
		qWarning() << __FUNCTION__
			<< ": Received a response for unknown ContactBook URL: " << a_Reply.url().toString()
			<< ", ignoring.";
		assert(!"Unknown ContactBook URL");
		return;
	}

	// The server should have reported fresh Etags for each contact in the addressbook
	// Sync the list of contacts and request any that have changed:
	QByteArray baReq;
	QXmlStreamWriter w(&baReq);
	int toSync = 0;
	w.writeStartDocument();
	w.writeNamespace(NS_DAV, "d");
	w.writeNamespace(NS_CARDDAV, "c");
	w.writeStartElement("c:addressbook-query");
		w.writeStartElement("d:prop");
			w.writeEmptyElement("d:getetag");
			w.writeEmptyElement("c:address-data");
		w.writeEndElement();

		const auto & children = m_DavPropertyTree->nodeChildren(a_Reply.url());
		auto size = children.size();
		for (int i = 0; i < size; ++i)
		{
			const auto & chUrl = children.at(i);
			auto contact = cb->contactFromUrl(chUrl);
			if (contact == nullptr)
			{
				w.writeStartElement("d:href");
					w.writeCharacters(m_DavPropertyTree->hrefFromUrl(chUrl));
				w.writeEndElement();
				toSync += 1;
				continue;
			}
			const auto & childNode = m_DavPropertyTree->node(chUrl);
			auto serverEtag = childNode.findProp<DavPropertyTree::TextProperty>(NS_DAV, "getetag");
			if (serverEtag == nullptr)
			{
				qDebug() << __FUNCTION__
					<< ": Server didn't report an etag for contact at " << chUrl.toString()
					<< ", skipping contact.";
				continue;
			}
			if (contact->etag() != serverEtag->value())
			{
				w.writeStartElement("d:href");
					w.writeCharacters(m_DavPropertyTree->hrefFromUrl(chUrl));
				w.writeEndElement();
				toSync += 1;
				continue;
			}
		}
	w.writeEndElement();
	w.writeEndDocument();
	qDebug() << __FUNCTION__ << ": # contacts to sync: " << toSync;
	if (toSync > 0)
	{
		m_DavPropertyTree->sendRequest(a_Reply.url(), "REPORT", 1, baReq, reqAddressbookData);
	}
}





void DeviceCardDav::respAddressData(const QNetworkReply & a_Reply)
{
	// Get the ContactBook on which to perform the sync:
	auto cb = contactBookFromUrl(a_Reply.url());
	if (cb == nullptr)
	{
		qWarning() << __FUNCTION__
			<< ": Received a response for unknown ContactBook URL: " << a_Reply.url().toString()
			<< ", ignoring.";
		assert(!"Unknown ContactBook URL");
		return;
	}

	// Sync the contacts:
	const auto & children = m_DavPropertyTree->nodeChildren(a_Reply.url());
	auto size = children.size();
	for (int i = 0; i < size; ++i)
	{
		const auto & chUrl = children.at(i);
		auto contact = cb->contactFromUrl(chUrl);
		if (contact == nullptr)
		{
			qDebug() << __FUNCTION__ <<": Creating a new contact for URL " << chUrl.toString();
			contact = std::static_pointer_cast<DavContact>(cb->createNewContact());
			contact->setUrl(chUrl);
		}
		const auto & childNode = m_DavPropertyTree->node(chUrl);
		auto serverEtag = childNode.findProp<DavPropertyTree::TextProperty>(NS_DAV, "getetag");
		if (serverEtag == nullptr)
		{
			qDebug() << __FUNCTION__
				<< ": Server didn't report an etag for contact at " << chUrl.toString()
				<< ", skipping contact sync.";
			continue;
		}
		contact->setEtag(serverEtag->value());
		auto serverData = childNode.findProp<DavPropertyTree::TextProperty>(NS_CARDDAV, "address-data");
		if (serverData == nullptr)
		{
			qDebug() << __FUNCTION__
				<< ": Server didn't report any address data for contact at " << chUrl.toString()
				<< ", skipping contact sync.";
			continue;
		}
		parseServerDataToContact(serverData->value(), contact);
		qDebug() << __FUNCTION__ << ": Contact parsed, URL " << chUrl.toString();
	}
}





QString DeviceCardDav::displayNameForAdressbook(const QUrl & a_AddressbookUrl)
{
	auto displayNameProp = m_DavPropertyTree->node(a_AddressbookUrl).findProp<DavPropertyTree::TextProperty>(NS_DAV, "displayname");
	if (displayNameProp != nullptr)
	{
		return displayNameProp->value();
	}
	auto path = a_AddressbookUrl.path();
	if (path.endsWith('/'))
	{
		path.remove(path.length() - 1, 1);
	}
	auto idxLastSlash = path.lastIndexOf('/');
	if (idxLastSlash > 0)
	{
		path.remove(0, idxLastSlash);
	}
	return path;
}





void DeviceCardDav::setOnline()
{
	qDebug() << __FUNCTION__;
	m_IsOnline = true;
	emit online(this, true);
}





void DeviceCardDav::setOffline()
{
	qDebug() << __FUNCTION__;
	m_IsOnline = false;
	emit online(this, false);
}





void DeviceCardDav::loadContactBook(DeviceCardDav::DavContactBook * a_ContactBook)
{
	qDebug() << __FUNCTION__ << ": Loading ContactBook " << a_ContactBook->m_BaseUrl.toString();
	QByteArray baReq;
	QXmlStreamWriter w(&baReq);
	w.writeStartDocument();
	w.writeNamespace(NS_DAV, "d");
	w.writeNamespace(NS_CARDDAV, "c");
	w.writeStartElement("c:addressbook-query");
		w.writeStartElement("d:prop");
			w.writeEmptyElement("d:getetag");
		w.writeEndElement();
	w.writeEndElement();
	w.writeEndDocument();
	m_DavPropertyTree->sendRequest(a_ContactBook->m_BaseUrl, "REPORT", 1, baReq, reqCheckAddressbookEtags);
}





DeviceCardDav::DavContactBookPtr DeviceCardDav::contactBookFromUrl(const QUrl & a_Url)
{
	for (auto & cb: m_ContactBooks)
	{
		if (cb->m_BaseUrl == a_Url)
		{
			return cb;
		}
	}
	return nullptr;
}





void DeviceCardDav::parseServerDataToContact(const QString & a_ServerData, DavContactPtr a_Contact)
{
	// Parse the VCard data into a new contact:
	try
	{
		auto baServerData = a_ServerData.toUtf8();

		// DEBUG: Save data to file:
		static int counter = 0;
		auto fnam = QString("dbg/contact_%1.vcf").arg(counter);
		QFile f(fnam);
		if (f.open(QIODevice::WriteOnly))
		{
			f.write(baServerData);
			f.close();
		}
		counter += 1;

		QBuffer bufServerData(&baServerData);
		bufServerData.open(QIODevice::ReadOnly);
		VCardParser::parse(bufServerData, std::dynamic_pointer_cast<Contact>(a_Contact));
	}
	catch (const EException & exc)
	{
		qDebug() << __FUNCTION__
			<< ": Exception while parsing contact at " << a_Contact->url().toString()
			<< ": " << exc.what()
			<< "\nServer data: " << a_ServerData;
		return;
	}
}





void DeviceCardDav::replyFinished(
	const QNetworkReply * a_Reply,
	const QByteArray * a_ResponseBody,
	QVariant a_UserData
)
{
	Q_UNUSED(a_ResponseBody);

	switch (a_UserData.toInt())
	{
		case reqDetectAddressBookSupport: return respDetectAddressBookSupport(*a_Reply);
		case reqCurrentUserPrincipal:     return respCurrentUserPrincipal(*a_Reply);
		case reqAddressbookRoot:          return respAddressbookRoot(*a_Reply);
		case reqListAddressbooks:         return respListAddressbooks(*a_Reply);
		case reqCheckAddressbookEtags:    return respCheckAddressbookEtags(*a_Reply);
		case reqAddressbookData:          return respAddressData(*a_Reply);
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





