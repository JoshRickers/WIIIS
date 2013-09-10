#include <iostream>
#include <string>
#include <vector>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <dirent.h>

using namespace std;
static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void initVideo()
{
	VIDEO_Init();
	WPAD_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	cout << "\x1b[2;0H";
}

int initFat()
{
	if(!fatInitDefault())
	{
		cout << "Unable to initialise FAT, exiting.\n";
		return 1;
	}
	
	DIR *dir;
	
	dir = opendir("/");
	if(!dir)
	{
		cout << "Failed to open dir, exiting.\n";
		return 1;
	}
	closedir(dir);
	return 0;
}

void searchDir(string &rootDir, vector<string> &files)
{
	
	string filePath;
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	
	dir = opendir(rootDir.c_str());
	
	while((entry=readdir(dir)) != 0)
	{
		stat(entry->d_name, &statbuf);
		if(entry->d_name[0] != '.')
		{
			filePath.erase();
			filePath = rootDir + string(entry->d_name) + "/";
			if(S_ISDIR(statbuf.st_mode))
			{
				searchDir(filePath, files);
			}
			if(string(entry->d_name).length() > 3)
			{
				if(string(entry->d_name).substr(string(entry->d_name).length() - 3, 3) ==  "jpg")
				{
					files.push_back(filePath.substr(0, filePath.size() - 1));
				}
			}
			
		}
	}
	closedir(dir);
}

int main(int argc, char **argv) 
{
	vector<string> files;
	initVideo();
	initFat();
	string rootDir = "/";
	
	cout << "Wii Image Search, press A to start or Home to quit.\n";
	while(1)
	{
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);
		if(pressed & WPAD_BUTTON_HOME) 
		{
			return 0;
		}
		if(pressed & WPAD_BUTTON_A)
		{
			searchDir(rootDir, files);
			for(std::vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
			{
				cout << *i << "\n";
			}
		}
		VIDEO_WaitVSync();
	}

	return 0;
}