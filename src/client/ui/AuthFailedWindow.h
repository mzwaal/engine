/**
 * @file
 */

#pragma once

#include "ui/turbobadger/Window.h"
#include "core/Common.h"

namespace frontend {

class AuthFailedWindow: public ui::turbobadger::Window {
public:
	AuthFailedWindow(Window* parent) :
			ui::turbobadger::Window(parent) {
		core_assert_always(loadResourceFile("ui/window/client-authfailed.tb.txt"));
	}

	bool OnEvent(const tb::TBWidgetEvent &ev) override {
		if (ev.type == tb::EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("ok")) {
			Close();
			return true;
		}
		return ui::turbobadger::Window::OnEvent(ev);
	}
};

}
