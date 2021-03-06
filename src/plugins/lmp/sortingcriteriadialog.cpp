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

#include "sortingcriteriadialog.h"
#include <QInputDialog>
#include <QStandardItemModel>

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		enum Roles
		{
			CriteriaRole = Qt::UserRole + 1
		};
	}

	SortingCriteriaDialog::SortingCriteriaDialog (QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.CriteriaView_->setModel (Model_);
	}

	void SortingCriteriaDialog::SetCriteria (const QList<SortingCriteria>& criteria)
	{
		for (auto crit : criteria)
			AddCriteria (crit);
	}

	QList<SortingCriteria> SortingCriteriaDialog::GetCriteria () const
	{
		QList<SortingCriteria> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			result << static_cast<SortingCriteria> (Model_->item (i)->data (CriteriaRole).toInt ());
		return result;
	}

	void SortingCriteriaDialog::AddCriteria (SortingCriteria criteria)
	{
		auto item = new QStandardItem (GetCriteriaName (criteria));
		item->setData (static_cast<int> (criteria), CriteriaRole);
		Model_->appendRow (item);
	}

	void SortingCriteriaDialog::on_Add__released ()
	{
		const auto& criteria = GetAllCriteria ();
		QStringList items;
		for (const auto& item : criteria)
			items << GetCriteriaName (item);

		const auto& selected = QInputDialog::getItem (this,
				tr ("Select criteria"),
				tr ("Select criteria to be added:"),
				items,
				0,
				false);
		const auto& pos = items.indexOf (selected);
		if (pos < 0)
			return;

		AddCriteria (criteria.at (pos));
	}

	void SortingCriteriaDialog::on_Remove__released ()
	{
		const auto& current = Ui_.CriteriaView_->currentIndex ();
		if (!current.isValid ())
			return;

		Model_->removeRow (current.row ());
	}

	void SortingCriteriaDialog::on_MoveUp__released ()
	{
		const auto& current = Ui_.CriteriaView_->currentIndex ();
		const int row = current.row ();
		if (row <= 0)
			return;

		Model_->insertRow (row - 1, Model_->takeRow (row));
		const auto& newIdx = current.sibling (current.row () - 1, 0);
		Ui_.CriteriaView_->selectionModel ()->select (newIdx, QItemSelectionModel::ClearAndSelect);
		Ui_.CriteriaView_->setCurrentIndex (newIdx);
	}

	void SortingCriteriaDialog::on_MoveDown__released ()
	{
		const auto& current = Ui_.CriteriaView_->currentIndex ();
		if (!current.isValid ())
			return;

		const int row = current.row ();
		if (row >= Model_->rowCount () - 1)
			return;

		Model_->insertRow (row + 1, Model_->takeRow (row));
		const auto& newIdx = current.sibling (current.row () + 1, 0);
		Ui_.CriteriaView_->selectionModel ()->select (newIdx, QItemSelectionModel::ClearAndSelect);
		Ui_.CriteriaView_->setCurrentIndex (newIdx);
	}
}
}
