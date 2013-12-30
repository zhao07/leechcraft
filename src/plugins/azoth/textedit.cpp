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

#include "textedit.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QtDebug>
#include <util/shortcuts/shortcutmanager.h>
#include "chattab.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	TextEdit::TextEdit (QWidget *parent)
	: QTextEdit (parent)
	{
		auto wordShortcut = new QShortcut ({},
				this,
				SLOT (deleteWord ()));
		auto bolShortcut = new QShortcut (QString ("Ctrl+U"),
				this,
				SLOT (deleteBOL ()));
		auto eolShortcut = new QShortcut (QString ("Ctrl+K"),
				this,
				SLOT (deleteEOL ()));

		auto sm = Core::Instance ().GetShortcutManager ();
		sm->RegisterShortcut ("org.Azoth.TextEdit.DeleteWord",
				{ tr ("Delete the word before the cursor"), QKeySequence {}, {} },
				wordShortcut);
		sm->RegisterShortcut ("org.Azoth.TextEdit.DeleteBOL",
				{
					tr ("Delete from cursor to the beginning of line"),
					bolShortcut->key (),
					{}
				},
				bolShortcut);
		sm->RegisterShortcut ("org.Azoth.TextEdit.DeleteEOL",
				{
					tr ("Delete from cursor to the end of line"),
					eolShortcut->key (),
					{}
				},
				eolShortcut);
	}

	void TextEdit::keyPressEvent (QKeyEvent *event)
	{
		const QString& modOption = XmlSettingsManager::Instance ()
				.property ("SendOnModifier").toString ();
		Qt::KeyboardModifiers sendMod = Qt::NoModifier;
		if (modOption == "CtrlEnter")
			sendMod = Qt::ControlModifier;
		else if (modOption == "ShiftEnter")
			sendMod = Qt::ShiftModifier;

		const bool kpEnter = XmlSettingsManager::Instance ()
				.property ("KPEnterAlias").toBool ();
		const bool sendMsgButton = event->key () == Qt::Key_Return ||
				(kpEnter && event->key () == Qt::Key_Enter);
		const bool modifiersOk = event->modifiers () == sendMod ||
				(kpEnter && event->modifiers () == (sendMod | Qt::KeypadModifier));
		if (sendMsgButton && modifiersOk)
			emit keyReturnPressed ();
		else if (event->key () == Qt::Key_Tab)
		{
			if (event->modifiers () == Qt::NoModifier)
				emit keyTabPressed ();
			else
				event->ignore ();
		}
		else if (event->modifiers () & Qt::ShiftModifier &&
				(event->key () == Qt::Key_PageUp ||
				 event->key () == Qt::Key_PageDown))
			emit scroll (event->key () == Qt::Key_PageUp ? -1 : 1);
		else if (event->modifiers () == Qt::ControlModifier &&
				(event->key () >= Qt::Key_0 && event->key () <= Qt::Key_9))
			event->ignore ();
		else
		{
			emit clearAvailableNicks ();
			QTextEdit::keyPressEvent (event);
		}
	}

	void TextEdit::deleteWord ()
	{
		auto c = textCursor ();

		const auto pos = c.position ();
		c.movePosition (QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
		if (pos == c.position ())
			c.movePosition (QTextCursor::PreviousWord, QTextCursor::KeepAnchor);

		c.removeSelectedText ();
	}

	void TextEdit::deleteBOL ()
	{
		auto c = textCursor ();
		c.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
		c.removeSelectedText ();
	}

	void TextEdit::deleteEOL ()
	{
		auto c = textCursor ();
		c.movePosition (QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		c.removeSelectedText ();
	}
}
}
