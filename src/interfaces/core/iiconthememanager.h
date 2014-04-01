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

#include <functional>
#include <QString>
#include <QList>

class QAction;
class QIcon;

class IIconThemeManager
{
protected:
	virtual ~IIconThemeManager () {}
public:
	/** @brief Returns the current theme's icon for the given on and off
	 * states.
	 *
	 * @param[in] on The name of the icon in the "on" state.
	 * @param[in] off The name of the icon in the "off" state, if any.
	 * @return The QIcon object created from image files which could be
	 * obtained via GetIconPath().
	 *
	 * @sa GetIconPath
	 */
	virtual QIcon GetIcon (const QString& on, const QString& off = QString ()) = 0;

	/** @brief Updates the icons of the given actions.
	 *
	 * This function sets or updates the icons of \em actions according
	 * to the current iconset. This function also registers the actions
	 * so that they are automatically updated when the iconset changes.
	 *
	 * @param[in] actions The list of actions to update.
	 */
	virtual void UpdateIconset (const QList<QAction*>& actions) = 0;

	/** @brief Watches the given widget recursively and its child actions.
	 *
	 * This function merely installs the event filter on the given widget
	 * to watch for new actions or action changes.
	 *
	 * @param[in] widget The widget to manage.
	 */
	virtual void ManageWidget (QWidget *widget) = 0;

	/** @brief Registers the theme change handler.
	 *
	 * The given \em function will be invoked after each icon theme change.
	 *
	 * @param[in] function The function to invoke after theme change.
	 */
	virtual void RegisterChangeHandler (const std::function<void ()>& function) = 0;
};

Q_DECLARE_INTERFACE (IIconThemeManager, "org.LeechCraft.IIconThemeManager/1.0");
