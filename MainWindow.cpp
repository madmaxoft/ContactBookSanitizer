#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Session.h"
#include "SessionModel.h"
#include "ContactBookModel.h"
#include "Device.h"





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
	m_SessionModel.reset(new SessionModel(m_Session.get()));
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
}





void MainWindow::connectSignals()
{
	connect(m_UI->tvSession, &QTreeView::activated, this, &MainWindow::sessionItemActivated);
	connect(m_UI->tvSession, &QTreeView::clicked,   this, &MainWindow::sessionItemActivated);
}





void MainWindow::sessionItemActivated(const QModelIndex & a_Index)
{
	m_ContactBookModel->setContactBook(m_SessionModel->getContactBook(a_Index));
	m_UI->tvContactBook->expandAll();
}





