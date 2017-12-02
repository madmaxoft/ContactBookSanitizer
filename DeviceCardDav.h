#ifndef DEVICECARDDAV_H
#define DEVICECARDDAV_H




#include <QUrl>
#include <QTimer>
#include "Device.h"
#include "DavPropertyTree.h"
#include "ContactBook.h"





class DeviceCardDav:
	public Device
{
	Q_OBJECT
	using Super = Device;


public:

	DeviceCardDav();

	/** Returns the display name that should be used for this device. */
	virtual QString displayName() const override;

	/** Starts the device.
	The device can start its background threads here, and report its Online state only after this call. */
	virtual void start() override;

	/** Stops the device.
	The device should stop its threads here asynchronously. */
	virtual void stop() override;

	/** Returns true if the device is currently online. */
	virtual bool isOnline() const override;

	/** Returns all the contact books currently available in the device. */
	virtual const std::vector<ContactBookPtr> contactBooks() override
	{
		std::vector<ContactBookPtr> res;
		for (const auto & cb: m_ContactBooks)
		{
			res.push_back(cb);
		}
		return res;
	}


protected:

	/** Contact specialization for DAV contact books. */
	class DavContact:
		public Contact
	{
	protected:
		QString m_Etag;
		QUrl m_Url;

	public:
		const QString & etag() const { return m_Etag; }
		void setEtag(const QString & a_Etag) { m_Etag = a_Etag; }
		const QUrl & url() const { return m_Url; }
		void setUrl(const QUrl & a_Url) { m_Url = a_Url; }
	};

	using DavContactPtr = std::shared_ptr<DavContact>;


	/** ContactBook specialization for DAV contact books.
	Remembers the addressbook's base URL. */
	class DavContactBook:
		public ContactBook
	{
		using Super = ContactBook;

	public:

		QUrl m_BaseUrl;

		explicit DavContactBook(const QUrl a_BaseUrl, const QString & a_DisplayName):
			Super(a_DisplayName),
			m_BaseUrl(a_BaseUrl)
		{
		}

		// ContactBook overrides:
		virtual ContactPtr createNewContact() override;

		/** Returns the contact represented by the specified URL, or nullptr if no such contact. */
		DavContactPtr contactFromUrl(const QUrl & a_Url);
	};

	using DavContactBookPtr = std::shared_ptr<DavContactBook>;


	/** The server's base URL. */
	QUrl m_ServerUrl;

	/** The user name to use when connecting to the server. */
	QString m_UserName;

	/** The password to use when connecting to the server. */
	QString m_Password;

	/** The contact books on the server. */
	std::vector<DavContactBookPtr> m_ContactBooks;

	/** The device name, as displayed to the user. */
	QString m_DisplayName;

	/** User-visible status of the device. Empty means OK, non-empty means error.
	Appended to DisplayName. */
	QString m_Status;

	/** Indicates the online state of this device, based on periodic pings to the addressbook API of the server. */
	bool m_IsOnline;

	/** The current user principal URL. */
	QUrl m_PrincipalUrl;

	/** The home URL of the address books for this user. */
	QUrl m_AddressbookHomeUrl;

	/** The timer used for checking the online status of this device periodically.
	Triggers the periodicCheck() slot. */
	QTimer m_PeriodicCheck;

	/** The WebDAV protocol parser and storage for the WebDAV properties.
	Is nullptr while this object is not loaded. */
	std::unique_ptr<DavPropertyTree> m_DavPropertyTree;


	/** Loads the Device-specific data from the configuration.
	a_Config is a config returned by save() in a previous app run, through which a Device descendant is
	expected to persist its logical state - connection settings, login etc. */
	virtual bool load(const QJsonObject & a_Config) override;

	/** Saves the Device-specific config data, so that the Device can be re-created next time
	the app is started. The descendants are expected to save their logical state - connection settings,
	login etc. */
	virtual QJsonObject save() const override;

	/** Handles the response for addressbook support detection. */
	void respDetectAddressBookSupport(const QNetworkReply & a_Reply);

	/** Handles the response for current user principal request. */
	void respCurrentUserPrincipal(const QNetworkReply & a_Reply);

	/** Handles the response for addressbook root query. */
	void respAddressbookRoot(const QNetworkReply & a_Reply);

	/** Handles the response for addressbook list query. */
	void respListAddressbooks(const QNetworkReply & a_Reply);

	/** Handles the response for addressbook Etag report. */
	void respCheckAddressbookEtags(const QNetworkReply & a_Reply);

	/** Handles the response for addressbook data report. */
	void respAddressData(const QNetworkReply & a_Reply);

	/** Returns the display name for the specified addressbook.
	If the server doesn't provide an addressbook displayname, it is synthesized from the URL. */
	QString displayNameForAdressbook(const QUrl & a_AddressbookUrl);

	/** Marks the device as online. */
	void setOnline();

	/** Marks the device as offline.
	TODO: Triggers the online-state-change signals. */
	void setOffline();

	/** Loads the server-side changes for the specified ContactBook (async). */
	void loadContactBook(DavContactBook * a_ContactBook);

	/** Returns the ContactBook that is represented by the specified URL.
	Returns nullptr if URL not found. */
	DavContactBookPtr contactBookFromUrl(const QUrl & a_Url);

	/** Parses the VCard data received from the server into the specified contact. */
	void parseServerDataToContact(const QString & a_ServerData, DavContactPtr a_Contact);


protected slots:

	/** Triggered when a network request to the CardDAV server finishes. */
	void replyFinished(
		const QNetworkReply * a_Reply,
		const QByteArray * a_ResponseBody,
		QVariant a_UserData
	);

	/** Emitted when the DAV response parser runs into an error while processing a response. */
	void davResponseError(const QString & a_Error);

	/** Pings the addressbook API of the server to update the m_IsOnline member.
	Also initiates the refresh of ContactBooks available on the server. */
	void periodicCheck();
};





#endif // DEVICECARDDAV_H
