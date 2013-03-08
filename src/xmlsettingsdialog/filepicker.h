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

#ifndef XMLSETTINGSDIALOG_FILEPICKER_H
#define XMLSETTINGSDIALOG_FILEPICKER_H
#include <QWidget>

class QLineEdit;
class QPushButton;

namespace LeechCraft
{
	class FilePicker : public QWidget
	{
		Q_OBJECT

		QLineEdit *LineEdit_;
		QPushButton *BrowseButton_;
		bool ClearOnCancel_;
		QString Filter_;
	public:
		enum class Type
		{
			ExistingDirectory,
			OpenFileName,
			SaveFileName
		};
	private:
		Type Type_;
	public:
		FilePicker (Type = Type::ExistingDirectory, QWidget* = 0);
		void SetText (QString);
		QString GetText () const;
		void SetClearOnCancel (bool);
		void SetFilter (const QString&);
	private slots:
		void chooseFile ();
	signals:
		void textChanged (const QString&);
	};
};

#endif

