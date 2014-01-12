/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QObject>
#include <QDir>
#include <libimobiledevice/afc.h>
#include <interfaces/lmp/iunmountablesync.h>
#include "mobileraii.h"

template<typename T>
class QFutureWatcher;

namespace LeechCraft
{
namespace LMP
{
struct UnmountableFileInfo;

namespace jOS
{
	class GpodDb;
	struct UploadResult;

	class Connection : public QObject
	{
		Q_OBJECT

		const MobileRaii<idevice_t> Device_;
		const MobileRaii<lockdownd_client_t> Lockdown_;
		const MobileRaii<lockdownd_service_descriptor_t> Service_;
		const MobileRaii<afc_client_t> AFC_;

		const QString TempDirPath_;

		bool CopiedDb_ = false;

		GpodDb *DB_ = nullptr;

		struct QueueItem
		{
			QString LocalPath_;
			UnmountableFileInfo Info_;
		};
		QList<QueueItem> UploadQueue_;

		QFutureWatcher<UploadResult> *CurUpWatcher_ = nullptr;
	public:
		enum CopyOption
		{
			NoCopyOption = 0x0,
			CopyCreate = 0x1
		};
		Q_DECLARE_FLAGS (CopyOptions, CopyOption)

		Connection (const QByteArray&);

		afc_client_t GetAFC () const;

		QString GetFileInfo (const QString&, const QString&) const;
		bool Exists (const QString&);
	private:
		QString GetNextFilename (const QString&);

		QStringList ReadDir (const QString&, QDir::Filters);
		bool DownloadDir (const QString&, CopyOptions = NoCopyOption);
		bool DownloadFile (const QString&);
		bool MkDir (const QString&);
		bool UploadDir (const QString&);
		bool UploadFile (const QString&);
		bool UploadDatabase ();
	private slots:
		void itdbCopyFinished ();
		void rotateUploadQueue ();
		void handleTrackUploaded ();
		void handleDbLoaded ();
	signals:
		void error (const QString&);
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};

	typedef std::shared_ptr<Connection> Connection_ptr;
}
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::LMP::jOS::Connection::CopyOptions)
