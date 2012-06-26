#define WIN32_LEAN_AND_MEAN		


////// Defines
#define BITMAP_ID 0x4D42		
#define MAP_X	32				
#define MAP_Z	32				
#define MAP_SCALE	20.0f		
#define PI		3.14159

////// Includes
#include <windows.h>			
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>				
#include <gl/glu.h>				

////// Global Variables
HDC g_HDC;						
bool fullScreen = false;		
bool keyPressed[256];			

float angle = 0.0f;				
float radians = 0.0f;			
float waterHeight = 154.0f;		
bool waterDir = true;			

////// Mouse/Camera Variables
int mouseX, mouseY;						
float cameraX, cameraY, cameraZ;	
float lookX, lookY, lookZ;			

////// Texture Information
BITMAPINFOHEADER	bitmapInfoHeader;	
BITMAPINFOHEADER	landInfo;			
BITMAPINFOHEADER	waterInfo;			

unsigned char*		imageData;		
unsigned char*      landTexture;	
unsigned char*		waterTexture;	
unsigned int		land;			
unsigned int		water;			

////// Terrain Data
float terrain[MAP_X][MAP_Z][3];		


unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;							
	BITMAPFILEHEADER	bitmapFileHeader;		
	unsigned char		*bitmapImage;			
	int					imageIdx = 0;		
	unsigned char		tempRGB;				

	
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL)
		return NULL;

	
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	
	
	if (bitmapFileHeader.bfType != BITMAP_ID)
	{
		fclose(filePtr);
		return NULL;
	}

	
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	
	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

	
	if (!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}


	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);

	
	if (bitmapImage == NULL)
	{
		fclose(filePtr);
		return NULL;
	}

	
	for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx+=3)
	{
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}

	
	fclose(filePtr);
	return bitmapImage;
}


void InitializeTerrain()
{
	
	for (int z = 0; z < MAP_Z; z++)
	{
		for (int x = 0; x < MAP_X; x++)
		{
			terrain[x][z][0] = float(x)*MAP_SCALE;				
			terrain[x][z][1] = (float)imageData[(z*MAP_Z+x)*3];
			terrain[x][z][2] = -float(z)*MAP_SCALE;
		}
	}
}


bool LoadTextures()
{
	
	landTexture = LoadBitmapFile("green.bmp", &landInfo);
	if (!landTexture)
		return false;

	
	waterTexture = LoadBitmapFile("water.bmp", &waterInfo);
	if (!waterTexture)
		return false;

	
	glGenTextures(1, &land);                  
	glBindTexture(GL_TEXTURE_2D, land);       
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, landInfo.biWidth, landInfo.biHeight, GL_RGB, GL_UNSIGNED_BYTE, landTexture);

	
	glGenTextures(1, &water);
	glBindTexture(GL_TEXTURE_2D, water);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, waterInfo.biWidth, waterInfo.biHeight, GL_RGB, GL_UNSIGNED_BYTE, waterTexture);

	return true;
}

void CleanUp()
{
	free(imageData);
	free(landTexture);
	free(waterTexture);
}


void Initialize()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		

	glShadeModel(GL_SMOOTH);					
	glEnable(GL_DEPTH_TEST);					
	glEnable(GL_CULL_FACE);						
	glFrontFace(GL_CCW);						

	glEnable(GL_TEXTURE_2D);					

	imageData = LoadBitmapFile("terrain2.bmp", &bitmapInfoHeader);

	
	InitializeTerrain();
	LoadTextures();
}




// permukaan render disini bro

//============================


//prosedur controlnya disini ya

//=========================================


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX windowClass;		
	HWND	   hwnd;			
	MSG		   msg;				
	bool	   done;			
	DWORD	   dwExStyle;		
	DWORD	   dwStyle;			
	RECT	   windowRect;

	
	int width = 800;
	int height = 600;
	int bits = 32;

	

	windowRect.left=(long)0;						
	windowRect.right=(long)width;					
	windowRect.top=(long)0;							
	windowRect.bottom=(long)height;					

	
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= WndProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);	
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);		
	windowClass.hbrBackground	= NULL;								
	windowClass.lpszMenuName	= NULL;								
	windowClass.lpszClassName	= "MyClass";
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);		

	
	if (!RegisterClassEx(&windowClass))
		return 0;

	if (fullScreen)								
	{
		DEVMODE dmScreenSettings;					
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);	
		dmScreenSettings.dmPelsWidth = width;		
		dmScreenSettings.dmPelsHeight = height;		
		dmScreenSettings.dmBitsPerPel = bits;		
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			
			MessageBox(NULL, "Display mode failed", NULL, MB_OK);
			fullScreen=FALSE;	
		}
	}

	if (fullScreen)								
	{
		dwExStyle=WS_EX_APPWINDOW;				
		dwStyle=WS_POPUP;						
		ShowCursor(FALSE);						
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	
		dwStyle=WS_OVERLAPPEDWINDOW;					
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStymle);		

	
	hwnd = CreateWindowEx(NULL,									
						  "MyClass",							
						  "Texture Example 3: The Heightfield Terrain Example",		
						  dwStyle | WS_CLIPCHILDREN |
						  WS_CLIPSIBLINGS,
						  0, 0,									
						  windowRect.right - windowRect.left,
						  windowRect.bottom - windowRect.top,	
						  NULL,									
						  NULL,									
						  hInstance,							
						  NULL);								

	
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);			
	UpdateWindow(hwnd);					

	done = false;						
	Initialize();						

	
	while (!done)
	{
		PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);

		if (msg.message == WM_QUIT)		
		{
			done = true;				
		}
		else
		{
			if (keyPressed[VK_ESCAPE])
				done = true;
			else
			{
				Render();

				TranslateMessage(&msg);		
				DispatchMessage(&msg);
			}
		}
	}

	CleanUp();

	if (fullScreen)
	{
		ChangeDisplaySettings(NULL,0);		
		ShowCursor(TRUE);					

	return msg.wParam;
}
