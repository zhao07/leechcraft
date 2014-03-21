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

#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QSslCertificate>

namespace LeechCraft
{
namespace CertMgr
{
	class CertsModel : public QAbstractItemModel
	{
		typedef QList<QPair<QString, QList<QSslCertificate>>> CertsDict_t;
		CertsDict_t Issuer2Certs_;
	public:
		typedef std::shared_ptr<void> ModifyGuard_t;

		CertsModel (QObject* = 0);

		QModelIndex index (int row, int column, const QModelIndex& parent) const;
		QModelIndex parent (const QModelIndex& child) const;
		int columnCount (const QModelIndex& parent) const;
		int rowCount (const QModelIndex& parent) const;
		QVariant data (const QModelIndex& index, int role) const;

		void AddCert (const QSslCertificate&);
		void RemoveCert (const QSslCertificate&);

		void ResetCerts (const QList<QSslCertificate>&);
	private:
		CertsDict_t::iterator GetListPosForCert (const QSslCertificate&);
		CertsDict_t::const_iterator GetListPosForCert (const QSslCertificate&) const;

		CertsDict_t::iterator CreateListPosForCert (const QSslCertificate&);
		QList<QSslCertificate>& CreateListForCert (const QSslCertificate&);
	};
}
}
