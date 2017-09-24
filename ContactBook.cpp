#include "ContactBook.h"





ContactBook::ContactBook(const QString & a_DisplayName):
	Super(nullptr),
	m_DisplayName(a_DisplayName)
{

}




void ContactBook::addContact(ContactPtr a_Contact)
{
	m_Contacts.push_back(a_Contact);
}




