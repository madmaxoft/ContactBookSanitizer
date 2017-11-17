#include "DavPropertyTree.h"
#include <QDebug>
#include <QAuthenticator>
#include <QFile>
#include "Exceptions.h"





static void logReply(QNetworkReply * a_Reply, const QByteArray & a_ResponseBody)
{
	// Log the reply to Qt log:
	auto & req = a_Reply->request();
	auto method = req.attribute(QNetworkRequest::CustomVerbAttribute).toString();
	qDebug() << "Reply finished: " << method << a_Reply->url().toString();
	qDebug() << "UserData: " << a_Reply->request().attribute(QNetworkRequest::User);
	qDebug() << "Error: " << a_Reply->error();
	qDebug() << "Status: " << a_Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
	qDebug() << "Headers (" << a_Reply->rawHeaderPairs().count() << "):";
	for (const auto & hdr: a_Reply->rawHeaderPairs())
	{
		qDebug() << "  " << hdr.first << ": " << hdr.second;
	}
	qDebug() << "Response body: " << a_ResponseBody;

	// Log the reply to a file:
	static int counter = 0;
	auto path = a_Reply->url().path();
	if (!path.isEmpty() && path.right(1) == "/")
	{
		path = path.left(path.length() - 1);
	}
	auto lastSlash = path.lastIndexOf('/');
	if (lastSlash >= 0)
	{
		path.remove(0, lastSlash + 1);
	}
	QFile f(QString("response_%1.log").arg(counter++));
	f.open(QFile::WriteOnly);
	f.write(QString("Request: %1 %2\n").arg(method, req.url().toString()).toUtf8());
	f.write(QString("Error: %1\n").arg(a_Reply->error()).toUtf8());
	f.write(QString("Status: %1\n").arg(a_Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()).toUtf8());
	f.write(QString("Headers (%1):\n").arg(a_Reply->rawHeaderPairs().count()).toUtf8());
	for (const auto & hdr: a_Reply->rawHeaderPairs())
	{
		f.write("  ");
		f.write(hdr.first);
		f.write(": ");
		f.write(hdr.second);
		f.write("\n");
	}
	f.write("\n");
	f.write(a_ResponseBody);
}




////////////////////////////////////////////////////////////////////////////////
// DavPropertyTree::HrefProperty:

std::shared_ptr<DavPropertyTree::Property> DavPropertyTree::HrefProperty::createInstance(const QDomNode & a_Node)
{
	std::shared_ptr<DavPropertyTree::HrefProperty> res(new DavPropertyTree::HrefProperty);
	auto n = a_Node.firstChild();
	while (!n.isNull())
	{
		if ((n.namespaceURI() == "DAV:") && (n.localName() == "href") && n.hasChildNodes())
		{
			res->m_Href = n.firstChild().toText().data();
			break;
		}
	}
	return res;
}





////////////////////////////////////////////////////////////////////////////////
// DavPropertyTree::Node:

DavPropertyTree::Node::Node(const QUrl & a_Url):
	m_Url(a_Url)
{
}





void DavPropertyTree::Node::addProperty(const QString & a_Namespace, const QString & a_PropName, std::shared_ptr<Property> a_Value)
{
	auto key = std::make_pair(a_Namespace, a_PropName);
	m_Properties[key] = a_Value;
}





////////////////////////////////////////////////////////////////////////////////
// DavPropertyTree:

DavPropertyTree::DavPropertyTree(const QUrl & a_BaseUrl, const QString & a_UserName, const QString & a_Password):
	m_BaseUrl(a_BaseUrl),
	m_UserName(a_UserName),
	m_Password(a_Password)
{
	connect(&m_NAM, &QNetworkAccessManager::finished,               this, &DavPropertyTree::internalRequestFinished);
	connect(&m_NAM, &QNetworkAccessManager::authenticationRequired, this, &DavPropertyTree::authenticationRequired);
}





