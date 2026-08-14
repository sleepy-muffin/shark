#ifdef _WIN32

#include <windows.h>
#include <objidl.h>
#include <propidl.h>
#include <gdiplus.h>
#include <windowsx.h>

#include <algorithm>
#include <memory>

using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

#include "shark/termux.hpp"

// Win32 Native Window Creator
// It will create a UI thread, and this UI thread will handle all other termux window behaviors.
// For simplicity.

// Only create the termux object with std::shared_ptr

// Unique object class

#define WM_DISPATCH (WM_USER + 1)

thread_local termux_thread* local_termux_thread_ = NULL;

termux::termux(
	termux_thread* termux_thread, HWND hwnd) : termux_thread_(termux_thread), hwnd_(hwnd)
{}

struct termux::control
{
	enum class kind { text_block, button, text_edit };

	int id;
	kind type;
	RECT bounds;
	std::wstring text;
	std::wstring placeholder;
	button_callback on_click;
	bool pressed = false;
	bool editable = true;
};

termux::~termux() = default;

termux::control* termux::find_control(int id)
{
	for (const auto& control : controls_) {
		if (control->id == id)
			return control.get();
	}
	return nullptr;
}

int termux::hit_test(POINT point) const
{
	for (auto it = controls_.rbegin(); it != controls_.rend(); ++it) {
		const RECT& bounds = (*it)->bounds;
		if (point.x >= bounds.left && point.x < bounds.right && point.y >= bounds.top &&
			point.y < bounds.bottom)
			return (*it)->id;
	}
	return 0;
}

void termux::redraw()
{
	InvalidateRect(hwnd_, NULL, FALSE);
}

int termux::add_text_block(RECT bounds, std::wstring text)
{
	controls_.push_back(std::make_unique<control>(
		control{next_control_id_++, control::kind::text_block, bounds, std::move(text)}));
	redraw();
	return controls_.back()->id;
}

int termux::add_button(RECT bounds, std::wstring text, button_callback on_click)
{
	controls_.push_back(std::make_unique<control>(control{next_control_id_++, control::kind::button,
		bounds, std::move(text), L"", std::move(on_click)}));
	redraw();
	return controls_.back()->id;
}

int termux::add_text_edit(
	RECT bounds, std::wstring text, std::wstring placeholder, bool editable)
{
	controls_.push_back(std::make_unique<control>(control{next_control_id_++, control::kind::text_edit,
		bounds, std::move(text), std::move(placeholder)}));
	controls_.back()->editable = editable;
	redraw();
	return controls_.back()->id;
}

std::wstring termux::text_edit_value(int control_id) const
{
	for (const auto& control : controls_) {
		if (control->id == control_id && control->type == control::kind::text_edit)
			return control->text;
	}
	return L"";
}

void termux::set_text_edit_value(int control_id, std::wstring text)
{
	if (GetCurrentThreadId() != termux_thread_->ui_thread_id_) {
		termux_thread_->dispatch([this, control_id, text = std::move(text)]() mutable {
			set_text_edit_value(control_id, std::move(text));
		});
		return;
	}

	if (control* edit = find_control(control_id);
		edit && edit->type == control::kind::text_edit) {
		edit->text = std::move(text);
		redraw();
	}
}

