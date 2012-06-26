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





void Render()
{
	radians =  float(PI*(angle-90.0f)/180.0f);

	
	cameraX = lookX + sin(radians)*mouseY;	
	cameraZ = lookZ + cos(radians)*mouseY; 
	cameraY = lookY + mouseY / 2.0f;

	
	lookX = (MAP_X*MAP_SCALE)/2.0f;
	lookY = 150.0f;
	lookZ = -(MAP_Z*MAP_SCALE)/2.0f;

	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glLoadIdentity();

	
	gluLookAt(cameraX, cameraY, cameraZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);

	
	glBindTexture(GL_TEXTURE_2D, land);

	
	for (int z = 0; z < MAP_Z-1; z++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		
		for (int x = 0; x < MAP_X-1; x++)
		{
			
		
			
			glColor3f(terrain[x][z][1]/255.0f, terrain[x][z][1]/255.0f, terrain[x][z][1]/255.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(terrain[x][z][0], terrain[x][z][1], terrain[x][z][2]);

			
			glTexCoord2f(1.0f, 0.0f);
			glColor3f(terrain[x+1][z][1]/255.0f, terrain[x+1][z][1]/255.0f, terrain[x+1][z][1]/255.0f);
			glVertex3f(terrain[x+1][z][0], terrain[x+1][z][1], terrain[x+1][z][2]);

			
			glTexCoord2f(0.0f, 1.0f);
			glColor3f(terrain[x][z+1][1]/255.0f, terrain[x][z+1][1]/255.0f, terrain[x][z+1][1]/255.0f);
			glVertex3f(terrain[x][z+1][0], terrain[x][z+1][1], terrain[x][z+1][2]);

			
			glColor3f(terrain[x+1][z+1][1]/255.0f, terrain[x+1][z+1][1]/255.0f, terrain[x+1][z+1][1]/255.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(terrain[x+1][z+1][0], terrain[x+1][z+1][1], terrain[x+1][z+1][2]);
		}
		glEnd();
	}
	
	glEnable(GL_BLEND);

	
	glDepthMask(GL_FALSE);

	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glColor4f(0.5f, 0.5f, 1.0f, 0.7f);			
	glBindTexture(GL_TEXTURE_2D, water);		
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);				
		glVertex3f(terrain[0][0][0], waterHeight, terrain[0][0][2]);

		glTexCoord2f(10.0f, 0.0f);			
		glVertex3f(terrain[MAP_X-1][0][0], waterHeight, terrain[MAP_X-1][0][2]);

		glTexCoord2f(10.0f, 10.0f);				
		glVertex3f(terrain[MAP_X-1][MAP_Z-1][0], waterHeight, terrain[MAP_X-1][MAP_Z-1][2]);

		glTexCoord2f(0.0f, 10.0f);				
		glVertex3f(terrain[0][MAP_Z-1][0],waterHeight, terrain[0][MAP_Z-1][2]);
	glEnd();

	
	glDepthMask(GL_TRUE);

	
	glDisable(GL_BLEND);

	
	if (waterHeight > 155.0f)
		waterDir = false;
	else if (waterHeight < 154.0f)
		waterDir = true;

	if (waterDir)
		waterHeight += 0.01f;
	else
		waterHeight -= 0.01f;

	glFlush();
	SwapBuffers(g_HDC);			
}


void SetupPixelFormat(HDC hDC)
{
	int nPixelFormat;					

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	
		1,								
		PFD_DRAW_TO_WINDOW |			
		PFD_SUPPORT_OPENGL |			
		PFD_DOUBLEBUFFER,				
		PFD_TYPE_RGBA,					
		32,								
		0, 0, 0, 0, 0, 0,				
		0,								
		0,								
		0,								
		0, 0, 0, 0,						
		16,								
		0,								
		0,								
		PFD_MAIN_PLANE,					
		0,								
		0, 0, 0 };						

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);	

	SetPixelFormat(hDC, nPixelFormat, &pfd);		
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hRC;					
	static HDC hDC;						
	int width, height;					
	int oldMouseX, oldMouseY;

	switch(message)
	{
		case WM_CREATE:					

			hDC = GetDC(hwnd);			
			g_HDC = hDC;
			SetupPixelFormat(hDC);		

			
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			return 0;
			break;

		case WM_CLOSE:					

			
			wglMakeCurrent(hDC, NULL);
			wglDeleteContext(hRC);

			
			PostQuitMessage(0);

			return 0;
			break;

		case WM_SIZE:
			height = HIWORD(lParam);		
			width = LOWORD(lParam);

			if (height==0)					
			{
				height=1;					
			}

			glViewport(0, 0, width, height);	
			glMatrixMode(GL_PROJECTION);		
			glLoadIdentity();					

			
			gluPerspective(54.0f,(GLfloat)width/(GLfloat)height,1.0f,1000.0f);

			glMatrixMode(GL_MODELVIEW);			
			glLoadIdentity();					

			return 0;
			break;

		case WM_KEYDOWN:					
			keyPressed[wParam] = true;
			return 0;
			break;

		case WM_KEYUP:
			keyPressed[wParam] = false;
			return 0;
			break;

		case WM_MOUSEMOVE:
			
			oldMouseX = mouseX;
			oldMouseY = mouseY;

			
			mouseX = LOWORD(lParam);
			mouseY = HIWORD(lParam);

			
			if (mouseY < 200)
				mouseY = 200;
			if (mouseY > 450)
				mouseY = 450;

			if ((mouseX - oldMouseX) > 0)		
				angle += 3.0f;
			else if ((mouseX - oldMouseX) < 0)	
				angle -= 3.0f;

			return 0;
			break;
		default:
			break;
	}

	return (DefWindowProc(hwnd, message, wParam, lParam));
}


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
