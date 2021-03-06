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

#include <QObject>
#include <util/utilconfig.h>

class QTimer;

namespace LeechCraft
{
namespace Util
{
	/** @brief Allows to hide a widget or popup after mouse leave.
	 *
	 * This class is used to automatically hide a top-level widget like a
	 * tooltip or popup after mouse has left the widget and some time has
	 * passed.
	 *
	 * This class supports performing arbitrary operations in addition to
	 * destructing the widget, just pass the corresponding slot to call
	 * in the constructor. Despite this functionality, we will say
	 * "deletion" each time we speak of the operation without noticing
	 * that any other operation is possible on the widget.
	 *
	 * It is also possible to directly start the destruction timer using
	 * the Start() function, and to stop is with its counterpart Stop().
	 *
	 * The widget on which this mixin is constructed takes ownership of
	 * this class, so there is no need to keep it around or delete it
	 * explicitly.
	 */
	class UnhoverDeleteMixin : public QObject
	{
		Q_OBJECT

		QTimer *LeaveTimer_;
		bool ContainsMouse_;
		bool IgnoreNext_;
	public:
		/** @brief Creates the mixin for the given parent widget.
		 *
		 * @param[in] parent The widget for which should be watched for
		 * mouse leave events.
		 * @param[in] slot The slot to call when enough time has passed
		 * since mouse leave. By default it is <code>deleteLater()</code>.
		 */
		UTIL_API UnhoverDeleteMixin (QWidget *parent, const char *slot = SLOT (deleteLater ()));

		/** @brief Manually starts the timer.
		 *
		 * This function can be used to start the timer after a Stop().
		 *
		 * If the widget currently contains the mouse, this function does
		 * nothing.
		 *
		 * @param[in] timeout The number of milliseconds to wait before
		 * the widget is deleted..
		 *
		 * @sa Stop()
		 */
		void UTIL_API Start (int timeout = 1200);

		/** @brief Stops the previously started timer.
		 *
		 * This function stops the started destruction timer, both if it
		 * is started as the result of mouse leave or due to Start().
		 *
		 * After a Stop() the widget will never be deleted without mouse
		 * entering the widget and then leaving it again, or calling
		 * Start().
		 *
		 * This function is useful if a user is currently interacting
		 * with a logical child of the watched widget, and though the
		 * watched widget doesn't contain mouse at the moment, it still
		 * makes sense to keep it around.
		 *
		 * @sa Start()
		 */
		void UTIL_API Stop ();

		/** @brief Ignores the next leave event.
		 *
		 * This function is useful if one knows that the watched widget
		 * is going to be resized, for example, and needs to cancel the
		 * deletion of the widget upon receiving the corresponding
		 * leave event.
		 */
		void UTIL_API IgnoreNext ();
	protected:
		bool eventFilter (QObject*, QEvent*);
	};
}
}
