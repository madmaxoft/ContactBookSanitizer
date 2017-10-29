#include "DlgAddDevice.h"
#include <assert.h>
#include <QDebug>
#include <QLabel>
#include <QListView>
#include <QVBoxLayout>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include "Session.h"
#include "Device.h"
#include "ui_WizAddDeviceCardDav.h"





////////////////////////////////////////////////////////////////////////////////
// DlgAddDevice:

DlgAddDevice::DlgAddDevice()
{
	setWindowTitle(tr("Add a new device"));

	// Add all the pages:
	setPage(pgDeviceType,    new WizDeviceType(m_DeviceConfig));
	setPage(pgVcfFileName,   new WizVcfFile(m_DeviceConfig));
	setPage(pgCardDavServer, new WizCardDav(m_DeviceConfig));

	setStartId(pgDeviceType);
}





void DlgAddDevice::show(Session & a_Session)
{
	// Show the wizard:
	m_DeviceConfig = QJsonObject();
	if (exec() != QDialog::Accepted)
	{
		return;
	}

	// Create the new device:
	auto dev = Device::createFromConfig(m_DeviceConfig);
	if (dev == nullptr)
	{
		qWarning() << "Failed to create a device from the following config:" << m_DeviceConfig;
		return;
	}

	// Add the device to the session:
	auto devPtr = dev.get();
	a_Session.addDevice(std::move(dev));
	devPtr->start();
}





////////////////////////////////////////////////////////////////////////////////
// WizDeviceType:

WizDeviceType::WizDeviceType(QJsonObject & a_DeviceConfig):
	m_DeviceConfig(a_DeviceConfig)
{
	setTitle(tr("Choose the type of device to add:"));

	// Add all the device types as items into the model:
	auto itemExample = new QStandardItem(tr("Example"));
	itemExample->setIcon(QIcon(":/res/DevTypeExample.png"));
	itemExample->setData("Example");
	m_Model.appendRow(itemExample);
	auto itemVcfFile = new QStandardItem(tr("VCF file"));
	itemVcfFile->setIcon(QIcon(":/res/DevTypeVcfFile.png"));
	itemVcfFile->setData("VcfFile");
	m_Model.appendRow(itemVcfFile);
	auto itemCardDav = new QStandardItem(tr("CardDAV server"));
	itemCardDav->setIcon(QIcon(":/res/DevTypeCardDav.png"));
	itemCardDav->setData("CardDav");
	m_Model.appendRow(itemCardDav);

	// Create the UI:
	auto verticalLayout = new QVBoxLayout(this);
	auto lvDeviceType = new QListView(this);
	lvDeviceType->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lvDeviceType->setSelectionMode(QAbstractItemView::SingleSelection);
	lvDeviceType->setSelectionBehavior(QAbstractItemView::SelectRows);
	lvDeviceType->setModel(&m_Model);
	verticalLayout->addWidget(lvDeviceType);
	setLayout(verticalLayout);

	// Connect the signals:
	connect(lvDeviceType->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WizDeviceType::selectionChanged);
	connect(lvDeviceType,                   &QListView::doubleClicked,              this, &WizDeviceType::doubleClickedItem);
}





int WizDeviceType::nextId() const
{
	auto devType = m_DeviceConfig["type"].toString();
	if (devType.isEmpty())
	{
		return -1;  // No continuation possible (isComplete() will block continuation)
	}
	if (devType == "Example")
	{
		return -1;  // No more pages
	}
	if (devType == "VcfFile")
	{
		return DlgAddDevice::pgVcfFileName;
	}
	if (devType == "CardDav")
	{
		return DlgAddDevice::pgCardDavServer;
	}
	assert(!"Unknown device type");
	return -1;
}





bool WizDeviceType::isComplete() const
{
	// Check that the DeviceType is known:
	auto devType = m_DeviceConfig["type"].toString();
	if (devType.isEmpty())
	{
		return false;
	}
	if (devType == "Example")
	{
		return true;
	}
	if (devType == "VcfFile")
	{
		return true;
	}
	if (devType == "CardDav")
	{
		return true;
	}
	assert(!"Unknown device type");
	return false;
}





void WizDeviceType::setDeviceTypeFromModel(const QModelIndex & a_ModelIndex)
{
	auto sel = m_Model.itemFromIndex(a_ModelIndex);
	auto devType = sel->data().toString();
	m_DeviceConfig["type"] = devType;
	if (devType == "Example")
	{
		setFinalPage(true);
	}
	else
	{
		setFinalPage(false);
	}
	emit completeChanged();
}





