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

#include "progressmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel)
	{
		Model_->setColumnCount (3);
	}

	QAbstractItemModel* ProgressManager::GetRepresentation () const
	{
		return Model_;
	}

	void ProgressManager::AddAccount (Account *acc)
	{
		connect (acc,
				SIGNAL (gotProgressListener (ProgressListener_g_ptr)),
				this,
				SLOT (handlePL (ProgressListener_g_ptr)));
	}

	void ProgressManager::handlePL (ProgressListener_g_ptr pl)
	{
		if (!pl)
			return;

		connect (pl,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handlePLDestroyed (QObject*)));
		connect (pl,
				SIGNAL (progress (int, int)),
				this,
				SLOT (handleProgress (int, int)));

		QList<QStandardItem*> row;
		row << new QStandardItem (pl->GetContext ());
		row << new QStandardItem (tr ("Running..."));
		row << new QStandardItem (QString (""));
		Model_->appendRow (row);

		Listener2Row_ [pl] = row.last ();
	}

	void ProgressManager::handlePLDestroyed (QObject *obj)
	{
		QStandardItem *item = Listener2Row_.take (obj);
		if (!item)
			return;

		Model_->removeRow (item->row ());
	}

	void ProgressManager::handleProgress (const int done, const int total)
	{
		auto item = Listener2Row_.value (sender ());
		if (!item)
			return;

		item->setText (QString ("%1/%2").arg (done).arg (total));
	}
}
}
