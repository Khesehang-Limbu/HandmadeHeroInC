#include <windows.h>
#include <stdint.h>
#include <xinput.h>

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
	//Pixel is always 32 Bit, and in memory, RGB is stored as Ox xx bb gg rr in little endian systems
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
}; 

struct Win32_Window_Dimension {
	int Width;
	int Height;
};

#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)  
typedef XINPUT_GET_STATE(x_input_get_state);

XINPUT_GET_STATE(XInputGetStateStub){
	return 0;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef XINPUT_SET_STATE(x_input_set_state);

XINPUT_SET_STATE(XInputSetStateStub){
	return 0;
}

global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void){
	HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
	if(XInputLibrary){
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
	}
} 

Win32_Window_Dimension Win32_Get_Window_Dimension(HWND Window){
	Win32_Window_Dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Height = ClientRect.bottom - ClientRect.top;
	Result.Width = ClientRect.right - ClientRect.left;
	
	return Result;
}

global_variable Win32_Offscreen_Buffer GlobalBackBuffer;

internal void RenderWeirdGradient(Win32_Offscreen_Buffer *Buffer, int XOffset, int YOffset){
	uint8 *Row = (uint8 *)Buffer->Memory;

	for (int Y = 0; Y<Buffer->Height; ++Y){
		
		uint32 *Pixel = (uint32 *)Row;
		
		for (int X = 0; X < Buffer->Width; ++X){
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
		Row += Buffer->Pitch;
	}

}

internal void Win32ResizeDipSection(Win32_Offscreen_Buffer *Buffer, int Width, int Height){
	if (Buffer->Memory){
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}
	
	Buffer->Width = Width;
	Buffer->Height = Height;
	int BytesPerPixel;
	BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	
	int BitmapMemorySize = (Buffer->Width * Buffer->Height)*BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * BytesPerPixel;

}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, Win32_Offscreen_Buffer *Buffer){
	StretchDIBits(
		DeviceContext,
		/*
		X, Y, Height,Width,
		X, Y, Height, Width,
		*/
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,	
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

		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_KEYDOWN:
		{
			uint32 VKCode = wParam;
			bool wasDown = ((lParam & (1 << 30)) != 0);
			bool isDown = ((lParam & (1 << 31)) == 0);

			if (wasDown != isDown){
				if (VKCode == 'W'){
				}
				if (VKCode == 'A'){
				}
				if (VKCode == 'S'){
				}
				if (VKCode == 'd'){
				}
				if (VKCode == 'Q'){
				}
				if (VKCode == 'E'){
				}
				if (VKCode == VK_SPACE){
				}
				if (VKCode == VK_ESCAPE){
					OutputDebugStringA("Escape: ");
					if (isDown){
						OutputDebugStringA("isDown");
					}

					if (wasDown){
						OutputDebugStringA("wasDown");
					}	
					OutputDebugStringA("\n");
				}
				if (VKCode == VK_LEFT){
				}
				if (VKCode == VK_RIGHT){
				}
				if (VKCode == VK_UP){
				}
				if (VKCode == VK_DOWN){
				}

			}
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
			Win32_Window_Dimension Dimension = Win32_Get_Window_Dimension(Window);
			Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer);
			EndPaint(Window, &Paint);
		}break;

		default:
		{
			//OutputDebugStringA("From Default\n");
			result = DefWindowProc(Window, Message, wParam, lParam);
		}break;

	}
	return result;
	

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   PSTR lpCmdLine, int nCmdShow)
{

	Win32LoadXInput();
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

				DWORD dwResult;    
				for (DWORD ControllerIndex=0; ControllerIndex< XUSER_MAX_COUNT; ++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					if (XInputGetState_(ControllerIndex, &ControllerState) == ERROR_SUCCESS){
						_XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

						bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;
					}else{
						// Controller is not connected
					}
				}

				Win32_Window_Dimension Dimension = Win32_Get_Window_Dimension(Window);
				RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer);
				ReleaseDC(Window, DeviceContext);
				
				++XOffset;
				YOffset+=2;
			}
		}
	}else{
		//TODO: Logging
	}
	return 0;
}
