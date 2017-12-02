#include <QString>
#include <QtTest>
#include "../VCardParser.h"
#include "../Exceptions.h"





class TestVCardParser:
	public QObject
{
	Q_OBJECT

public:
	TestVCardParser();

private Q_SLOTS:
	void testBasicCRLF();
	void testBasicLF();
	void testQuotes();
	void testBase64();
};





TestVCardParser::TestVCardParser()
{
}





void TestVCardParser::testBasicCRLF()
{
	QByteArray vcard(
		"begin:vcard\r\n"
		"version:4\r\n"
		"fn:Example contact\r\n"
		"TEL;tYPe=Work:112\r\n"
		"email;work:always.busy@emergency.com\r\n"
		"end:vcard"
	);
	QBuffer buf(&vcard);
	buf.open(QIODevice::ReadOnly | QIODevice::Text);
	ContactBookPtr contacts(new ContactBook(""));
	try
	{
		VCardParser::parse(buf, contacts);
	}
	catch (const EException & exc)
	{
		QFAIL("Failed to parse basic VCard");
	}
	QVERIFY(contacts != nullptr);
	QVERIFY(contacts->contacts().size() == 1);
	const auto & c = contacts->contacts()[0];
	const auto & sentences = c->sentences();
	QVERIFY(sentences.size() == 3);
	QVERIFY(sentences[0].m_Group.isEmpty());
	QVERIFY(sentences[0].m_Key == "fn");
	QVERIFY(sentences[0].m_Params.empty());
	QVERIFY(sentences[0].m_Value == "Example contact");
	QVERIFY(sentences[1].m_Group.isEmpty());
	QVERIFY(sentences[1].m_Key == "tel");
	QVERIFY(sentences[1].m_Params.size() == 1);
	QVERIFY(sentences[1].m_Params[0].m_Name == "type");
	QVERIFY(sentences[1].m_Params[0].m_Values.size() == 1);
	QVERIFY(sentences[1].m_Params[0].m_Values[0] == "Work");
	QVERIFY(sentences[1].m_Value == "112");
	QVERIFY(sentences[2].m_Group.isEmpty());
	QVERIFY(sentences[2].m_Key == "email");
	QVERIFY(sentences[2].m_Params.size() == 1);
	QVERIFY(sentences[2].m_Params[0].m_Name == "work");
	QVERIFY(sentences[2].m_Params[0].m_Values.empty());
	QVERIFY(sentences[2].m_Value == "always.busy@emergency.com");
}





void TestVCardParser::testBasicLF()
{
	QByteArray vcard(
		"begin:vcard\n"
		"version:4\n"
		"fn:Example contact\n"
		"TEL;tYPe=Work:112\n"
		"email;work:always.busy@emergency.com\n"
		"end:vcard\n"
	);
	QBuffer buf(&vcard);
	buf.open(QIODevice::ReadOnly | QIODevice::Text);
	ContactBookPtr contacts(new ContactBook(""));
	try
	{
		VCardParser::parse(buf, contacts);
	}
	catch (const EException & exc)
	{
		QFAIL("Failed to parse basic VCard");
	}
	QVERIFY(contacts != nullptr);
	QVERIFY(contacts->contacts().size() == 1);
	const auto & c = contacts->contacts()[0];
	const auto & sentences = c->sentences();
	QVERIFY(sentences.size() == 3);
	QVERIFY(sentences[0].m_Group.isEmpty());
	QVERIFY(sentences[0].m_Key == "fn");
	QVERIFY(sentences[0].m_Params.empty());
	QVERIFY(sentences[0].m_Value == "Example contact");
	QVERIFY(sentences[1].m_Group.isEmpty());
	QVERIFY(sentences[1].m_Key == "tel");
	QVERIFY(sentences[1].m_Params.size() == 1);
	QVERIFY(sentences[1].m_Params[0].m_Name == "type");
	QVERIFY(sentences[1].m_Params[0].m_Values.size() == 1);
	QVERIFY(sentences[1].m_Params[0].m_Values[0] == "Work");
	QVERIFY(sentences[1].m_Value == "112");
	QVERIFY(sentences[2].m_Group.isEmpty());
	QVERIFY(sentences[2].m_Key == "email");
	QVERIFY(sentences[2].m_Params.size() == 1);
	QVERIFY(sentences[2].m_Params[0].m_Name == "work");
	QVERIFY(sentences[2].m_Params[0].m_Values.empty());
	QVERIFY(sentences[2].m_Value == "always.busy@emergency.com");
}





