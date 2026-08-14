#pragma once

#ifdef _WIN32

// #include "framework.h"
#include <windows.h>
#include <objidl.h>
#include <propidl.h>
#include <gdiplus.h>
#include <commctrl.h>

#include <latch>
#include <memory>
#include <thread>

#include <functional>
#include <optional>
#include <vector>
#include <string>

#include "shark/log.hpp"

class termux_thread;

class termux
{
	friend class termux_thread;

	struct control;
	HWND hwnd_;
	termux_thread* termux_thread_;
	std::vector<std::unique_ptr<control>> controls_;
	int next_control_id_ = 1;
	int focused_control_id_ = 0;
	int hovered_control_id_ = 0;

	control* find_control(int id);
	int hit_test(POINT point) const;
	void paint(HDC hdc);
	bool mouse_move(POINT point);
	bool mouse_down(POINT point);
	bool mouse_up(POINT point);
	bool char_input(wchar_t character);
	bool key_down(WPARAM key);

public:
	using button_callback = std::function<void()>;

	termux(termux_thread* termux_thread, HWND hwnd);
	~termux();

	// void key_bind();
	void key_bind_hotkey(UINT fsModifiers, UINT vk, std::function<void(std::weak_ptr<termux>)>);
	// void key_bind_ll();

	// Controls are owner-drawn with GDI+. Rectangles use client-area pixels.
	int add_text_block(RECT bounds, std::wstring text);
	int add_button(RECT bounds, std::wstring text, button_callback on_click);
	int add_text_edit(RECT bounds, std::wstring text = L"", std::wstring placeholder = L"",
					  bool editable = true);
	std::wstring text_edit_value(int control_id) const;
	void set_text_edit_value(int control_id, std::wstring text);
	void redraw();

	termux(const termux&) = delete;
	termux& operator=(const termux&) = delete;
};

class termux_thread
{
	std::wstring wnd_class_ = L"MnWndClass";

	// std::unordered_map<termux_collection::iterator, std::function<void(std::shared_ptr<termux>>>
	// hotkey_collections_;

public:
	using termux_collection = std::vector<std::shared_ptr<termux>>;

	typedef void (*p_dispatch_func)(termux_thread*, void*);

	struct dispatch_info
	{
		void* data_;
		p_dispatch_func func_;
	};

	termux_thread();
	~termux_thread();
	void run();
	void dispatch(std::function<void()> func) const;
	std::weak_ptr<termux> new_window(int width = 800, int height = 450);
	DWORD ui_thread_id_ = 0;

private:
	HINSTANCE hInstance_;

	// Recycle marks
	Gdiplus::GdiplusStartupInput gdi_input_;
	ULONG_PTR gdi_token_;

	ATOM wnd_class_token_;

	std::latch start_clean_up{1};

	bool ll_keyboard_begin();
	bool ll_keyboard_end();
	HHOOK ll_keyboard_hhook = NULL;

	LRESULT keyboard_ll_proc(int code, WPARAM wParam, LPARAM lParam);
	LRESULT wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Static functions
	static LRESULT CALLBACK KeyboardLLProc(int code, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	std::thread this_thread_;
	termux_collection termux_collection_;
};



#endif

