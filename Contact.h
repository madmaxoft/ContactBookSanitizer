#ifndef CONTACT_H
#define CONTACT_H




#include <memory>
#include <vector>

#include <QString>





/** Represents a single contact in a ContactBook.
Contains all data for the contact. */
class Contact
{

public:
	/** Represents the name components. */
	struct NameComponents
	{
		QString m_FirstName;
		QString m_MiddleName;
		QString m_LastName;
	};


	/** Represents a single number property for a contact. */
	struct Number
	{
		enum Usage
		{
			cnuMobile,
			cnuHome,
			cnuWork,
			cnuFax,
			cnuPager,
			cnuOther,
			cnuCustom,
		} m_Usage;
		QString m_CustomUsage;
		QString m_Number;


		/** Creates a new Number object initialized with the specified values. */
		Number(Usage a_Usage, const QString & a_Number, const QString & a_CustomUsage = QString()):
			m_Usage(a_Usage),
			m_CustomUsage(a_CustomUsage),
			m_Number(a_Number)
		{}
	};
	typedef std::vector<Number> Numbers;


	/** Represents a single email address property for a contact. */
	struct Email
	{
		enum Usage
		{
			ceuHome,
			ceuWork,
			ceuMobile,
			ceuOther,
			ceuCustom,
		} m_Usage;
		QString m_CustomUsage;
		QString m_Email;


		/** Creates a new Email object initialized with the specified values. */
		Email(Usage a_Usage, const QString & a_Email, const QString & a_CustomUsage = QString()):
			m_Usage(a_Usage),
			m_CustomUsage(a_CustomUsage),
			m_Email(a_Email)
		{}
	};
	typedef std::vector<Email> Emails;


	/** Represents a single (postal) address property for a contact. */
	struct Address
	{
		enum Usage
		{
			cauHome,
			cauWork,
			cauOther,
			cauCustom,
		} m_Usage;
		QString m_CustomUsage;
		QString m_POBox;
		QString m_Ext;
		QString m_Street;
		QString m_City;
		QString m_State;
		QString m_PostCode;
		QString m_Country;
		double m_Latitude;
		double m_Longitude;
	};
	typedef std::vector<Address> Addresses;


	/** Represents a single IM property for a contact. */
	struct IM
	{
		enum Kind
		{
			imkAIM,
			imkWindowsLive,
			imkYahoo,
			imkSkype,
			imkQQ,
			imkHangouts,
			imkICQ,
			imkJabber,
			imkCustomIM,
		} m_Kind;
		QString m_CustomKind;
		QString m_IM;


		/** Creates a new IM object initialized with the specified values. */
		IM(Kind a_Kind, const QString & a_IM, const QString & a_CustomKind = QString()):
			m_Kind(a_Kind),
			m_CustomKind(a_CustomKind),
			m_IM(a_IM)
		{}
	};
	typedef std::vector<IM> IMs;


	struct Picture
	{
		enum Format
		{
			cpfJPEG,
			cpfPNG,
			cpfGIF,
			cpfCustom,
		} m_Format;
		QString m_CustomFormat;
		QByteArray m_ImageData;


		/** Creates a new Picture object initialized with the specified values. */
		Picture(Format a_Format, const QByteArray & a_ImageData, const QString & a_CustomFormat = QString()):
			m_Format(a_Format),
			m_CustomFormat(a_CustomFormat),
			m_ImageData(a_ImageData)
		{}
	};
	typedef std::vector<Picture> Pictures;


	struct URL
	{
		enum Kind
		{
			cukHomepage,
			cukCompanyHome,
			cukEShop,
			cukCustom,
		} m_Kind;
		QString m_CustomKind;
		QString m_URL;


		/** Creates a new URL object initialized with the specified values. */
		URL(Kind a_Kind, const QString & a_URL, const QString & a_CustomKind = QString()):
			m_Kind(a_Kind),
			m_CustomKind(a_CustomKind),
			m_URL(a_URL)
		{}
	};
	typedef std::vector<URL> URLs;


	/** Creates a new Contact, initializes the DisplayName. */
	Contact(const QString & a_DisplayName);

