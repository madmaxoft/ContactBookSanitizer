#ifndef DAVPROPERTYHANDLERS_H
#define DAVPROPERTYHANDLERS_H





#include "DavPropertyTree.h"





/** Registry of the property handlers.
Implements a storage of DavPropertyTree::Property handlers tied to a specific property type (namespace, localname).
The entire class is intended as a singleton, with individual property handlers registering themselves at startup
and then the protocol parser using it in read-only mode. */
class DavPropertyHandlers
{
public:

	/** Helper class for registering handlers on startup (by creating a global variable of this type). */
	class Registrator
	{
	public:
		Registrator(
			const QString & a_Namespace,
			const QString & a_LocalName,
			std::shared_ptr<DavPropertyTree::Property> a_Handler
		)
		{
			DavPropertyHandlers::add(a_Namespace, a_LocalName, a_Handler);
		}
	};


	/** Registers a new handler for the specified property node type. */
	static void add(
		const QString & a_Namespace,
		const QString & a_LocalName,
		std::shared_ptr<DavPropertyTree::Property> a_Handler
	)
	{
		get().internalAdd(a_Namespace, a_LocalName, a_Handler);
	}

	/** Returns the handler for the specified property node type, or nullptr if no such handler known. */
	static std::shared_ptr<DavPropertyTree::Property> find(const QString & a_Namespace, const QString & a_LocalName)
	{
		return get().internalFind(a_Namespace, a_LocalName);
	}


protected:

	/** Type for storing the property handlers, map of Namespace -> {(}map of localname -> handler}.*/
	using HandlerMap = QHash<QString, QHash<QString, std::shared_ptr<DavPropertyTree::Property>>>;

	HandlerMap m_Handlers;


	/** Returns the singleton instance of this class. */
	static DavPropertyHandlers & get();

	/** Registers a new handler for the specified property node type.
	This is the real (non-static) implementation. */
	void internalAdd(
		const QString & a_Namespace,
		const QString & a_LocalName,
		std::shared_ptr<DavPropertyTree::Property> a_Handler
	);

	/** Returns the handler for the specified property node type, or nullptr if no such handler known.
	This is the real implementation, with const-guarantee on the storage object. */
	std::shared_ptr<DavPropertyTree::Property> internalFind(
		const QString & a_Namespace,
		const QString & a_LocalName
	) const;
};





#endif // DAVPROPERTYHANDLERS_H
