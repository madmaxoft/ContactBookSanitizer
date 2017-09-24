#include "Contact.h"
#include <algorithm>





Contact::Contact(const QString & a_DisplayName):
	m_DisplayName(a_DisplayName)
{
	setElement("FN", m_DisplayName.toStdString());
}





void Contact::setElement(const std::string & a_Name, const std::string & a_Value)
{
	// Store the element:
	m_Elements[a_Name] = a_Value;

	// Mirror into member variable, if appropriate:
	auto mvar = internalMemberForElement(a_Name);
	if (mvar != nullptr)
	{
		*mvar = QString::fromStdString(a_Value);
	}
}





QString * Contact::internalMemberForElement(const std::string & a_ElementName)
{
	if (a_ElementName.empty())
	{
		return nullptr;
	}

	// Poor-man's LowerCase is sufficient - vCard common names are ASCII-only
	std::string lcElementName = a_ElementName;
	std::transform(lcElementName.begin(), lcElementName.end(), lcElementName.begin(), ::tolower);

	// Try to match known elements:
	switch (lcElementName[0])
	{
		case 'f':
		{
			if (lcElementName == "fn")
			{
				return &m_DisplayName;
			}
			return nullptr;
		}
	}
	return nullptr;
}