	// Getters:
	const QString & getDisplayName() const { return m_DisplayName; }
	const NameComponents & getDisplayNameComponents() const { return m_DisplayNameComponents; }
	const QString & getPhoneticName() const { return m_PhoneticName; }
	const NameComponents & getPhoneticNameComponents() const { return m_PhoneticNameComponents; }
	const QString & getNotes() const { return m_Notes; }
	const QString & getOrganization() const { return m_Organization; }
	const QString & getTitle() const { return m_Title; }
	const QString & getNickname() const { return m_Nickname; }
	const Numbers & getNumbers() const { return m_Numbers; }
	const Emails & getEmails() const { return m_Emails; }
	const Addresses & getAddresses() const { return m_Addresses; }
	const IMs & getIMs() const { return m_IMs; }
	const Pictures & getPictures() const { return m_Pictures; }
	const URLs & getURLs() const { return m_URLs; }

	// Setters:
	void setDisplayName(const QString & a_DisplayName) { m_DisplayName = a_DisplayName; }
	void setPhoneticName(const QString & a_PhoneticName) { m_PhoneticName = a_PhoneticName; }
	void setNotes(const QString & a_Notes) { m_Notes = a_Notes; }
	void setOrganization(const QString & a_Organization) { m_Organization = a_Organization; }
	void setTitle(const QString & a_Title) { m_Title = a_Title; }
	void setNickname(const QString & a_Nickname) { m_Nickname = a_Nickname; }

	// Adders:
	void addNumber (const Number & a_Number)   { m_Numbers.push_back(a_Number); }
	void addEmail  (const Email & a_Email)     { m_Emails.push_back(a_Email); }
	void addAddress(const Address & a_Address) { m_Addresses.push_back(a_Address); }
	void addIM     (const IM & a_IM)           { m_IMs.push_back(a_IM); }
	void addPicture(const Picture & a_Picture) { m_Pictures.push_back(a_Picture); }

	// Deleters:
	void delNumber (size_t a_Idx) { m_Numbers.erase  (m_Numbers.begin()   + a_Idx); }
	void delEmail  (size_t a_Idx) { m_Emails.erase   (m_Emails.begin()    + a_Idx); }
	void delAddress(size_t a_Idx) { m_Addresses.erase(m_Addresses.begin() + a_Idx); }
	void delIM     (size_t a_Idx) { m_IMs.erase      (m_IMs.begin()       + a_Idx); }
	void delPicture(size_t a_Idx) { m_Pictures.erase (m_Pictures.begin()  + a_Idx); }

	// Changers:
	void setNumber (size_t a_Idx, const Number & a_Number)   { m_Numbers  [a_Idx] = a_Number; }
	void setEmail  (size_t a_Idx, const Email & a_Email)     { m_Emails   [a_Idx] = a_Email; }
	void setAddress(size_t a_Idx, const Address & a_Address) { m_Addresses[a_Idx] = a_Address; }
	void setIM     (size_t a_Idx, const IM & a_IM)           { m_IMs      [a_Idx] = a_IM; }
	void setPicture(size_t a_Idx, const Picture & a_Picture) { m_Pictures [a_Idx] = a_Picture; }

protected:

	/** The main name of the contact. */
	QString m_DisplayName;

	/** Name, broken into individual components (if available).
	All empty strings when not available. */
	NameComponents m_DisplayNameComponents;

	/** Phonetic transcription of the name. */
	QString m_PhoneticName;

	/** Phonetic transcription of the name, broken into individual components. */
	NameComponents m_PhoneticNameComponents;

	QString m_Notes;
	QString m_Organization;
	QString m_Title;
	QString m_Nickname;

	/** All the numbers available for the contact. */
	Numbers m_Numbers;

	/** All the email addresses available for the contact. */
	Emails m_Emails;

	/** All the addresses available for the contact. */
	Addresses m_Addresses;

	/** All the IMs available for the contact. */
	IMs m_IMs;

	/** All the pictures of the contact. */
	Pictures m_Pictures;

	/** All the URLs available for the contact. */
	URLs m_URLs;
};

typedef std::shared_ptr<Contact> ContactPtr;





#endif // CONTACT_H
