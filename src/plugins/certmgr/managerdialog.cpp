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

#include "managerdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include "manager.h"
#include "certsmodel.h"

namespace LeechCraft
{
namespace CertMgr
{
	ManagerDialog::ManagerDialog (Manager *manager, QWidget *parent)
	: QDialog { parent }
	, Manager_ { manager }
	{
		Ui_.setupUi (this);

		Ui_.SystemTree_->setModel (manager->GetSystemModel ());

		connect (Ui_.SystemTree_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (updateSystemButtons ()));
		updateSystemButtons ();
	}

	QSslCertificate ManagerDialog::GetSelectedCert (CertPart part) const
	{
		QTreeView *view = nullptr;
		switch (part)
		{
		case CertPart::System:
			view = Ui_.SystemTree_;
			break;
		case CertPart::Local:
			view = Ui_.LocalTree_;
			break;
		}

		if (!view)
			return {};

		const auto& selected = view->selectionModel ()->selectedRows ();
		return selected.value (0).data (CertsModel::CertificateRole).value<QSslCertificate> ();
	}

	void ManagerDialog::on_AddLocal__released ()
	{
		const auto& paths = QFileDialog::getOpenFileNames (this,
				tr ("Select certificate files"),
				QDir::homePath (),
				"Certificates (*.crt);;All files (*)");
		if (paths.isEmpty ())
			return;

		QList<QSslCertificate> certs;
		for (const auto& path : paths)
		{
			certs << QSslCertificate::fromPath (path, QSsl::Pem);
			certs << QSslCertificate::fromPath (path, QSsl::Der);
		}

		const auto numAdded = Manager_->AddCerts (certs);

		if (paths.size () > 1 ||
				QFileInfo { paths.value (0) }.isDir ())
			QMessageBox::information (this,
					tr ("Certificates import"),
					tr ("%n certificate(s) were added.", nullptr, numAdded));
	}

	void ManagerDialog::on_RemoveLocal__released ()
	{
		Manager_->RemoveCert (GetSelectedCert (CertPart::Local));
	}

	void ManagerDialog::on_Enable__released ()
	{
		Manager_->ToggleBlacklist (GetSelectedCert (CertPart::System), false);
		updateSystemButtons ();
	}

	void ManagerDialog::on_Disable__released ()
	{
		Manager_->ToggleBlacklist (GetSelectedCert (CertPart::System), true);
		updateSystemButtons ();
	}

	void ManagerDialog::updateSystemButtons ()
	{
		const auto& cert = GetSelectedCert (CertPart::System);

		if (cert.isNull ())
		{
			Ui_.Enable_->setEnabled (false);
			Ui_.Disable_->setEnabled (false);
			return;
		}

		const auto isBl = Manager_->IsBlacklisted (cert);
		Ui_.Enable_->setEnabled (isBl);
		Ui_.Disable_->setEnabled (!isBl);
	}
}
}
