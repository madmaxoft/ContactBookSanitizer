#ifndef DLGADDDEVICE_H
#define DLGADDDEVICE_H





#include <memory>

#include <QWizard>
#include <QJsonObject>
#include <QStandardItemModel>





// fwd:
class Session;
class QItemSelection;
class QLineEdit;





class DlgAddDevice: public QWizard
{
	Q_OBJECT
	using Super = QWizard;


public:

	/** Enumeration of all the wizard pages' IDs. */
	enum
	{
		pgDeviceType = 1,
		pgVcfFileName,
	};


	/** Creates a new wizard dialog. */
	DlgAddDevice();

	/** Displays the wizard, and if it successfully finishes, creates the new device in a_Session. */
	void show(Session & a_Session);

protected:

	/** The configuration for the newly created device.
	This will be filled by the individual subpages of the wizard. */
	QJsonObject m_DeviceConfig;
};




/** Wizard page: select the device type. */
class WizDeviceType:
	public QWizardPage
{
	Q_OBJECT
	using Super = QWizardPage;


public:

	/** Creates an instance of this page.
	a_DeviceConfig is the JSON config shared with the parent wizard, where the data is to be written. */
	WizDeviceType(QJsonObject & a_DeviceConfig);

	// QWizardPage overrides:
	virtual int nextId() const override;
	virtual bool isComplete() const override;

protected:

	/** The configuration for the newly created device, shared from the main wizard object. */
	QJsonObject & m_DeviceConfig;

	/** The model used for displaying the device types. */
	QStandardItemModel m_Model;


	/** Sets the device type from the specified item into m_DeviceConfig. */
	void setDeviceTypeFromModel(const QModelIndex & a_ModelIndex);

private slots:

	/** Triggered by the device type view when the selected device type is changed. */
	void selectionChanged(const QItemSelection & a_Selected, const QItemSelection & a_Deselected);

	/** Uses the specified item as the device type and advances the wizard. */
	void doubleClickedItem(const QModelIndex & a_ModelIndex);
};





/** Wizard page: VCF file name */
class WizVcfFile:
	public QWizardPage
{
	Q_OBJECT
	using Super = QWizardPage;


public:

	/** Creates an instance of this page.
	a_DeviceConfig is the JSON config shared with the parent wizard, where the data is to be written. */
	WizVcfFile(QJsonObject & a_DeviceConfig);

	// QWizardPage overrides:
	virtual bool isComplete() const override;

protected:

	/** The configuration for the newly created device, shared from the main wizard object. */
	QJsonObject & m_DeviceConfig;

	/** The model used for displaying the device types. */
	QStandardItemModel m_Model;

	/** The edit box for the file name. */
	QLineEdit * m_leFileName;

private slots:

	/** Triggered by the Browse button. */
	void browsePressed();

	/** Saves the filename from m_leFileName into m_DeviceConfig. */
	void fileNameChanged();
};





#endif // DLGADDDEVICE_H
