#include "HorizontalContactView.h"
#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QElapsedTimer>





/** Returns the row height appropriate for the specified linespacing of the font. */
inline static int lineSpacingToRowHeight(int a_LineSpacing)
{
	/* The row height needs to include the font's linespacing
	and a space for at least two more pixels, scaled by the font size.
	2 pixels for the minimum font size of 10 pixels. */
	return a_LineSpacing + 2 * a_LineSpacing / 10;
}





HorizontalContactView::HorizontalContactView(QWidget * a_Parent):
	Super(a_Parent),
	m_Header(Qt::Horizontal, this),
	m_RowHeight(0),
	m_TitleRowHeight(0),
	m_GeometryRecursionBlock(false)
{
	// Set the default fonts:
	QFont fntLabel;
	setLabelFont(fntLabel);
	QFont fntValue;
	fntValue.setBold(true);
	setValueFont(fntValue);
	QFont fntTitle;
	// Make the title font 25% larger by default:
	if (fntTitle.pixelSize() > 0)
	{
		// The font was created using PixelSize
		fntTitle.setPixelSize(fntTitle.pixelSize() * 5 / 4);
	}
	else
	{
		// The font was created using PointSize
		fntTitle.setPointSizeF(fntTitle.pointSizeF() * 5 / 4);
	}
	setTitleFont(fntTitle);
	QFontMetrics fm(fntLabel);
	m_LabelWidth = fm.width("Default label width");
	m_ValueWidth = fm.width("Default value width is larger");

	m_HeaderModel.setHorizontalHeaderItem(0, new QStandardItem);
	m_HeaderModel.setHorizontalHeaderItem(1, new QStandardItem);
	m_Header.setModel(&m_HeaderModel);
	m_Header.resizeSection(0, m_LabelWidth);
	m_Header.resizeSection(1, m_ValueWidth);
	connect(&m_Header,             &QHeaderView::sectionResized, this, &HorizontalContactView::headerSectionResized);
	connect(horizontalScrollBar(), &QScrollBar::valueChanged,    this, &HorizontalContactView::horizontalScrollBarValueChanged);
}





void HorizontalContactView::setContactBook(ContactBookPtr a_ContactBook)
{
	m_CurrentLayout.clear();
	m_HeaderModel.clear();
	m_HeaderModel.setHorizontalHeaderItem(0, new QStandardItem);
	m_HeaderModel.setHorizontalHeaderItem(1, new QStandardItem);
	m_Header.resizeSection(0, m_LabelWidth);
	m_Header.resizeSection(1, m_ValueWidth);
	m_ContactBook = a_ContactBook;
	parseContacts();
	viewport()->update();
}





void HorizontalContactView::setLabelFont(const QFont & a_LabelFont)
{
	m_LabelFont = a_LabelFont;
	QFontMetrics fm(m_LabelFont);
	auto rh = lineSpacingToRowHeight(fm.lineSpacing());
	if (rh == m_RowHeight)
	{
		// The linespacing is the same as before, or smaller, no need to recalculate layout
		return;
	}
	if (rh > m_RowHeight)
	{
		// This change forces a higher row, need to recalculate layout
		m_RowHeight = rh;
		recalculateLayout();
		return;
	}

	// The linespacing got smaller, need to check ValueFont whether RowHeight can be decreased:
	QFontMetrics fmValueFont(m_ValueFont);
	auto rhValue = lineSpacingToRowHeight(fmValueFont.lineSpacing());
	if (std::max(rhValue, rh) == m_RowHeight)
	{
		// No change in the row height
		return;
	}
	m_RowHeight = std::max(rhValue, rh);
	recalculateLayout();
}





void HorizontalContactView::setValueFont(const QFont & a_ValueFont)
{
	m_ValueFont = a_ValueFont;
	QFontMetrics fm(m_ValueFont);
	auto rh = lineSpacingToRowHeight(fm.lineSpacing());
	if (rh == m_RowHeight)
	{
		// The linespacing is the same as before, or smaller, no need to recalculate layout
		return;
	}
	if (rh > m_RowHeight)
	{
		// This change forces a higher row, need to recalculate layout
		m_RowHeight = rh;
		recalculateLayout();
		return;
	}

	// The linespacing got smaller, need to check LabelFont whether RowHeight can be decreased:
	QFontMetrics fmLabelFont(m_LabelFont);
	auto rhLabel = lineSpacingToRowHeight(fmLabelFont.lineSpacing());
	if (std::max(rhLabel, rh) == m_RowHeight)
	{
		// No change in the row height
		return;
	}
	m_RowHeight = std::max(rhLabel, rh);
	recalculateLayout();
}





void HorizontalContactView::setTitleFont(const QFont & a_TitleFont)
{
	m_TitleFont = a_TitleFont;
	QFontMetrics fm(m_TitleFont);
	auto rh = lineSpacingToRowHeight(fm.lineSpacing());
	if (rh == m_TitleRowHeight)
	{
		// The row height has no change, no need to recalculate layout
		return;
	}
	m_TitleRowHeight = rh;
	recalculateLayout();
}





