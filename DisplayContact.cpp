#include "DisplayContact.h"
#include "VCardParser.h"





DisplayContact::DisplayContact(const Contact & a_Contact):
	m_Contact(a_Contact)
{
}





std::shared_ptr<DisplayContact> DisplayContact::fromContact(const Contact & a_Contact)
{
	std::shared_ptr<DisplayContact> res(new DisplayContact(a_Contact));

	for (const auto & s: a_Contact.sentences())
	{
		if (s.m_Key == "fn")
		{
			res->m_DisplayName = s.m_Value;
		}
		else if (s.m_Key == "n")
		{
			res->addNameItem(s);
		}
		else if (s.m_Key == "tel")
		{
			res->addTelItem(s);
		}
		else if (s.m_Key == "email")
		{
			res->addEmailItem(s);
		}
	}

	return res;
}





void DisplayContact::addNameItem(const Contact::Sentence & a_NameSentence)
{
	auto components = VCardParser::breakValueIntoComponents(a_NameSentence.m_Value);
	while (components.size() < 5)
	{
		components.push_back({});
	}
	const auto & lastNames = components[0];
	const auto & firstNames = components[1];
	const auto & middleNames = components[2];
	const auto & prefixes = components[3];
	const auto & suffixes = components[4];

	auto name = composeName(firstNames, middleNames, lastNames, prefixes, suffixes);
	addItem(nullptr, tr("Name"), {name});
}





void DisplayContact::addTelItem(const Contact::Sentence & a_TelSentence)
{
	// TODO: Decide if the number is home, work, fax etc.
	addItem(icoTel(), tr("Home"), {a_TelSentence.m_Value});
}





void DisplayContact::addEmailItem(const Contact::Sentence & a_EmailSentence)
{
	// TODO
}





void DisplayContact::addItem(const QIcon * a_Icon, const QString & a_Label, const std::vector<QString> & a_Values)
{
	m_Items.emplace_back(new Item(a_Icon, a_Label, a_Values));
}





QString DisplayContact::composeName(
	const std::vector<QByteArray> & a_FirstNames,
	const std::vector<QByteArray> & a_MiddleNames,
	const std::vector<QByteArray> & a_LastNames,
	const std::vector<QByteArray> & a_Prefixes,
	const std::vector<QByteArray> & a_Suffixes
)
{
	QString res;
	for (const auto & s: a_Prefixes)
	{
		if ((res.length() > 0) && !res.at(res.length() - 1).isSpace())
		{
			res.append(" ");
		}
		res.append(QString::fromUtf8(s));
	}
	for (const auto & s: a_FirstNames)
	{
		if ((res.length() > 0) && !res.at(res.length() - 1).isSpace())
		{
			res.append(" ");
		}
		res.append(QString::fromUtf8(s));
	}
	for (const auto & s: a_MiddleNames)
	{
		if ((res.length() > 0) && !res.at(res.length() - 1).isSpace())
		{
			res.append(" ");
		}
		res.append(QString::fromUtf8(s));
	}
	for (const auto & s: a_LastNames)
	{
		if ((res.length() > 0) && !res.at(res.length() - 1).isSpace())
		{
			res.append(" ");
		}
		res.append(QString::fromUtf8(s));
	}

	if (!a_Suffixes.empty() && !a_Suffixes[0].isEmpty())
	{
		res.append(",");
	}
	for (const auto & s: a_Suffixes)
	{
		if ((res.length() > 0) && !res.at(res.length() - 1).isSpace())
		{
			res.append(" ");
		}
		res.append(QString::fromUtf8(s));
	}
	return res;
}





QIcon * DisplayContact::icoTel()
{
	static QIcon ico{":/res/Phone.png"};
	return &ico;
}





QIcon * DisplayContact::icoEmail()
{
	static QIcon ico(":/res/Email.png");
	return &ico;
}





