// clang-format off

def_struct_primitive	( send_message,			windows,	"sendMessage");
def_struct_primitive	( getChildren_HWND,		windows,	"getChildrenHWND");
def_struct_primitive	( getChild_HWND,		windows,	"getChildHWND");
def_struct_primitive	( getMAX_HWND,			windows,	"getMAXHWND");
def_struct_primitive	( getDesktop_HWND,		windows,	"getDesktopHWND");
def_struct_primitive	( snapshot,				windows,	"snapshot");

def_struct_primitive	( post_message,			windows,	"postMessage");
def_struct_primitive	( process_messages,		windows,	"processPostedMessages");
def_struct_primitive	( peek_Message,			windows,	"peekMessage");
def_struct_primitive	( getData_HWND,			windows,	"getHWNDData");

def_struct_primitive	( getParent_HWND,		windows,	"getParentHWND");

def_struct_primitive	( set_window_pos,		windows,	"setWindowPos");
def_struct_primitive	( get_window_pos,		windows,	"getWindowPos");

def_struct_primitive	( client_to_screen,		windows,	"clientToScreen");
def_struct_primitive	( screen_to_client,		windows,	"screenToClient");
def_struct_primitive	( getCapture,			windows,	"getCapture");
def_struct_primitive	( getFocus,				windows,	"getFocus");
def_struct_primitive	( isWindowEnabled,		windows,	"isWindowEnabled");

def_struct_primitive	( isWindow,				windows,	"isWindow");
def_struct_primitive	( isWindowVisible,		windows,	"isWindowVisible");
def_struct_primitive	( showWindow,			windows,	"showWindow");
def_struct_primitive	( getWindowPlacement,	windows,	"getWindowPlacement");
def_struct_primitive	( setWindowPlacement,	windows,	"setWindowPlacement");
def_struct_primitive	( windows_setFocus,				windows,	"setFocus");
def_struct_primitive	( windows_setForegroundWindow,	windows,	"setForegroundWindow");
def_struct_primitive	( windows_getWindowLong,		windows,	"getWindowLong");
def_struct_primitive	( windows_setWindowLong,		windows,	"setWindowLong");

def_struct_primitive	( windowFromPoint,		windows,	"windowFromPoint");
// clang-format on
