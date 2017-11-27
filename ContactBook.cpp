#include "ContactBook.h"





ContactBook::ContactBook(const QString & a_DisplayName):
	Super(nullptr),
	m_DisplayName(a_DisplayName)
{

}





void ContactBook::setDisplayName(const QString & a_NewDisplayName)
{
	m_DisplayName = a_NewDisplayName;
	emit displayNameChanged(a_NewDisplayName);
}





ContactPtr ContactBook::createNewContact()
{
	ContactPtr res(new Contact);
	addContact(res);
	return res;
}





void ContactBook::addContact(ContactPtr a_Contact)
{
	m_Contacts.push_back(a_Contact);
	// TODO: emit the appropriate signals
}




