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

global_variable bool GlobalRunning; //No need to initialize the variable because the compiler itself initializes the default false value


struct Win32_Offscreen_Buffer {
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int BytesPerPixel;
	int Pitch;
}; 

struct Win32_Window_Dimension {
	int Width;
	int Height;
};

Win32_Window_Dimension Win32_Get_Window_Dimension(HWND Window){
	Win32_Window_Dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Height = ClientRect.bottom - ClientRect.top;
	Result.Width = ClientRect.right - ClientRect.left;
	
	return Result;
}

global_variable Win32_Offscreen_Buffer GlobalBackBuffer;

internal void RenderWeirdGradient(Win32_Offscreen_Buffer Buffer, int XOffset, int YOffset){
	uint8 *Row = (uint8 *)Buffer.Memory;

	for (int Y = 0; Y<Buffer.Height; ++Y){
		
		uint32 *Pixel = (uint32 *)Row;
		
		for (int X = 0; X < Buffer.Width; ++X){
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
		Row += Buffer.Pitch;
	}

}

internal void Win32ResizeDipSection(Win32_Offscreen_Buffer *Buffer, int Width, int Height){
	if (Buffer->Memory){
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}
	
	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	
	int BitmapMemorySize = (Buffer->Width * Buffer->Height)*Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * Buffer->BytesPerPixel;

}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, Win32_Offscreen_Buffer Buffer, int X, int Y, int Width, int Height){
	StretchDIBits(
		DeviceContext,
		/*
		X, Y, Height,Width,
		X, Y, Height, Width,
		*/
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,	
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
		}break;
	
		case WM_DESTROY:
		{
			//TODO: Handle This As An Error - Recreate The Window
			GlobalRunning = false;
			OutputDebugStringA("From WM_Destroy\n");
		}break;
	
		case WM_CLOSE:
		{
			//TODO: Handle This With A Message To The User?
			GlobalRunning = false;
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

			Win32_Window_Dimension Dimension = Win32_Get_Window_Dimension(Window);
			Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, X, Y, Width, Height);
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
	WindClass.style = CS_OWNDC|CS_HREDRAW;
	WindClass.lpfnWndProc = MainWindowCallback;
  	WindClass.hInstance = hInstance;
	//WindClass.hIcon;
	Win32ResizeDipSection(&GlobalBackBuffer, 1280, 720);
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
			GlobalRunning = true;
			int XOffset = 0;
			int YOffset = 0;
			while(GlobalRunning){
				MSG Message;
				while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)){
					if (Message.message == WM_QUIT){
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				
				Win32_Window_Dimension Dimension = Win32_Get_Window_Dimension(Window);
				RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);
				
				++XOffset;
			}
		}
	}else{
		//TODO: Logging
	}
	return 0;
}
