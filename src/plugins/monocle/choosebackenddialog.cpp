/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "choosebackenddialog.h"
#include <interfaces/iinfo.h>

namespace LeechCraft
{
namespace Monocle
{
	ChooseBackendDialog::ChooseBackendDialog (const QList<QObject*>& backends, QWidget *parent)
	: QDialog (parent)
	, Backends_ (backends)
	{
		Ui_.setupUi (this);
		Q_FOREACH (auto backend, backends)
		{
			auto ii = qobject_cast<IInfo*> (backend);
			Ui_.BackendSelector_->addItem (ii->GetIcon (),
					QString ("%1 (%2)")
						.arg (ii->GetName ())
						.arg (ii->GetInfo ()));
		}
	}

	QObject* ChooseBackendDialog::GetSelectedBackend () const
	{
		return Backends_.value (Ui_.BackendSelector_->currentIndex ());
	}

	bool ChooseBackendDialog::GetRememberChoice () const
	{
		return Ui_.RememberBox_->checkState () == Qt::Checked;
	}
}
}