void DavPropertyTree::sendRequest(
	const QUrl & a_Url,
	const char * a_HttpMethod,
	int a_Depth,
	const QByteArray & a_RequestBody,
	const QVariant & a_UserData
)
{
	auto req = new QNetworkRequest(a_Url);
	req->setAttribute(QNetworkRequest::User, a_UserData);
	req->setHeader(QNetworkRequest::ContentTypeHeader, "application/xml; charset=utf-8");
	req->setRawHeader("Depth", QByteArray::number(a_Depth));
	m_NAM.sendCustomRequest(*req, a_HttpMethod, a_RequestBody);
}





void DavPropertyTree::processResponse(const QNetworkReply & a_Reply, const QByteArray & a_Response)
{
	try
	{
		internalProcessResponse(a_Reply, a_Response);
	}
	catch (const EDavResponseException & exc)
	{
		qDebug() << "Error while processing response:";
		const auto & req = a_Reply.request();
		qDebug() << "Request: "
			<< req.attribute(QNetworkRequest::UserMax) << req.url();
		qDebug() << "Response: "
			<< a_Reply.attribute(QNetworkRequest::HttpStatusCodeAttribute)
			<< a_Reply.attribute(QNetworkRequest::HttpReasonPhraseAttribute)
			<< "\n"
			<< a_Response;
		qDebug() << "Error: "
			<< "File " << exc.m_SrcFileName.c_str()
			<< "line " << exc.m_SrcLine
			<< ", Message: " << exc.m_Message;
		emit responseError(
			a_Reply,
			a_Response,
			a_Reply.request().attribute(QNetworkRequest::User),
			tr("%1 (%2:%3)")
				.arg(exc.m_Message)
				.arg(QString::fromStdString(exc.m_SrcFileName))
				.arg(exc.m_SrcLine)
		);
	}
}





DavPropertyTree::Node & DavPropertyTree::node(const QUrl & a_Url)
{
	auto & res = m_NodeMap[a_Url];
	if (res == nullptr)
	{
		res.reset(new Node(a_Url));
	}
	return *res;
}





void DavPropertyTree::internalProcessResponse(const QNetworkReply & a_Reply, const QByteArray & a_Response)
{
	// Only process a 207 (multistatus) response, skip all the others:
	auto httpResponseCode = a_Reply.attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (httpResponseCode != 207)
	{
		qDebug() << __FUNCTION__ << ": Skipping response, not a 207 multistatus, but a " << httpResponseCode;
		return;
	}

	// Parse the XML body:
	QDomDocument doc;
	QString err;
	if (!doc.setContent(a_Response, true, &err))
	{
		throw EDavResponseException(
			__FILE__, __LINE__,
			tr("Cannot parse server response when asking for current user principal: %1").arg(err)
		);
	}

	// Process the XML:
	bool hasMultiStatus = false;
	auto n = doc.firstChild();
	while (!n.isNull())
	{
		if ((n.namespaceURI() == "DAV:") && (n.localName() == "multistatus"))
		{
			// Process a "multistatus" response
			hasMultiStatus = true;
			auto n2 = n.firstChild();
			while (!n2.isNull())
			{
				if (n2.namespaceURI() == "DAV:")
				{
					if (n2.localName() == "response")
					{
						// Process a "response" element
						processElementResponse(a_Reply, a_Response, n2);
					}
				}
				n2 = n2.nextSibling();
			}
		}
		n = n.nextSibling();
	}
	if (!hasMultiStatus)
	{
		throw EDavResponseException(
			__FILE__, __LINE__,
			tr("Response doesn't contain a DAV:multistatus element")
		);
	}
}





