#include "DavPropertyHandlers.h"





DavPropertyHandlers & DavPropertyHandlers::get()
{
	static DavPropertyHandlers instance;
	return instance;
}





void DavPropertyHandlers::internalAdd(const QString & a_Namespace, const QString & a_LocalName, std::shared_ptr<DavPropertyTree::Property> a_Handler)
{
	m_Handlers[a_Namespace][a_LocalName] = a_Handler;
}





std::shared_ptr<DavPropertyTree::Property> DavPropertyHandlers::internalFind(const QString & a_Namespace, const QString & a_LocalName) const
{
	auto itrN = m_Handlers.find(a_Namespace);
	if (itrN == m_Handlers.end())
	{
		// Namespace not found
		return nullptr;
	}
	auto itrL = itrN->find(a_LocalName);
	if (itrL == itrN->end())
	{
		// LocalName not found
		return nullptr;
	}
	return *itrL;
}
