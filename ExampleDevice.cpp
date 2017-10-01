#include "ExampleDevice.h"
#include "ContactBook.h"





ExampleDevice::ExampleDevice()
{
	// Fill in the example contacts:
	m_ExampleContactBooks.emplace_back(new ContactBook(tr("Example contact book")));
	ContactPtr contact(new Contact(tr("Example contact")));
	contact->addNumber({Contact::Number::cnuWork, tr("112")});
	contact->addEmail({Contact::Email::ceuWork, tr("always.busy@emergency.com")});
	m_ExampleContactBooks[0]->addContact(contact);
}





QString ExampleDevice::getDisplayName() const
{
	return tr("Example device");
}





void ExampleDevice::start()
{
	emit online(true);
}





void ExampleDevice::stop()
{
	// Nothing needed yet
}





bool ExampleDevice::isOnline() const
{
	// Always online
	return true;
}





const std::vector<ContactBookPtr> & ExampleDevice::getContactBooks()
{
	return m_ExampleContactBooks;
}





bool ExampleDevice::load(const QJsonObject & a_Config)
{
	// No config needed:
	Q_UNUSED(a_Config);

	return true;
}





QJsonObject ExampleDevice::save() const
{
	return QJsonObject();
}
