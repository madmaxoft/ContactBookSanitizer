#ifndef DAVPROPERTYTREE_H
#define DAVPROPERTYTREE_H





#include <unordered_map>
#include <memory>
#include <atomic>
#include <QString>
#include <QObject>
#include <QUrl>
#include <QHash>
#include <QNetworkReply>
#include <QDomDocument>





// fwd:
class QBuffer;





/** Represents a tree of URLs that are discovered on a DAV server, together with the known properties
for them.
The information is built incrementally based on the passed WebDAV responses received from the server.
The nodes for individual URLs are stored in a map of URL -> DavPropertyTree::Node, so that any
URL can be quickly located. */
class DavPropertyTree:
	public QObject
{
	Q_OBJECT
	using Super = QObject;

public:

	/** Representation of a single property value.
	This is an interface from which individual properties must inherit.
	It also is a factory - it creates other instances of the same type. The idea is that an instance of
	a subclass is stored with property identification and then asked to create new instances for each parsed
	property value. */
	class Property
	{
	public:
		// Force a virtual destructor in descendants
		virtual ~Property() {}

		/** Creates a new instance of the property, based on the specified DOM node containing the property.
		a_Node is the DOM node representing the property in the WebDAV server response (<d:prop>). */
		virtual std::shared_ptr<Property> createInstance(const QDomNode & a_Node) = 0;
	};


	/** A property that contains only text. */
	class TextProperty:
		public Property
	{
		using Super = Property;

	protected:

		/** The text contents. */
		QString m_Value;


	public:

		const QString & value() const { return m_Value; }
		void setValue(const QString & a_Value) { m_Value = a_Value; }

		// Property overrides:
		virtual std::shared_ptr<Property> createInstance(const QDomNode & a_Node) override;
	};


	/** A property that contains a single HREF element as its value.
	If the property doesn't have a HREF value in it, the m_Href will be set to empty string. */
	class HrefProperty:
		public Property
	{
		using Super = Property;

	public:
		/** The HREF tag's value. */
		QString m_Href;

		virtual std::shared_ptr<Property> createInstance(const QDomNode & a_Node) override;
	};


	/** Property representing (multiple) resource types of an URL object. */
	class ResourceTypeProperty:
		public Property
	{
		using Super = Property;

	public:
		/** The resource types, represented as pairs of (namespace, localname). */
		QList<std::pair<QString, QString>> m_ResourceTypes;

		virtual std::shared_ptr<Property> createInstance(const QDomNode & a_Node) override;

		/** Returns true if the specified resource type is present in m_ResourceTypes. */
		bool hasResourceType(const QString & a_Namespace, const QString & a_LocalName) const;
	};


	/** Representation of a single node in the tree of URLs.
	A node can have any number of properties. */
	class Node
	{
	public:
		Node(const QUrl & a_Url);

		/** Type for mapping Namespace+PropName -> PropValue */
		using Properties = std::map<std::pair<QString, QString>, std::shared_ptr<Property>>;

		const Properties & properties() const { return m_Properties; }
		void addProperty(
			const QString & a_Namespace,
			const QString & a_PropName,
			std::shared_ptr<Property> a_Value
		);

		/** Returns the specified property dyn-casted to the templated Property subclass type.
		If the property is not found, or cannot by dyn-casted, returns nullptr.
		Typical usage: node.findProp<HrefProperty>("DAV:", "current-user-principal") */
		template <typename PROP>
		std::shared_ptr<PROP> findProp(const QString & a_Namespace, const QString & a_Name) const
		{
			auto rawProp = m_Properties.find(std::make_pair(a_Namespace, a_Name));
			if (rawProp == m_Properties.end())
			{
				return nullptr;
			}
			return std::dynamic_pointer_cast<PROP>(rawProp->second);
		}


	protected:

		/** The URL of the node. */
		QUrl m_Url;

		/** All the currently known properties for the node. */
		Properties m_Properties;
	};


	/** Creates a new instance bound to the specified base URL and using the specified login. */
	DavPropertyTree(const QUrl & a_BaseUrl, const QString & a_UserName, const QString & a_Password);

	/** Sends the specified HTTP request to the server.
	When the request finishes, it is processed and then the requestFinished signal is emitted.
	a_UserData is a simple value that is reported back in the requestFinished() signal. */
	void sendRequest(const QUrl & a_Url,
		const char * a_HttpMethod,
		int a_Depth,
		const QByteArray & a_RequestBody = QByteArray(),
		const QVariant & a_UserData = QVariant()
	);

	/** Processes a WebDAV server response, updates the properties based on the response.
	Normally this shouldn't be called from the outside, unless the client code is doing all the server
	interactions on its own. This is called internally for all responses received for sendRequest() calls. */
	void processResponse(const QNetworkReply & a_Reply, const QByteArray & a_Response);

	/** Returns the node representation for the specified URL.
	If such a node doesn't exist, creates a new one and returns that.
	If the URL is not present, but differs from an URL present just by a trailing slash, the difference is ignored
	and the present node is returned. This allows clients to transparently handle servers that return slashed URLs
	for non-slashed queries. */
	Node & node(const QUrl & a_Url);

	/** Returns the node representation for the specified Href.
	If such a node doesn't exist, creates a new one and returns that. */
	Node & node(const QString & a_Href)
	{
		return node(urlFromHref(a_Href));
	}

	/** Returns true if the specified URL already has a node representation in NodeMap. */
	bool hasNode(const QUrl & a_Url) const;

	/** Converts a Href string into a fully qualified QUrl.
	Uses the m_BaseUrl to provide the parts missing from the (possibly relative) href.
	Also resolves the possible up-dirs ("../") etc.,returning a canonical URL that can be used in the nodemap. */
	QUrl urlFromHref(const QString & a_Href) const;

	/** Converts an URL into a href.
	If the URL is not based on m_BaseUrl, throws an EException descendant.
	Also resolves the possible up-dirs ("../") etc., returning a canonical Href that can be used for
	querying the server. */
	QString hrefFromUrl(const QUrl & a_Url) const;

	/** Returns URLs of known immediate children of the specified node. */
	QList<QUrl> nodeChildren(const QUrl & a_NodeUrl);

protected:

	/** Type for mapping URLs to their representation as a Node instance. */
	using NodeMap = QHash<QUrl, std::shared_ptr<Node>>;

	/** Type for storing QBuffer instances for each individual request sent. */
	using BufferMap = QMap<unsigned, std::shared_ptr<QBuffer>>;


	/** The base URL for which the tree is built. */
	QUrl m_BaseUrl;

	/** UserName to use when querying the server. */
	QString m_UserName;

	/** Password to use when querying the server. */
	QString m_Password;

	/** All the known nodes. */
	NodeMap m_NodeMap;

	/** The network access manager used to send the requests to the server. */
	QNetworkAccessManager m_NAM;

	/** The buffers allocated for requests, waiting to be freed.
	Each request sent needs a QBuffer instance; one is created for each request and stored here
	based on the m_NextBufferIndex value; the index is stored in the QNetworkRequest object's Attributes.
	Once the QNetworkRequest object is finished, the buffer is removed from here, effectively freeing
	the memory. */
	BufferMap m_Buffers;

	std::atomic<unsigned> m_NextBufferIndex;


	/** Implementation of the response processing.
	May throw EDavResponseException, caller handles that by emitting a responseError() signal. */
	void internalProcessResponse(const QNetworkReply & a_Reply, const QByteArray & a_Response);

	/** Handles the <DAV:response> element in the multistatus response.
	May throw EDavResponseException, caller handles that by emitting a responseError() signal. */
	void processElementResponse(const QDomNode & a_ResponseElement);

	/** Handles the <DAV:propstat> element inside a <DAV:response> element in the multistatus response.
	May throw EDavResponseException, caller handles that by emitting a responseError() signal. */
	void processElementPropstat(const QString & a_Href, const QDomNode & a_PropstatElement);

	/** Handles the <DAV:prop> element, its status should already have been checked by the caller.
	Stores the property in to the storage. */
	void processElementProp(const QString & a_Href, const QDomNode & a_PropElement);

signals:

	/** Emitted when a reply is received for a request that was sent using sendRequest.
	The reply is already processed internally, if appropriate.
	a_Reply has already had all its body read, the data is provided in the a_ResponseBody parameter.
	a_UserData is the data that was supplied for the request in the sendRequest() call. */
	void requestFinished(
		const QNetworkReply * a_Reply,
		const QByteArray * a_ResponseBody,
		QVariant a_UserData
	);

	/** Emitted when processResponse runs into an error.
	a_Reply is the reply that has caused the error. Its body has already been read all, and is provided in
	a_ResponseBody.
	a_UserData is the data that was supplied for the request in the sendRequest() call.
	a_Error is the error message. */
	void responseError(
		const QNetworkReply * a_Reply,
		const QByteArray * a_ResponseBody,
		QVariant a_UserData,
		const QString & a_Error
	);

protected slots:

	/** Emitted by m_NAM when the reply is received. */
	void internalRequestFinished(QNetworkReply * a_Reply);

	/** Triggered when a network request to the server requests authentication.
	Provides the auth supplied in m_UserName and m_Password. */
	void authenticationRequired(QNetworkReply * a_Reply, QAuthenticator * a_Auth);
};





static const char NS_DAV[] = "DAV:";
static const char NS_CARDDAV[] = "urn:ietf:params:xml:ns:carddav";





#endif // DAVPROPERTYTREE_H