void HorizontalContactView::parseContacts()
{
	// Parse the contacts into displayable items:
	m_DisplayContacts.clear();
	if (m_ContactBook == nullptr)
	{
		return;
	}
	QElapsedTimer timer;
	timer.start();
	for (const auto & contact: m_ContactBook->contacts())
	{
		m_DisplayContacts.push_back(DisplayContact::fromContact(*contact));
	}
	qDebug() << __FUNCTION__ << ": Parsing into DisplayContact took " << timer.restart() << " msec.";

	// Sort:
	std::sort(m_DisplayContacts.begin(), m_DisplayContacts.end(),
		[](DisplayContactPtr a_Contact1, DisplayContactPtr a_Contact2)
		{
			return (a_Contact1->displayName() < a_Contact2->displayName());
		}
	);

	// Update contact card layout:
	recalculateLayout();
}





void HorizontalContactView::recalculateLayout()
{
	m_CurrentLayout.clear();
	int currentTop = 0;
	int viewportHeight = viewport()->height();
	int currentColumn = 0;
	int maxHeight = 0;
	for (const auto & dc: m_DisplayContacts)
	{
		int height = getContactHeight(dc);
		if ((currentTop > 0) && (currentTop + height > viewportHeight))
		{
			currentColumn += 1;
			currentTop = 0;
		}
		m_CurrentLayout.emplace_back(currentColumn, currentTop, dc);
		currentTop += height;
		if (currentTop > maxHeight)
		{
			maxHeight = currentTop;
		}
		currentTop += 2;
	}

	// Set the scrollbars:
	auto cardWidth = m_LabelWidth + m_ValueWidth;
	auto hsb = horizontalScrollBar();
	auto contentWidth = (currentColumn + 1) * cardWidth;
	if (contentWidth > viewport()->width())
	{
		hsb->setMaximum(contentWidth - viewport()->width());
		hsb->setPageStep(viewport()->width());
	}
	else
	{
		hsb->setValue(0);
		hsb->setMaximum(0);
		hsb->setPageStep(0);
	}
	auto vsb = verticalScrollBar();
	if (maxHeight < viewport()->height())
	{
		vsb->setValue(0);
		vsb->setMaximum(0);
		vsb->setPageStep(0);
	}
	else
	{
		vsb->setMaximum(maxHeight - viewport()->height());
		vsb->setPageStep(viewport()->height());
	}

	// Set the header:
	int numColumns = std::max(currentColumn, 1);
	m_HeaderModel.setColumnCount(numColumns * 2);
	for (int i = 0; i < numColumns; i += 2)
	{
		m_HeaderModel.setHorizontalHeaderItem(i,     new QStandardItem);
		m_HeaderModel.setHorizontalHeaderItem(i + 1, new QStandardItem);
		m_Header.resizeSection(i,     m_LabelWidth);
		m_Header.resizeSection(i + 1, m_ValueWidth);
	}

	// Force a re-draw
	viewport()->update();
}





int HorizontalContactView::getContactHeight(const DisplayContactPtr & a_Contact)
{
	int numValues = static_cast<int>(a_Contact->items().size());
	if (a_Contact->hasPicture())
	{
		// A picture is worth 4 item lines
		numValues = std::max(numValues, 4);
	}
	return m_TitleRowHeight + numValues * m_RowHeight;
}




void HorizontalContactView::paintCard(
	QPainter & a_Painter,
	const HorizontalContactView::ContactLayout & a_Card,
	int a_HorzOffset,
	int a_VertOffset
)
{
	// Draw the card outline:
	auto columnWidth = m_LabelWidth + m_ValueWidth;
	QRect cardRect(
		a_Card.m_Column * columnWidth - a_HorzOffset, a_Card.m_Top - a_VertOffset,
		columnWidth - 2, getContactHeight(a_Card.m_Contact)
	);
	a_Painter.setBrush(QBrush(QColor(Qt::white)));
	a_Painter.setPen(QPen(QColor(0xefefff)));
	a_Painter.drawRect(cardRect);

	// Draw the title:
	a_Painter.setFont(m_TitleFont);
	QRect titleRect(
		cardRect.x() + 1, cardRect.y() + 1,
		columnWidth - 3, m_TitleRowHeight - 2
	);
	a_Painter.setBrush(QBrush(QColor(0xefefff)));
	a_Painter.drawRect(titleRect);
	a_Painter.setPen(QPen(Qt::SolidLine));
	a_Painter.drawText(titleRect, a_Card.m_Contact->displayName());

	// Draw the picture, if it exists:
	int top = cardRect.y() + m_TitleRowHeight;
	if (a_Card.m_Contact->hasPicture())
	{
		QRect pictureRect(
			cardRect.x() + 1, top,
			m_RowHeight * 3, m_RowHeight * 4
		);
		a_Painter.drawPixmap(pictureRect, a_Card.m_Contact->picture());
	}

	// Draw the items:
	auto labelLeft = cardRect.x() + 3 + m_RowHeight * 3;
	auto actualLabelWidth = m_LabelWidth - m_RowHeight * 3 - 5;
	auto valueLeft = cardRect.x() + m_LabelWidth;
	for (const auto & item: a_Card.m_Contact->items())
	{
		QRect labelRect(labelLeft, top, actualLabelWidth, m_RowHeight - 2);
		a_Painter.setFont(m_LabelFont);
		a_Painter.drawText(labelRect, item->m_Label);
		QRect valueRect(valueLeft, top, m_ValueWidth - 3, m_RowHeight - 2);
		a_Painter.setFont(m_ValueFont);
		a_Painter.drawText(valueRect, item->m_Values[0]);
		top += m_RowHeight;
	}
}