void TestVCardParser::testQuotes()
{
	QByteArray vcard(
		"begin:vcard\r\n"
		"version:4\r\n"
		"tel;some=param;type=\"Work,fAX\";other=param;another=\"quoted param\":123456789\r\n"
		"tel;type=\"Home\";some=param:123456788\r\n"
		"fn:Example contact\r\n"
		"end:vcard"
	);
	QBuffer buf(&vcard);
	buf.open(QIODevice::ReadOnly | QIODevice::Text);
	ContactBookPtr contacts(new ContactBook(""));
	try
	{
		VCardParser::parse(buf, contacts);
	}
	catch (const EException & exc)
	{
		QFAIL("Failed to parse basic VCard");
	}
	QVERIFY(contacts != nullptr);
	QCOMPARE(static_cast<int>(contacts->contacts().size()), 1);
	const auto & c = contacts->contacts()[0];
	const auto & sentences = c->sentences();
	QCOMPARE(static_cast<int>(sentences.size()), 3);
	QCOMPARE(static_cast<int>(sentences[0].m_Params.size()), 4);
	QCOMPARE(static_cast<int>(sentences[0].m_Params[1].m_Values.size()), 2);
	QCOMPARE(sentences[0].m_Params[1].m_Name, QByteArray("type"));
	QCOMPARE(sentences[0].m_Params[1].m_Values[0], QByteArray("Work"));
	QCOMPARE(sentences[0].m_Params[1].m_Values[1], QByteArray("fAX"));
	QCOMPARE(sentences[0].m_Value, QByteArray("123456789"));

	QCOMPARE(static_cast<int>(sentences[1].m_Params.size()), 2);
	QCOMPARE(static_cast<int>(sentences[1].m_Params[0].m_Values.size()), 1);
	QCOMPARE(sentences[1].m_Params[0].m_Name, QByteArray("type"));
	QCOMPARE(sentences[1].m_Params[0].m_Values[0], QByteArray("Home"));
	QCOMPARE(sentences[1].m_Value, QByteArray("123456788"));
}





void TestVCardParser::testBase64()
{
	QByteArray vcard(
		"begin:vcard\r\n"
		"version:4\r\n"
		"tel;type=\"Work,fAX\";encoding=base64;some=param:MTIzNDU2Nzg5\r\n"
		"fn:Example contact\r\n"
		"end:vcard"
	);
	QBuffer buf(&vcard);
	buf.open(QIODevice::ReadOnly | QIODevice::Text);
	ContactBookPtr contacts(new ContactBook(""));
	try
	{
		VCardParser::parse(buf, contacts);
	}
	catch (const EException & exc)
	{
		QFAIL("Failed to parse basic VCard");
	}
	QVERIFY(contacts != nullptr);
	QCOMPARE(static_cast<int>(contacts->contacts().size()), 1);
	const auto & c = contacts->contacts()[0];
	const auto & sentences = c->sentences();
	QCOMPARE(static_cast<int>(sentences.size()), 2);
	QCOMPARE(static_cast<int>(sentences[0].m_Params.size()), 3);
	QCOMPARE(static_cast<int>(sentences[0].m_Params[0].m_Values.size()), 2);
	QCOMPARE(sentences[0].m_Params[0].m_Values[0], QByteArray("Work"));
	QCOMPARE(sentences[0].m_Params[0].m_Values[1], QByteArray("fAX"));
	QCOMPARE(sentences[0].m_Value, QByteArray("123456789"));
}





QTEST_APPLESS_MAIN(TestVCardParser)





#include "TestVCardParser.moc"




