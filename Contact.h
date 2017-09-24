#ifndef CONTACT_H
#define CONTACT_H




#include <memory>
#include <map>

#include <QString>





/** Represents a single contact in a ContactBook.
Contains all data for the contact.
Represents the contact as a vCard internally, with some of the (common) data being mirrored in member variables. */
class Contact
{

public:

	/** Type used for the underlying storage in vCard format. */
	typedef std::map<std::string, std::string> VCardElements;


	Contact(const QString & a_DisplayName);

	const QString & getDisplayName() const { return m_DisplayName; }

	/** Returns a read-only representation of all the vCard data. */
	const VCardElements & getElements() const { return m_Elements; }

	/** Sets the specified element to the specified value.
	If the element name already exists, overwrites the value.
	Recognizes the common data and mirrors them into member variables. */
	void setElement(const std::string & a_Name, const std::string & a_Value);

protected:

	/** The main name of the contact. */
	QString m_DisplayName;

	/** The full data of the contact as a parsed vCard. */
	VCardElements m_Elements;


	/** Returns the pointer to the member variable in which the specified element's value is stored.
	Returns nullptr if the element is not a recognized common data. */
	QString * internalMemberForElement(const std::string & a_ElementName);

};

typedef std::shared_ptr<Contact> ContactPtr;





#endif // CONTACT_H