void HorizontalContactView::paintEvent(QPaintEvent * a_Event)
{
	auto horzValue = horizontalScrollBar()->value();
	auto vertValue = verticalScrollBar()->value();
	auto columnWidth = m_LabelWidth + m_ValueWidth;
	QPainter painter(viewport());
	painter.setPen(QPen(Qt::NoPen));
	painter.setBrush(QBrush(QColor(0xffffff)));
	painter.drawRect(a_Event->rect());
	auto leftLimit = a_Event->rect().x() + horzValue;
	auto rightLimit = a_Event->rect().right() + horzValue;
	for (const auto & lay: m_CurrentLayout)
	{
		auto colLeft = lay.m_Column * columnWidth;
		if (colLeft + columnWidth < leftLimit)
		{
			// This column is too much to the left
			continue;
		}
		if (colLeft > rightLimit)
		{
			// Already painted all visible items
			return;
		}
		paintCard(painter, lay, horzValue, vertValue);
	}
}





void HorizontalContactView::resizeEvent(QResizeEvent * a_Event)
{
	Super::resizeEvent(a_Event);

	updateInternalGeometry();
	recalculateLayout();  // Updates the viewport as well
}





void HorizontalContactView::updateInternalGeometry()
{
	if (m_GeometryRecursionBlock)
	{
		return;
	}
	m_GeometryRecursionBlock = true;

	// Update the margins:
	int height = 0;
	height = qMax(m_Header.minimumHeight(), m_Header.sizeHint().height());
	height = qMin(height, m_Header.maximumHeight());
	setViewportMargins(0, height, 0, 0);  // May produce a nested resizeEvent() call

	// Update the header:
	auto vg = viewport()->geometry();
	int horizontalTop = vg.top() - height;
	m_Header.setGeometry(vg.left(), horizontalTop, vg.width(), height);

	m_GeometryRecursionBlock = false;
}





void HorizontalContactView::headerSectionResized(int a_ColumnIndex, int a_OldSize, int a_NewSize)
{
	Q_UNUSED(a_OldSize);

	auto oldColPos = (a_ColumnIndex / 2) * (m_LabelWidth + m_ValueWidth);
	if ((a_ColumnIndex % 2) == 1)
	{
		oldColPos += m_LabelWidth;
	}

	// Update the width:
	if ((a_ColumnIndex % 2) == 0)
	{
		m_LabelWidth = a_NewSize;
		for (int i = m_HeaderModel.columnCount() - 2; i >= 0; i -= 2)
		{
			m_Header.resizeSection(i, a_NewSize);
		}
	}
	else
	{
		m_ValueWidth = a_NewSize;
		for (int i = m_HeaderModel.columnCount() - 1; i >= 0; i -= 2)
		{
			m_Header.resizeSection(i, a_NewSize);
		}
	}


	// Update the scrollbar while attempting to keep the resized section at the same place:
	auto cardWidth = m_LabelWidth + m_ValueWidth;
	if (!m_CurrentLayout.empty())
	{
		auto totalWidth = cardWidth * (m_CurrentLayout.back().m_Column + 1);
		if (totalWidth <= viewport()->width())
		{
			horizontalScrollBar()->setMaximum(0);
		}
		else
		{
			horizontalScrollBar()->setMaximum(totalWidth - viewport()->width());
			auto oldPos = horizontalScrollBar()->value();
			auto newColPos = (a_ColumnIndex / 2) * (m_LabelWidth + m_ValueWidth);
			if ((a_ColumnIndex % 2) == 1)
			{
				newColPos += m_LabelWidth;
			}
			auto val = std::max(newColPos - oldColPos + oldPos, 0);
			horizontalScrollBar()->setValue(val);
			m_Header.setOffset(val);
		}
	}
	viewport()->update();
}





void HorizontalContactView::horizontalScrollBarValueChanged(int a_NewValue)
{
	m_Header.setOffset(a_NewValue);
}




