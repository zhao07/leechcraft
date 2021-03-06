/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "categoryselector.h"
#include <algorithm>
#include <QStringList>
#include <QCheckBox>
#include <QVariant>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QtDebug>

using namespace LeechCraft::Util;
const int RoleTag = 52;

CategorySelector::CategorySelector (QWidget *parent)
: QTreeWidget (parent)
, Separator_ ("; ")
{
	setWindowTitle (tr ("Tags selector"));
	setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint);
	setRootIsDecorated (false);
	setUniformRowHeights (true);

	QRect avail = QApplication::desktop ()->availableGeometry (this);
	setMinimumHeight (avail.height () / 3 * 2);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	QAction *all = new QAction (tr ("Select all"), this);
	connect (all,
			SIGNAL (triggered ()),
			this,
			SLOT (selectAll ()));

	QAction *none = new QAction (tr ("Select none"), this);
	connect (none,
			SIGNAL (triggered ()),
			this,
			SLOT (selectNone ()));

	addAction (all);
	addAction (none);

	setContextMenuPolicy (Qt::ActionsContextMenu);
}

void CategorySelector::SetCaption (const QString& caption)
{
	setHeaderLabel (caption);
	Caption_ = caption;
}

void CategorySelector::setPossibleSelections (QStringList mytags)
{
	disconnect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	clear ();

	mytags.sort ();
	QList<QTreeWidgetItem*> items;
	for (auto i = mytags.begin (), end = mytags.end (); i != end; ++i)
	{
		if (i->isEmpty ())
			continue;

		auto item = new QTreeWidgetItem (QStringList (*i));
		item->setCheckState (0, Qt::Unchecked);
		item->setData (0, RoleTag, *i);
		items << item;
	}
	addTopLevelItems (items);

	setHeaderLabel (Caption_);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	emit tagsSelectionChanged (QStringList ());
}

QStringList CategorySelector::GetSelections ()
{
	QStringList tags;

	for (int i = 0, size = topLevelItemCount ();
			i < size; ++i)
	{
		QTreeWidgetItem *item = topLevelItem (i);
		if (item->checkState (0) == Qt::Checked)
			tags += item->data (0, RoleTag).toString ();
	}

	return tags;
}

void CategorySelector::SetSelections (const QStringList& tags)
{
	blockSignals (true);
	for (int i = 0; i < topLevelItemCount (); ++i)
	{
		Qt::CheckState state =
			tags.contains (topLevelItem (i)->data (0, RoleTag).toString ()) ?
				Qt::Checked :
				Qt::Unchecked;
		topLevelItem (i)->setCheckState (0, state);
	}
	blockSignals (false);
}

QString CategorySelector::GetSeparator () const
{
	return Separator_;
}

void CategorySelector::SetSeparator (const QString& sep)
{
	Separator_ = sep;
}

void CategorySelector::moveEvent (QMoveEvent *e)
{
	QWidget::moveEvent (e);
	QPoint pos = e->pos ();
	QRect avail = QApplication::desktop ()->availableGeometry (this);
	int dx = 0, dy = 0;
	if (pos.x () + width () > avail.width ())
		dx = width () + pos.x () - avail.width ();
	if (pos.y () + height () > avail.height () &&
			height () < avail.height ())
		dy = height () + pos.y () - avail.height ();

	if (dx || dy)
		move (pos - QPoint (dx, dy));
}

void CategorySelector::selectAll ()
{
	disconnect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	QStringList tags;

	for (int i = 0, size = topLevelItemCount (); i < size; ++i)
	{
		QTreeWidgetItem *item = topLevelItem (i);
		item->setCheckState (0, Qt::Checked);
		tags += item->data (0, RoleTag).toString ();
	}

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	emit tagsSelectionChanged (tags);
}

void CategorySelector::selectNone ()
{
	disconnect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	for (int i = 0; i < topLevelItemCount (); ++i)
		topLevelItem (i)->setCheckState (0, Qt::Unchecked);

	connect (this,
			SIGNAL (itemChanged (QTreeWidgetItem*, int)),
			this,
			SLOT (buttonToggled ()));

	emit tagsSelectionChanged (QStringList ());
}

void CategorySelector::lineTextChanged (const QString& text)
{
	QStringList tags = text.split (Separator_, QString::SkipEmptyParts);
	SetSelections (tags);
}

void CategorySelector::buttonToggled ()
{
	emit tagsSelectionChanged (GetSelections ());
}

