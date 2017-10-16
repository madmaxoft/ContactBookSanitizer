#ifndef EXAMPLEDEVICE_H
#define EXAMPLEDEVICE_H





#include <QCoreApplication>
#include "Device.h"





class ExampleDevice:
	public Device
{
	Q_DECLARE_TR_FUNCTIONS(ExampleDevice)
	using Super = Device;

public:
	ExampleDevice();


protected:

	/** The example contact books that this device represents. */
	std::vector<ContactBookPtr> m_ExampleContactBooks;


	// Device overrides:
	virtual QString displayName() const override;
	virtual void start(void) override;
	virtual void stop(void) override;
	virtual bool isOnline() const override;
	virtual const std::vector<ContactBookPtr> contactBooks() override;
	virtual bool load(const QJsonObject & a_Config) override;
	virtual QJsonObject save() const override;
};





#endif // EXAMPLEDEVICE_H
