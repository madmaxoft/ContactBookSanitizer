#include "ExampleDevice.h"

#include <QBuffer>

#include "ContactBook.h"
#include "VCardParser.h"





ExampleDevice::ExampleDevice()
{
	// Fill in the example contacts:
	m_ExampleContactBooks.emplace_back(new ContactBook(tr("Example contact book")));
	QByteArray vcard(
		"begin:vcard\r\n"
		"version:4\r\n"
		"fn:Example contact\r\n"
		"tel;type=work:112\r\n"
		"email;type=work:always.busy@emergency.com\r\n"
		"end:vcard"
	);
	QBuffer buf(&vcard);
	buf.open(QIODevice::ReadOnly | QIODevice::Text);
	VCardParser::parse(buf, m_ExampleContactBooks[0]);
}





QString ExampleDevice::displayName() const
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





const std::vector<ContactBookPtr> ExampleDevice::contactBooks()
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
