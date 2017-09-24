#include "Contact.h"
#include <algorithm>
#include <assert.h>





////////////////////////////////////////////////////////////////////////////////
// Contact::Element:

QString Contact::Element::getTypeDesc() const
{
	QString kind;
	switch (m_Kind)
	{
		case etkName:         kind = QString::fromUtf8("Name");         break;
		case etkPhoneNumber:  kind = QString::fromUtf8("Number");       break;
		case etkEmail:        kind = QString::fromUtf8("Email");        break;
		case etkNote:         kind = QString::fromUtf8("Note");         break;
		case etkAddress:      kind = QString::fromUtf8("Address");      break;
		case etkUrl:          kind = QString::fromUtf8("URL");          break;
		case etkOrganization: kind = QString::fromUtf8("Organization"); break;
		case etkIM:           kind = QString::fromUtf8("IM");           break;
		case etkNickname:     kind = QString::fromUtf8("Nickname");     break;
		case etkAnniversary:  kind = QString::fromUtf8("");             break;
		case etkPicture:      kind = QString::fromUtf8("Picture");      break;
		case etkUnknown:
		{
			return QString::fromUtf8("Unknown");
		}
	}
	switch (m_Usage)
	{
		case etuMobile:       return QString::fromUtf8("Mobile ") + kind;
		case etuHome:         return QString::fromUtf8("Home ") + kind;
		case etuWork:         return QString::fromUtf8("Work ") + kind;
		case etuFax:          return QString::fromUtf8("Fax ") + kind;
		case etuPager:        return QString::fromUtf8("Pager ") + kind;
		case etuInternetCall: return QString::fromUtf8("Internet call ") + kind;
		case etuBirthday:     return QString::fromUtf8("Birthday");
		case etuNameday:      return QString::fromUtf8("Nameday");
		case etuAIM:          return QString::fromUtf8("AIM ID");
		case etuWindowsLive:  return QString::fromUtf8("Windows Live ID");
		case etuYahoo:        return QString::fromUtf8("Yahoo ID");
		case etuSkype:        return QString::fromUtf8("Skype Name");
		case etuQQ:           return QString::fromUtf8("QQ");
		case etuHangouts:     return QString::fromUtf8("Hangouts Name");
		case etuICQ:          return QString::fromUtf8("ICQ UIN");
		case etuJabber:       return QString::fromUtf8("Jabber Name");
		case etuCustomIM:     return QString::fromUtf8("IM");
		case etuImagePNG:     return QString::fromUtf8("PNG ") + kind;
		case etuImageJPG:     return QString::fromUtf8("JPG ") + kind;
		case etuImageGIF:     return QString::fromUtf8("GIF ") + kind;
		case etuNA:           return kind;
		case etuUnknown:      return QString::fromUtf8("Unknown");
	}
	assert(!"Unknown kind / usage combination");
	return QString::fromUtf8("Unknown kind / usage combination");
}





////////////////////////////////////////////////////////////////////////////////
// Contact:

Contact::Contact(const QString & a_DisplayName):
	m_DisplayName(a_DisplayName)
{
}

void Contact::addElement(Contact::Element::Kind a_Kind, Contact::Element::Usage a_Usage, const QString & a_Value)
{
	m_Elements.emplace_back(a_Kind, a_Usage, a_Value);
}