void DavPropertyTree::processElementResponse(
	const QNetworkReply & a_Reply,
	const QByteArray & a_ResponseBody,
	const QDomNode & a_ResponseElement
)
{
	qDebug() << __FUNCTION__ << ": Processing <response> element.";

	// Find the href for which this element is reported:
	QString href;
	auto n = a_ResponseElement.firstChild();
	while (!n.isNull())
	{
		if ((n.namespaceURI() == "DAV:") && (n.localName() == "href") && n.hasChildNodes())
		{
			href = n.firstChild().toText().data();
			break;
		}
		n = n.nextSibling();
	}
	if (href.isEmpty())
	{
		qDebug() << __FUNCTION__ << ": No <DAV:href> element found in <DAV:response>, skipping the response node.";
		return;
	}

	// Process each member of the response:
	n = a_ResponseElement.firstChild();
	while (!n.isNull())
	{
		if (n.namespaceURI() == "DAV:")
		{
			if (n.localName() == "propstat")
			{
				processElementPropstat(a_Reply, a_ResponseBody, href, n);
			}
		}
		n = n.nextSibling();
	}
}




void DavPropertyTree::processElementPropstat(
	const QNetworkReply & a_Reply,
	const QByteArray & a_ResponseBody,
	const QString & a_Href,
	const QDomNode & a_PropstatElement
)
{
	qDebug() << __FUNCTION__ << ": Processing <propstat> element.";
	auto n = a_PropstatElement.firstChild();
	QDomNode propElement, statusElement;
	while (!n.isNull())
	{
		if (n.namespaceURI() == "DAV:")
		{
			if (n.localName() == "prop")
			{
				propElement = n;
			}
			else if (n.localName() == "status")
			{
				statusElement = n;
			}
		}
		n = n.nextSibling();
	}
	if (propElement.isNull() || statusElement.isNull())
	{
		qDebug()
			<< __FUNCTION__ << ": Missing prop or status element inside a propstat element:"
			<< "Prop: " << propElement.isNull()
			<< "Status: " << statusElement.isNull();
		return;
	}
	if (!statusElement.hasChildNodes())
	{
		qDebug() << __FUNCTION__ << ": status element has no content.";
		return;
	}

	// Parse and check the status line:
	auto status = statusElement.firstChild().toText().data();
	if (status.left(7) != "HTTP/1.")
	{
		qDebug() << __FUNCTION__ << ": Cannot parse status (initial match): " << status;
		return;
	}
	auto firstSpace = status.indexOf(' ');
	if (firstSpace < 0)
	{
		qDebug() << __FUNCTION__ << ": Cannot parse status (no space): " << status;
		return;
	}
	auto secondSpace = status.indexOf(' ', firstSpace + 1);
	if (secondSpace < 0)
	{
		qDebug() << __FUNCTION__ << ": Cannot parse status (second space): " << status;
		return;
	}
	bool ok = false;
	auto statusNum = status.mid(firstSpace + 1, secondSpace - firstSpace - 1).toInt(&ok);
	if (!ok)
	{
		qDebug() << __FUNCTION__ << ": Cannot parse status (not a number): " << status;
		return;
	}
	if (statusNum / 100 != 2)
	{
		qDebug() << __FUNCTION__ << ": Status unsuccessful: " << statusNum;
		return;
	}

	// The status is OK, process the property:
	processElementProp(href, propElement);
}




void DavPropertyTree::processElementProp(const QString & a_Href, const QDomNode & a_PropElement)
{
	auto n = this->node(a_Href);
	auto handler = findPropHandler(a_PropElement.namespaceURI(), a_PropElement.localName());

}





void DavPropertyTree::internalRequestFinished(QNetworkReply * a_Reply)
{
	auto baResp = a_Reply->readAll();

	// DEBUG:
	logReply(a_Reply, baResp);

	if (a_Reply->error() == QNetworkReply::NoError)
	{
		processResponse(*a_Reply, baResp);
	}
	emit requestFinished(*a_Reply, baResp, a_Reply->request().attribute(QNetworkRequest::User));
}





void DavPropertyTree::authenticationRequired(QNetworkReply * a_Reply, QAuthenticator * a_Auth)
{
	Q_UNUSED(a_Reply);

	qDebug() << __FUNCTION__ << ": Sending authentication...";
	a_Auth->setUser(m_UserName);
	a_Auth->setPassword(m_Password);
}




