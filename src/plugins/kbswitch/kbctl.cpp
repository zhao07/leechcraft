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

#include "kbctl.h"
#include <QtDebug>
#include <QFile>
#include <util/x11/xwrapper.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

typedef bool (*QX11FilterFunction) (XEvent *event);
extern void qt_installX11EventFilter (QX11FilterFunction func);

namespace LeechCraft
{
namespace KBSwitch
{
	namespace
	{
		bool EventFilter (XEvent *msg)
		{
			return KBCtl::Instance ().Filter (msg);
		}
	}

	KBCtl::KBCtl ()
	{
		InitDisplay ();
		SetupRules ();

		XWindowAttributes wa;
		XGetWindowAttributes (Display_, Window_, &wa);
		const auto rootEvents = StructureNotifyMask |
				SubstructureNotifyMask |
				PropertyChangeMask |
				FocusChangeMask |
				KeymapStateMask |
				LeaveWindowMask |
				EnterWindowMask;
		XSelectInput (Display_, Window_, wa.your_event_mask | rootEvents);

		XkbSelectEventDetails (Display_, XkbUseCoreKbd,
				XkbStateNotify, XkbAllStateComponentsMask, XkbGroupStateMask);

		CheckExtWM ();

		if (!ExtWM_)
			SetupNonExtListeners ();

		UpdateGroupNames ();

		qt_installX11EventFilter (&EventFilter);
	}

	KBCtl& KBCtl::Instance ()
	{
		static KBCtl ctl;
		return ctl;
	}

	void KBCtl::Release ()
	{
		XCloseDisplay (Display_);
	}

	void KBCtl::SetSwitchPolicy (SwitchPolicy policy)
	{
		Policy_ = policy;
	}

	int KBCtl::GetCurrentGroup () const
	{
		XkbStateRec state;
		XkbGetState (Display_, XkbUseCoreKbd, &state);
		return state.group;
	}

	const QStringList& KBCtl::GetEnabledGroups () const
	{
		return Groups_;
	}

	int KBCtl::GetMaxEnabledGroups () const
	{
		return XkbNumKbdGroups;
	}

	QString KBCtl::GetLayoutName (int group) const
	{
		return Groups_.value (group);
	}

	QString KBCtl::GetLayoutDesc (int group) const
	{
		return LayName2Desc_ [GetLayoutName (group)];
	}

	const QHash<QString, QString> KBCtl::GetLayoutsD2N () const
	{
		return LayDesc2Name_;
	}

	const QHash<QString, QString> KBCtl::GetLayoutsN2D () const
	{
		return LayName2Desc_;
	}

	bool KBCtl::Filter (XEvent *event)
	{
		if (event->type == XkbEventType_)
		{
			HandleXkbEvent (event);
			return false;
		}

		switch (event->type)
		{
		case FocusIn:
		case FocusOut:
		case PropertyNotify:
			SetWindowLayout (Util::XWrapper::Instance ().GetActiveApp ());
			break;
		case CreateNotify:
		{
			const auto window = event->xcreatewindow.window;
			AssignWindow (window);
			break;
		}
		case DestroyNotify:
			Win2Group_.remove (event->xdestroywindow.window);
			break;
		}

		return false;
	}

	void KBCtl::HandleXkbEvent (XEvent *event)
	{
		XkbEvent xkbEv;
		xkbEv.core = *event;

		switch (xkbEv.any.xkb_type)
		{
		case XkbStateNotify:
			if (xkbEv.state.group == xkbEv.state.locked_group)
				Win2Group_ [Util::XWrapper::Instance ().GetActiveApp ()] = xkbEv.state.group;
			emit groupChanged (xkbEv.state.group);
			break;
		case XkbNewKeyboardNotify:
			Win2Group_.clear ();
			UpdateGroupNames ();
			break;
		default:
			break;
		}
	}

	void KBCtl::SetWindowLayout (Qt::HANDLE window)
	{
		if (Policy_ != SwitchPolicy::PerWindow)
			return;

		if (window == None)
			return;

		if (!Win2Group_.contains (window))
			return;

		const auto group = Win2Group_ [window];
		XkbLockGroup (Display_, XkbUseCoreKbd, group);
	}

	void KBCtl::InitDisplay ()
	{
		int xkbError, xkbReason;
		int mjr = XkbMajorVersion, mnr = XkbMinorVersion;
		Display_ = XkbOpenDisplay (nullptr,
				&XkbEventType_,
				&xkbError,
				&mjr,
				&mnr,
				&xkbReason);
		Window_ = DefaultRootWindow (Display_);

		NetActiveWinAtom_ = Util::XWrapper::Instance ().GetAtom ("_NET_ACTIVE_WINDOW");
	}