void WizDeviceType::selectionChanged(const QItemSelection & a_Selected, const QItemSelection & a_Deselected)
{
	Q_UNUSED(a_Deselected);

	// Set the DeviceType in device config based on the selection:
	const auto & indexes = a_Selected.indexes();
	if (indexes.isEmpty())
	{
		m_DeviceConfig["type"] = QString();
		return;
	}
	setDeviceTypeFromModel(indexes.at(0));
}





void WizDeviceType::doubleClickedItem(const QModelIndex & a_ModelIndex)
{
	if (!a_ModelIndex.isValid())
	{
		return;
	}
	setDeviceTypeFromModel(a_ModelIndex);
	if (wizard()->currentPage()->isFinalPage())
	{
		wizard()->accept();
	}
	else
	{
		wizard()->next();
	}
}





////////////////////////////////////////////////////////////////////////////////
// WizVcfFile:

WizVcfFile::WizVcfFile(QJsonObject & a_DeviceConfig):
	m_DeviceConfig(a_DeviceConfig),
	m_leFileName(new QLineEdit(this))
{
	setTitle(tr("Choose the VCF file:"));

	// Create the UI:
	auto layH = new QVBoxLayout(this);
	auto layV = new QHBoxLayout();
	layH->addItem(layV);
	m_leFileName->setText(m_DeviceConfig["type"].toString());
	layV->addWidget(m_leFileName);
	auto btnBrowse = new QPushButton(this);
	btnBrowse->setText(tr("Browse...", "VCF File"));
	layV->addWidget(btnBrowse);
	setLayout(layH);

	// Connect the signals:
	connect(btnBrowse,    &QPushButton::pressed,   this, &WizVcfFile::browsePressed);
	connect(m_leFileName, &QLineEdit::textChanged, this, &WizVcfFile::fileNameChanged);
	setFinalPage(true);
}





bool WizVcfFile::isComplete() const
{
	// Check that the file name is ok:
	auto fnam = m_DeviceConfig["fileName"].toString();
	if (fnam.isEmpty())
	{
		return false;
	}
	return QFile::exists(fnam);
}





void WizVcfFile::browsePressed()
{
	auto fileName = QFileDialog::getOpenFileName(
		nullptr,                // Parent widget
		tr("Open a VCF file"),  // Title
		QString(),              // Initial folder
		tr("VCF file (*.vcf)")  // Filter
	);
	if (fileName.isEmpty())
	{
		return;
	}
	m_leFileName->setText(fileName);
}





void WizVcfFile::fileNameChanged()
{
	m_DeviceConfig["fileName"] = m_leFileName->text();
	emit completeChanged();
}





////////////////////////////////////////////////////////////////////////////////
// WizCardDav:

WizCardDav::WizCardDav(QJsonObject & a_DeviceConfig):
	m_DeviceConfig(a_DeviceConfig),
	m_UI(new Ui::WizAddDeviceCardDav)
{
	m_UI->setupUi(this);
	setTitle(tr("CardDAV server connection settings"));
	setFinalPage(true);

	connect(m_UI->leDisplayName, &QLineEdit::textChanged, this, &WizCardDav::updateConfig);
	connect(m_UI->leServerUrl,   &QLineEdit::textChanged, this, &WizCardDav::updateConfig);
	connect(m_UI->leUserName,    &QLineEdit::textChanged, this, &WizCardDav::updateConfig);
	connect(m_UI->lePassword,    &QLineEdit::textChanged, this, &WizCardDav::updateConfig);
}





WizCardDav::~WizCardDav()
{
	// No explicit code needed, but the destructor still needs to be in the CPP file,
	// otherwise the m_UI destructor is generated in the header file and it doesn't have the UI class definition.
}





bool WizCardDav::isComplete() const
{
	return (
		!m_DeviceConfig["serverUrl"].toString().isEmpty() &&
		!m_DeviceConfig["userName"].toString().isEmpty() &&
		!m_DeviceConfig["password"].toString().isEmpty()
	);
}





void WizCardDav::updateConfig(const QString &)
{
	m_DeviceConfig["displayName"] = m_UI->leDisplayName->text();
	m_DeviceConfig["serverUrl"]   = m_UI->leServerUrl->text();
	m_DeviceConfig["userName"]    = m_UI->leUserName->text();
	m_DeviceConfig["password"]    = m_UI->lePassword->text();

	emit completeChanged();
}




