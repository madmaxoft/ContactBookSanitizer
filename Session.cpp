#include "Session.h"





Session::Session():
	Super(nullptr)
{

}





void Session::addContactBook(ContactBookPtr a_ContactBook)
{
	m_ContactBooks.push_back(a_ContactBook);
}





ContactBookPtr Session::getContactBook(const ContactBook * a_ContactBook)
{
	for (auto & cb: m_ContactBooks)
	{
		if (cb.get() == a_ContactBook)
		{
			return cb;
		}
	}
	return nullptr;
}