	void KBCtl::CheckExtWM ()
	{
		Atom type;
		int format;
		uchar *prop = nullptr;
		ulong count, after;
		const auto ret = XGetWindowProperty (Display_, Window_, NetActiveWinAtom_,
				0, sizeof (Window), 0, XA_WINDOW,
				&type, &format, &count, &after, &prop);
		if (ret == Success && prop)
		{
			XFree (prop);
			ExtWM_ = true;
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "extended window manager hints support wasn't detected, this won't work";
	}

	void KBCtl::SetupNonExtListeners ()
	{
		uint count = 0;
		Window d1, d2;
		Window *windows = nullptr;

		if (!XQueryTree (Display_, Window_, &d1, &d2, &windows, &count))
			return;

		for (uint i = 0; i < count; ++i)
			AssignWindow (windows [i]);

		if (windows)
			XFree (windows);
	}

	namespace
	{
		QString FindX11Dir ()
		{
			static const auto dirs =
			{
				"/etc/X11",
				"/usr/share/X11",
				"/usr/local/share/X11",
				"/usr/X11R6/lib/X11",
				"/usr/X11R6/lib64/X11",
				"/usr/local/X11R6/lib/X11",
				"/usr/local/X11R6/lib64/X11",
				"/usr/lib/X11",
				"/usr/lib64/X11",
				"/usr/local/lib/X11",
				"/usr/local/lib64/X11",
				"/usr/pkg/share/X11",
				"/usr/pkg/xorg/lib/X11"
			};

			for (const auto& item : dirs)
			{
				const QString itemStr (item);
				if (QFile::exists (itemStr + "/xkb/rules"))
					return itemStr;
			}

			return {};
		}

		QString FindRulesFile (Display *display)
		{
			const auto& x11dir = FindX11Dir ();

			XkbRF_VarDefsRec vd;
			char *path = 0;
			if (XkbRF_GetNamesProp (display, &path, &vd) && path)
			{
				const QString pathStr (path);
				free (path);
				return x11dir + "/xkb/rules/" + pathStr;
			}

			for (auto rfName : { "base", "xorg", "xfree86" })
			{
				const auto rf = QString ("/xkb/rules/") + rfName;
				const auto& path = x11dir + rf;
				if (QFile::exists (path))
					return path;
			}

			return {};
		}
	}

	void KBCtl::SetupRules ()
	{
		const auto& rf = FindRulesFile (Display_);
		if (rf.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "rules file wasn't found";
			return;
		}

		char *locale = { 0 };
		const auto xkbRules = XkbRF_Load (QFile::encodeName (rf).data (),
				locale, true, true);
		if (!xkbRules)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot load rules from"
					<< rf;
			return;
		}

		for (int i = 0; i < xkbRules->layouts.num_desc; ++i)
		{
			const auto& desc = xkbRules->layouts.desc [i];
			LayName2Desc_ [desc.name] = desc.desc;
			LayDesc2Name_ [desc.desc] = desc.name;
		}
	}

	void KBCtl::UpdateGroupNames ()
	{
		auto desc = XkbAllocKeyboard ();
		XkbGetControls (Display_, XkbAllControlsMask, desc);
		XkbGetNames (Display_, XkbSymbolsNameMask | XkbGroupNamesMask, desc);

		if (!desc->names || !desc->names->groups)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get names";
			return;
		}

		Groups_.clear ();

		const auto group = desc->names->groups;
		size_t groupCount = 0;
		for (; groupCount < XkbNumKbdGroups && group [groupCount]; ++groupCount) ;

		char **result = new char* [groupCount];
		XGetAtomNames (Display_, group, groupCount, result);
		for (size_t i = 0; i < groupCount; ++i)
		{
			Groups_ << LayDesc2Name_ [result [i]];
			XFree (result [i]);
		}
		delete [] result;

		qDebug () << Q_FUNC_INFO << Groups_;
	}

	void KBCtl::AssignWindow (Qt::HANDLE window)
	{
		if (ExtWM_)
			return;

		XWindowAttributes wa;
		if (!XGetWindowAttributes (Display_, window, &wa))
			return;

		const auto windowEvents = EnterWindowMask |
				FocusChangeMask |
				PropertyChangeMask |
				StructureNotifyMask;
		XSelectInput (Display_, window, windowEvents);
	}
}
}
