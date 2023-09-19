#include <windows.h>

LRESULT MainWindowCallback(
  HWND window,
  UINT message,
  WPARAM wParam,
  LPARAM lParam 
){
	LRESULT result = 0;
	switch(message){
		case WM_SIZE:
		{
			OutputDebugStringA("From WM_SIZE\n");
		}break;
	
		case WM_DESTROY:
		{
			OutputDebugStringA("From WM_Destroy\n");
		}break;
	
		case WM_CLOSE:
		{
			OutputDebugStringA("From WM_Close\n");
		}break;
			
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("From WM_Activateapp\n");
		}break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			static DWORD Operation = WHITENESS;
			PatBlt(DeviceContext, X, Y, Width, Height, Operation);
			//Don't do this
			if (Operation == WHITENESS){
				Operation = BLACKNESS;
			}else{
				Operation = WHITENESS;
			}
			EndPaint(window, &Paint);
		}break;

		default:
		{
			OutputDebugStringA("From Default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		}break;

	}
	return result;
	

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   PSTR lpCmdLine, int nCmdShow)
{
	tagWNDCLASSA WindClass = {};
	WindClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindClass.lpfnWndProc = MainWindowCallback;
  	WindClass.hInstance = hInstance;
	//WindClass.hIcon;
  	WindClass.lpszClassName = "HandmadeHeroWindowClass";

	MessageBox(0, "This is Handmade Hero", "HandMade Hero", MB_OK|MB_ICONINFORMATION);
	if(RegisterClass(&WindClass)){
		HWND WindowHandle = CreateWindowExA(
			WS_EX_OVERLAPPEDWINDOW,
			WindClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0);
		if (WindowHandle){
			MSG Message;
			for (;;){
				BOOL MessageResult = GetMessage(&Message, 0, 0, 0); 
				if(MessageResult > 0){
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}else{
					break;
				}
			}
		}
	}else{
		//TODO: Logging
	}
	return 0;
}
