#include <Common/If/Common.h>
#include <Common/If/Window.h>


Window::Window( ) :
    m_handle(0)
{
	COMMON("(+) Window.");
}



Window::~Window( )
{
//	COMMON("(-) Window.");
}


void
Window::create(Instance instance, WNDPROC msgHndlr, Int menu, 
               Int smallIcon, Int largeIcon, const Char* title)
{
    WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;

    //typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

    wcex.lpfnWndProc	= (WNDPROC)(msgHndlr);
    wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= (HINSTANCE) instance;
	wcex.hIcon			= LoadIcon((HINSTANCE) instance, (LPCTSTR) largeIcon);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)menu;
	wcex.lpszClassName	= title;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)smallIcon);

	RegisterClassEx(&wcex);

    m_currentInstance = instance; // Store instance handle in our member variable

    m_handle = CreateWindow(title, title, WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, m_currentInstance, NULL);

    if (!m_handle)
    {
        COMMON("(!) Window::create : Creating window failed!");
        return;
    }
 
    ShowWindow(m_handle, SW_SHOWNORMAL);
    UpdateWindow(m_handle);

    m_accelerator = LoadAccelerators(instance, (LPTSTR)menu);
}


Boolean 
Window::handleMessages( )
{
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    //PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	if (!TranslateAccelerator(m_handle, m_accelerator, &msg)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message==WM_QUIT)
		return false;
    else
        return true;
}


void
Window::show( )
{
    ShowWindow(m_handle, SW_SHOWNORMAL);
}



void 
Window::hide( )
{
    ShowWindow(m_handle, SW_HIDE);
}



Messagebox::Messagebox(Window::Handle handle, Char* title, Char* message)
{
    COMMON("(+) MessageBox, title = %s, message = %s.", title, message);
    MessageBox(handle, message, title, MB_OK);
}


Messagebox::~Messagebox( )
{
    COMMON("(-) MessageBox.");
}