void termux::paint(HDC hdc)
{
	Graphics graphics(hdc);
	graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	SolidBrush background(Color(255, 250, 250, 250));
	graphics.FillRectangle(&background, 0, 0, 10000, 10000);

	Font ui_font(L"Segoe UI", 15.0f, FontStyleRegular, UnitPixel);
	Font fallback_font(FontFamily::GenericSansSerif(), 15.0f, FontStyleRegular, UnitPixel);
	const Font* body = ui_font.GetLastStatus() == Ok ? &ui_font : &fallback_font;
	StringFormat format;
	format.SetLineAlignment(StringAlignmentCenter);
	format.SetTrimming(StringTrimmingEllipsisCharacter);

	for (const auto& item : controls_) {
		const control& c = *item;
		Rect rect(c.bounds.left, c.bounds.top, c.bounds.right - c.bounds.left,
			c.bounds.bottom - c.bounds.top);
		RectF text_bounds(static_cast<REAL>(rect.X), static_cast<REAL>(rect.Y),
			static_cast<REAL>(rect.Width), static_cast<REAL>(rect.Height));
		graphics.SetClip(rect);
		const bool hovered = c.id == hovered_control_id_;
		const bool focused = c.id == focused_control_id_;

		if (c.type == control::kind::text_block) {
			SolidBrush text(Color(255, 35, 35, 35));
			format.SetAlignment(StringAlignmentNear);
			graphics.DrawString(c.text.c_str(), -1, body, text_bounds, &format, &text);
			graphics.ResetClip();
			continue;
		}

		if (c.type == control::kind::button) {
			const Color fill = c.pressed ? Color(255, 0, 91, 176) :
				hovered ? Color(255, 0, 120, 215) : Color(255, 0, 104, 190);
			SolidBrush button_fill(fill), button_text(Color::White);
			graphics.FillRectangle(&button_fill, rect);
			format.SetAlignment(StringAlignmentCenter);
			graphics.DrawString(c.text.c_str(), -1, body, text_bounds, &format, &button_text);
			graphics.ResetClip();
			continue;
		}

		SolidBrush edit_fill(Color::White);
		Pen border(focused ? Color(255, 0, 120, 215) : Color(255, 190, 190, 190),
			focused ? 2.0f : 1.0f);
		graphics.FillRectangle(&edit_fill, rect);
		graphics.DrawRectangle(&border, rect);
		RectF text_rect(static_cast<REAL>(rect.X + 10), static_cast<REAL>(rect.Y),
			static_cast<REAL>(std::max(0, rect.Width - 20)), static_cast<REAL>(rect.Height));
		const std::wstring& visible = c.text.empty() ? c.placeholder : c.text;
		SolidBrush text(c.text.empty() ? Color(255, 130, 130, 130) : Color(255, 35, 35, 35));
		format.SetAlignment(StringAlignmentNear);
		format.SetLineAlignment(c.editable ? StringAlignmentCenter : StringAlignmentNear);
		graphics.DrawString(visible.c_str(), -1, body, text_rect, &format, &text);
		format.SetLineAlignment(StringAlignmentCenter);

		if (focused && c.editable) {
			RectF caret_area;
			graphics.MeasureString(c.text.c_str(), -1, body, text_rect, &format, &caret_area);
			const REAL caret_x = c.text.empty() ? text_rect.X : caret_area.X + caret_area.Width + 1.0f;
			Pen caret(Color(255, 35, 35, 35), 1.0f);
			graphics.DrawLine(&caret, caret_x, text_rect.Y + 8.0f, caret_x,
				text_rect.Y + text_rect.Height - 8.0f);
		}
		graphics.ResetClip();
	}
}

bool termux::mouse_move(POINT point)
{
	const int hit = hit_test(point);
	if (hovered_control_id_ == hit)
		return false;
	hovered_control_id_ = hit;
	redraw();
	return true;
}

bool termux::mouse_down(POINT point)
{
	const int hit = hit_test(point);
	focused_control_id_ = 0;
	if (control* c = find_control(hit)) {
		if (c->type == control::kind::button)
			c->pressed = true;
		if (c->type == control::kind::text_edit && c->editable)
			focused_control_id_ = c->id;
	}
	SetForegroundWindow(hwnd_);
	SetFocus(hwnd_);
	redraw();
	return hit != 0;
}

bool termux::mouse_up(POINT point)
{
	for (const auto& item : controls_) {
		control& c = *item;
		if (c.type == control::kind::button && c.pressed) {
			c.pressed = false;
			if (c.id == hit_test(point) && c.on_click)
				c.on_click();
			redraw();
			return true;
		}
	}
	return false;
}

bool termux::char_input(wchar_t character)
{
	control* c = find_control(focused_control_id_);
	if (!c || c->type != control::kind::text_edit || !c->editable || character < L' ')
		return false;
	c->text += character;
	redraw();
	return true;
}

bool termux::key_down(WPARAM key)
{
	control* c = find_control(focused_control_id_);
	if (!c || c->type != control::kind::text_edit || !c->editable || key != VK_BACK)
		return false;
	if (!c->text.empty())
		c->text.pop_back();
	redraw();
	return true;
}

