#include <windows.h>
#include <stdint.h>

#define internal static //Since static in C means many things based on the usecase, renaming the static to mean something
#define global_variable 
#define local_persist

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
global_variable bool Running; //No need to initialize the variable because the compiler itself initializes the default false value
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset){
	int Width = BitmapWidth;
	int Height = BitmapHeight;

	int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	int Pitch = Width*BytesPerPixel;
	uint8 *Row = (uint8 *)BitmapMemory;

	for (int Y = 0; Y<BitmapHeight; ++Y){
		
		uint32 *Pixel = (uint32 *)Row;
		
		for (int X = 0; X < BitmapWidth; ++X){
			uint8 Blue = (X + XOffset);
			uint8 Green = (Y + YOffset);
			*Pixel++ = ((Green << 8) | Blue);
			/*
			*Pixel = (uint8)(X + XOffset);
			++Pixel;
			
			*Pixel = (uint8)(Y + YOffset);
			++Pixel;

			*Pixel = 0;
			++Pixel;

			*Pixel = 0;
			++Pixel;
			*/
		}
		Row += Pitch;
	}

}

internal void Win32ResizeDipSection(int Width, int Height){
	if (BitmapMemory){
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}
	
	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	RenderWeirdGradient(128, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height){
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;

	StretchDIBits(
		DeviceContext,
		/*
		X, Y, Height,Width,
		X, Y, Height, Width,
		*/
		0, 0, BitmapWidth, BitmapHeight,
		0, 0, WindowWidth, WindowHeight,
		BitmapMemory,
		&BitmapInfo,	
		DIB_RGB_COLORS,
		SRCCOPY);
}

LRESULT MainWindowCallback(
  HWND Window,
  UINT Message,
  WPARAM wParam,
  LPARAM lParam 
){
	LRESULT result = 0;
	switch(Message){
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Height = ClientRect.bottom - ClientRect.top;
			int Width = ClientRect.right - ClientRect.left;
			Win32ResizeDipSection(Width, Height);
		}break;
	
		case WM_DESTROY:
		{
			//TODO: Handle This As An Error - Recreate The Window
			Running = false;
			OutputDebugStringA("From WM_Destroy\n");
		}break;
	
		case WM_CLOSE:
		{
			//TODO: Handle This With A Message To The User?
			Running = false;
			OutputDebugStringA("From WM_Close\n");
		}break;
			
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("From WM_Activateapp\n");
		}break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;

			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			
			Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
			EndPaint(Window, &Paint);
		}break;

		default:
		{
			OutputDebugStringA("From Default\n");
			result = DefWindowProc(Window, Message, wParam, lParam);
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

	//MessageBox(0, "This is Handmade Hero", "HandMade Hero", MB_OK|MB_ICONINFORMATION);
	if(RegisterClass(&WindClass)){
		HWND Window = CreateWindowExA(
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
		if (Window){
			Running = true;
			int XOffset = 0;
			int YOffset = 0;
			while(Running){
				MSG Message;
				while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)){
					if (Message.message == WM_QUIT){
						Running = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				
				RenderWeirdGradient(XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				int WindowWidth = ClientRect.right - ClientRect.left;
				Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
				ReleaseDC(Window, DeviceContext);
				
				++XOffset;
			}
		}
	}else{
		//TODO: Logging
	}
	return 0;
}
