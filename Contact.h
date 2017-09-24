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
	/** Stores a single piece of information for the contact (single line in vCard). */
	struct Element
	{
		/** The element's kind - what kind of data it contains (phone number / email / url ...). */
		enum Kind
		{
			etkName,
			etkPhoneNumber,
			etkEmail,
			etkNote,
			etkAddress,
			etkUrl,
			etkOrganization,
			etkIM,
			etkNickname,
			etkAnniversary,
			etkPicture,  // m_Usage must indicate the image type, m_Value is Base64-ed image data
			etkUnknown,  // The full vCard line is stored within the associated data
		} m_Kind;

		/** How the element should be used (home / work / fax / internet call ... */
		enum Usage
		{
			// Phone number / email / generic usage:
			etuMobile,
			etuHome,
			etuWork,
			etuFax,
			etuPager,
			etuInternetCall,

			// Anniversaries:
			etuBirthday,
			etuNameday,

			// IM usage:
			etuAIM,
			etuWindowsLive,
			etuYahoo,
			etuSkype,
			etuQQ,
			etuHangouts,
			etuICQ,
			etuJabber,
			etuCustomIM,

			// etkPicture's image types:
			etuImagePNG,
			etuImageJPG,
			etuImageGIF,

			etuNA,  // For items that don't need usage specification
			etuUnknown,
		} m_Usage;

		/** The value of the element - the actual phone number, email etc. */
		QString m_Value;


		/** Constructs a new element out of the specified types. */
		Element(Kind a_Kind, Usage a_Usage, const QString & a_Value):
			m_Kind(a_Kind),
			m_Usage(a_Usage),
			m_Value(a_Value)
		{
		}

		/** Returns the description for the m_Kind + m_Usage combination used in this element. */
		QString getTypeDesc() const;
	};


	Contact(const QString & a_DisplayName);

	const QString & getDisplayName() const { return m_DisplayName; }

	/** Returns a read-only reference to all data elements. */
	const std::vector<Element> & getElements() const { return m_Elements; }

	/** Adds a new element to the contact. */
	void addElement(Element::Kind a_Kind, Element::Usage a_Usage, const QString & a_Value);

protected:

	/** The main name of the contact. */
	QString m_DisplayName;

	/** The full data of the contact. */
	std::vector<Element> m_Elements;
};

typedef std::shared_ptr<Contact> ContactPtr;





#endif // CONTACT_H
