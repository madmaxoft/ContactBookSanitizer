#include "MainWindow.h"
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "Session.h"
#include "SessionModel.h"
#include "ContactBookModel.h"
#include "Device.h"
#include "DlgAddDevice.h"





MainWindow::MainWindow(std::unique_ptr<Session> && a_Session):
	Super(nullptr),
	m_UI(new Ui::MainWindow),
	m_Session(std::move(a_Session))
{
	m_UI->setupUi(this);

	// Add a decoration to the splitter handle to make it more visible:
	{
		auto handle = m_UI->splitHorz->handle(1);
		auto layout = new QVBoxLayout(handle);
		layout->setSpacing(0);
		layout->setMargin(0);
		auto line = new QFrame(handle);
		line->setFrameShape(QFrame::StyledPanel);
		line->setFrameShadow(QFrame::Sunken);
		layout->addWidget(line);
	}

	connectSignals();

	// Start the devices:
	m_Session->startDevices();

	// Set up the session display:
	if (m_SessionModel != nullptr)
	{
		disconnect(m_SessionModel.get(), nullptr, this, nullptr);  // Disconnect all signals from current session to this window
	}
	m_SessionModel.reset(new SessionModel(m_Session.get()));
	connect(m_SessionModel.get(), &SessionModel::deviceItemCreated, this, &MainWindow::expandDeviceItem);
	m_UI->tvSession->setModel(m_SessionModel.get());

	// Expand the online devices:
	auto onlineRoot = m_SessionModel->index(0, 0);
	m_UI->tvSession->expand(onlineRoot);
	for (int i = 0;; ++i)
	{
		auto mi = m_SessionModel->index(i, 0, onlineRoot);
		if (!mi.isValid())
		{
			break;
		}
		m_UI->tvSession->expand(mi);
	}

	// Set up the ContactBook display:
	m_ContactBookModel.reset(new ContactBookModel(nullptr));
	m_UI->tvContactBook->setModel(m_ContactBookModel.get());
}





MainWindow::~MainWindow()
{
	m_Session->stopDevices();
	m_Session->saveToFile();
}





void MainWindow::connectSignals()
{
	connect(m_UI->tvSession,       &QTreeView::activated, this, &MainWindow::sessionItemActivated);
	connect(m_UI->tvSession,       &QTreeView::clicked,   this, &MainWindow::sessionItemActivated);
	connect(m_UI->actDeviceAddNew, &QAction::triggered,   this, &MainWindow::addNewDevice);
	connect(m_UI->actDeviceDel,    &QAction::triggered,   this, &MainWindow::delDevice);
}





void MainWindow::sessionItemActivated(const QModelIndex & a_Index)
{
	m_ContactBookModel->setContactBook(m_SessionModel->getContactBook(a_Index));
	m_UI->tvContactBook->expandAll();
}





void MainWindow::addNewDevice()
{
	DlgAddDevice dlg;
	dlg.show(*m_Session);
}





void MainWindow::delDevice()
{
	auto dev = selectedDevice();
	if (dev == nullptr)
	{
		return;
	}

	// Ask the user:
	auto res = QMessageBox::question(this, tr("ContactBookSanitizer"),
		tr("Are you sure you want to remove the device %1?").arg(dev->displayName()),
		QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape
	);
	if (res != QMessageBox::Yes)
	{
		return;
	}

	m_Session->delDevice(dev);
}





Device * MainWindow::selectedDevice(void)
{
	auto sel = m_UI->tvSession->selectionModel()->selectedIndexes();
	if (sel.isEmpty())
	{
		return nullptr;
	}
	return m_SessionModel->deviceFromIndex(sel.at(0));
}





void MainWindow::expandDeviceItem(Device * a_Device, const QModelIndex & a_Index)
{
	Q_UNUSED(a_Device);

	m_UI->tvSession->expand(a_Index);
}





