#include "DisplayContact.h"
#include <QElapsedTimer>
#include <QDebug>
#include "VCardParser.h"





DisplayContact::DisplayContact(const Contact & a_Contact):
	m_Contact(a_Contact)
{
}





std::shared_ptr<DisplayContact> DisplayContact::fromContact(const Contact & a_Contact)
{
	QElapsedTimer timer;
	timer.start();

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

	qDebug() << __FUNCTION__ << ": took " << timer.elapsed() << " msec.";

	return res;
}





void DisplayContact::addNameItem(const Contact::Sentence & a_NameSentence)
{
	auto components = VCardParser::breakValueIntoParts(a_NameSentence.m_Value);
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
	// Decide the base type:
	QString type(tr("Other", "Phone"));
	if (isType(a_TelSentence.m_Params, "home"))
	{
		type = tr("Home", "Phone");
	}
	else if (isType(a_TelSentence.m_Params, "work"))
	{
		type = tr("Work", "Phone");
	}
	else if (
		isType(a_TelSentence.m_Params, "mobile") ||
		isType(a_TelSentence.m_Params, "cell")
	)
	{
		type = tr("Mobile", "Phone");
	}

	// If it is a fax number, add that info to the label:
	if (isType(a_TelSentence.m_Params, "fax"))
	{
		type = tr("%1 fax").arg(type);
	}

	addItem(icoTel(), type, {a_TelSentence.m_Value});
}





void DisplayContact::addEmailItem(const Contact::Sentence & a_EmailSentence)
{
	// Decide the base type:
	QString type(tr("Other", "Email"));
	if (isType(a_EmailSentence.m_Params, "home"))
	{
		type = tr("Home", "Email");
	}
	else if (isType(a_EmailSentence.m_Params, "work"))
	{
		type = tr("Work", "Email");
	}
	else if (
		isType(a_EmailSentence.m_Params, "mobile") ||
		isType(a_EmailSentence.m_Params, "cell")
	)
	{
		type = tr("Mobile", "Email");
	}

	addItem(icoEmail(), type, {a_EmailSentence.m_Value});
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





bool DisplayContact::isType(const Contact::SentenceParams & a_SentenceParams, const QByteArray & a_LcType)
{
	for (const auto & p: a_SentenceParams)
	{
		if ((p.m_Values.empty() || p.m_Values[0].isEmpty()) && (p.m_Name.toLower() == a_LcType))
		{
			// The sentence has a simple value-less parameter ("TEL;HOME:...")
			return true;
		}
		if (p.m_Name == "type")
		{
			for (const auto & v: p.m_Values)
			{
				if (v.toLower() == a_LcType)
				{
					return true;
				}
			}
		}
	}
	return false;
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