termux_thread::termux_thread() : this_thread_(&termux_thread::run, this)
{
	this_thread_.detach();
	start_clean_up.wait();
}

termux_thread::~termux_thread()
{
	GdiplusShutdown(gdi_token_);
	ll_keyboard_end();
}

void termux_thread::dispatch(std::function<void()> func) const
{
	struct dispatch_data
	{
		std::function<void()> func_;
	};

	p_dispatch_func callback = [](termux_thread*, void* data) -> void
	{
		std::unique_ptr<dispatch_data> task(reinterpret_cast<dispatch_data*>(data));
		task->func_();
	};

	dispatch_info* info = new dispatch_info(new dispatch_data{std::move(func)}, callback);
	if (!PostThreadMessage(ui_thread_id_, WM_DISPATCH, WPARAM(0), (LPARAM)info)) {
		delete reinterpret_cast<dispatch_data*>(info->data_);
		delete info;
	}
}

void termux::key_bind_hotkey(
	UINT fsModifiers, UINT vk, std::function<void(std::weak_ptr<termux>)>)
{}

void termux_thread::run()
{
	local_termux_thread_ = this;
	ui_thread_id_ = GetCurrentThreadId();
	hInstance_ = GetModuleHandle(NULL);

	// Pre Initialize
	GdiplusStartup(&gdi_token_, &gdi_input_, NULL);
	ll_keyboard_begin();

	WNDCLASS wndClass;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance_;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = wnd_class_.c_str();

	wnd_class_token_ = RegisterClass(&wndClass);

	MSG msg;

	// shark::log::debug("local_termux_thread_ {}", (unsigned long long)local_termux_thread_);
	// shark::log::debug("this_thread_ {}", GetThreadIdFromNativeHandle(this_thread_));
	start_clean_up.count_down();
	shark::log::debug("Looping main thread id {} : {}", ui_thread_id_, wnd_class_token_);

	while (true) {
		GetMessage(&msg, NULL, 0, 0);
		shark::log::debug("GetMessage caught: {} ", msg.message);
		switch (msg.message) {
		case WM_DISPATCH: {
			dispatch_info* info = (dispatch_info*)msg.lParam;
			info->func_(this, info->data_);
			delete info;
			continue;
		}
		default:
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	shark::log::debug("termux_thread::run() quiting");
}


std::weak_ptr<termux> termux_thread::new_window(int width, int height)
{
	struct dispatch_data
	{
		HWND hwnd_ = NULL;
		std::latch the_latch_{1};
		int width_;
		int height_;
	} _dispatch_data;

	_dispatch_data.width_ = width;
	_dispatch_data.height_ = height;

	p_dispatch_func callback = [](termux_thread* parent, void* data) -> void
	{
		printf("termux_thread::new_window p_dispatch_func  \n");

		dispatch_data* _data = reinterpret_cast<dispatch_data*>(data);

		_data->hwnd_ = CreateWindowEx(
			WS_EX_TOPMOST | WS_EX_PALETTEWINDOW, parent->wnd_class_.c_str(),
									  TEXT("Getting Started"), WS_OVERLAPPEDWINDOW,
									  CW_USEDEFAULT, CW_USEDEFAULT, _data->width_, _data->height_,
								NULL, NULL, parent->hInstance_, NULL);

		ShowWindow(_data->hwnd_, SW_SHOW);
		UpdateWindow(_data->hwnd_);

		_data->the_latch_.count_down();
	};

	dispatch_info* info = new dispatch_info(&_dispatch_data, callback);

	BOOL bpost = PostThreadMessage(ui_thread_id_, WM_DISPATCH, WPARAM(0), (LPARAM)info);
	shark::log::debug("termux_thread::new_widow wait {}", ui_thread_id_);
	_dispatch_data.the_latch_.wait();
	shark::log::debug("termux_thread::new_widow finished{}", ui_thread_id_);

	if (bpost) {
		std::shared_ptr<termux> ptr = std::make_shared<termux>(this, _dispatch_data.hwnd_);
		termux_collection_.emplace_back(std::move(ptr));
		return termux_collection_.at(termux_collection_.size() - 1ULL);
	}
	else {
		return {};
	}
}
/*
void termux_thread::assign(std::shared_ptr<termux> _termux) {
	// DWORD thread_id = GetThreadIdFromNativeHandle(this->this_thread_);
	// shark::log::debug("PostThreadMessage {}", bpost);
	BOOL bpost1 = PostThreadMessage(ui_thread_id_, WM_CLOSE, WPARAM(0), LPARAM(0));
	// shark::log::debug("PostThreadMessage {}", bpost1);
}
*/

LRESULT termux_thread::wnd_proc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// shark::log::debug("proc");

	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		for (const auto& window : termux_collection_) {
			if (window->hwnd_ == hWnd) {
				window->paint(hdc);
				break;
			}
		}
		EndPaint(hWnd, &ps);
		return 0;
	case WM_MOUSEMOVE:
		for (const auto& window : termux_collection_) {
			if (window->hwnd_ == hWnd) {
				window->mouse_move({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
				return 0;
			}
		}
		return 0;
	case WM_LBUTTONDOWN:
		for (const auto& window : termux_collection_) {
			if (window->hwnd_ == hWnd) {
				window->mouse_down({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
				return 0;
			}
		}
		return 0;
	case WM_LBUTTONUP:
		for (const auto& window : termux_collection_) {
			if (window->hwnd_ == hWnd) {
				window->mouse_up({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
				return 0;
			}
		}
		return 0;
	case WM_CHAR:
		for (const auto& window : termux_collection_) {
			if (window->hwnd_ == hWnd && window->char_input(static_cast<wchar_t>(wParam)))
				return 0;
		}
		break;
	case WM_KEYDOWN:
		for (const auto& window : termux_collection_) {
			if (window->hwnd_ == hWnd && window->key_down(wParam))
				return 0;
		}
		break;
	case WM_HOTKEY:

		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

VOID OnPaint(
	HDC hdc)
{
	Graphics graphics(hdc);
	Pen pen(Color(255, 0, 0, 255));
	graphics.DrawLine(&pen, 0, 0, 200, 100);

	SolidBrush brush(Color(255, 0, 0, 255));
	FontFamily fontFamily(L"Times New Roman");
	Font font(&fontFamily, 24, FontStyleRegular, UnitPixel);
	PointF pointF(10.0f, 20.0f);

	graphics.DrawString(L"Hello World!", -1, &font, pointF, &brush);
}

LRESULT CALLBACK termux_thread::keyboard_ll_proc(
	int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0 || code == HC_NOREMOVE) {
		return CallNextHookEx(ll_keyboard_hhook, code, wParam, lParam);
	}
	if (lParam & 0x40000000) {
		// Don't process for release
		return CallNextHookEx(ll_keyboard_hhook, code, wParam, lParam);
	}
	// wchar_t szKeyName[200];
	// GetKeyNameText(lParam, (LPWSTR)szKeyName, 100);
	KBDLLHOOKSTRUCT* key_info = (KBDLLHOOKSTRUCT*)lParam;
	shark::log::debug("Key pressed: {}", key_info->vkCode);

	return CallNextHookEx(ll_keyboard_hhook, code, wParam, lParam);
}

bool termux_thread::ll_keyboard_begin()
{
	ll_keyboard_hhook = SetWindowsHookEx(WH_KEYBOARD_LL, termux_thread::KeyboardLLProc, NULL, 0);
	if (ll_keyboard_hhook == NULL) {
		MessageBox(NULL, L"SetWindowsHookEx Failed", L"Error", MB_OK);
		return true;
	}
	return false;
}

bool termux_thread::ll_keyboard_end()
{
	return UnhookWindowsHookEx(ll_keyboard_hhook);
}

#pragma region STATIC FUNCTIONS
LRESULT CALLBACK termux_thread::WndProc(
	HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return local_termux_thread_->wnd_proc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK termux_thread::KeyboardLLProc(
	int code, WPARAM wParam, LPARAM lParam)
{
	return local_termux_thread_->keyboard_ll_proc(code, wParam, lParam);
}
#pragma endregion


#endif

