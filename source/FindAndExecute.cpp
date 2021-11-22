/***************************************************************************
 * 
 * FIND AND EXECUTE - a small tool to run a command on many files in dos
 * 
 * $$
 *
 * Copyright (C) 2007 Peter D'Hoye
 *
 * All files in this archive are subject to the GNU General Public License.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

// FindAndExecute.cpp : the sole application source file.

#include "stdafx.h"

char szSPEC[256];
char szCMD[256];
char ExeString[MAX_PATH];


void splitfile(char* orgname1, char* orgname2, char* name1, char* name2, char* ext)
{
	char tstr[MAX_PATH];

	// long filename in orgname1
	lstrcpy(tstr,orgname1);
	int len = lstrlen(tstr);
	char* p = tstr+len-1; // on last char
	while((p>tstr) && (*p != '.')) p--;
	if(*p == '.')
	{
	    *p++ = 0;
	    lstrcpy(name1,tstr);
	    lstrcpy(ext,p);
	}
	else
	{
		// no extension ???
		lstrcpy(name1,orgname1);
		lstrcpy(ext,"");
	}

	// short filename in orgname2 - skip extension, should be the same!
	lstrcpy(tstr,orgname2);
	len = lstrlen(tstr);
	p = tstr+len-1; // on last char
	while((p>tstr) && (*p != '.')) p--;
	if(*p == '.')
	{
		*p++ = 0;
		lstrcpy(name2,tstr);
//		lstrcpy(ext,p);
	}
	else
	{
		// no extension ???
		lstrcpy(name2,orgname2);
//		lstrcpy(ext,"");
	}

}


int execute(char* formatter, char* path, char* path2, char* filename1, char* filename2, char* ext)
{
	// convert formatter string into fully qualified command
	memset(ExeString,0,MAX_PATH);
	char* p;
	
	while(*formatter)
	{
		if(*formatter == '#')
		{
			formatter++;
			switch(*formatter)
			{
				case 0:
					printf("\n\rERROR: invalid syntax in command to execute: incomplete variable");
					return -1;
				case 'F':
					// full path + filename
					lstrcat(ExeString,path);
					lstrcat(ExeString,"\\");
					lstrcat(ExeString,filename1);
					lstrcat(ExeString,".");
					lstrcat(ExeString,ext);
					break;
				case 'D':
					// full path + 8.3 filename
					lstrcat(ExeString,path2);
					lstrcat(ExeString,"\\");
					lstrcat(ExeString,filename2);
					lstrcat(ExeString,".");
					lstrcat(ExeString,ext);
					break;
				case 'p':
					// path
					lstrcat(ExeString,path);
					break;
				case 'f':
					// filename
					lstrcat(ExeString,filename1);
					lstrcat(ExeString,".");
					lstrcat(ExeString,ext);
					break;
				case 'd':
					// 8.3 filename
					lstrcat(ExeString,filename2);
					lstrcat(ExeString,".");
					lstrcat(ExeString,ext);
					break;
				case 'n':
					// filename without extension
					lstrcat(ExeString,filename1);
					break;
				case 'e':
					// extension
					lstrcat(ExeString,ext);
					break;
				case 'q':
                    //quote
					lstrcat(ExeString,"\"");
                    break;
				default:
					printf("\n\rERROR: invalid syntax in command to execute: unknown variable #%c",*formatter);
					return -1;
			}
			formatter++;
		}
		else
		{
			p = ExeString + lstrlen(ExeString);
			while((*formatter) && (*formatter != '#')) *p++ = *formatter++;
		}
	}

	// command is ready, lets execute....
	//printf("\n\rDEBUG ExeString=%s",ExeString);
	system(ExeString);

	return 0;
}


int fae(char* BaseDir, char* BaseDir2)
{
	// recursive scan through folders looking for files....

	HANDLE hSearch;
	WIN32_FIND_DATA FileData;
	char name1[MAX_PATH];
	char name2[MAX_PATH];
	char ext[MAX_PATH];
	char subpath[MAX_PATH];
	char subpath2[MAX_PATH];
	char searchpath[MAX_PATH];

//	printf("\r\nBaseDir  = %s",BaseDir);
//	printf("\r\nBaseDir2 = %s",BaseDir2);
//	printf("\r\n");

	// 1. search current folder for matching files
	lstrcpy(searchpath,BaseDir);
	if(searchpath[lstrlen(searchpath)-1] != '\\') lstrcat(searchpath,"\\");
	lstrcat(searchpath,szSPEC);

	hSearch = FindFirstFile(searchpath, &FileData); 
	while(hSearch != INVALID_HANDLE_VALUE)
	{
		// found some files..... process them
		if(FileData.cFileName[0] != '.')
		{
			// don't process current and previous directory markers (. and ..)
			// note this will also skip directories starting with a . which is not common on windows platforms
			if(lstrlen(FileData.cAlternateFileName))
			{
				splitfile(FileData.cFileName,FileData.cAlternateFileName,name1,name2,ext);
			}
			else
			{
				splitfile(FileData.cFileName,FileData.cFileName,name1,name2,ext);
			}
			execute(szCMD,BaseDir,BaseDir2,name1,name2,ext);
		}
		if(!FindNextFile(hSearch, &FileData)) break;
	}
	FindClose(hSearch);

	// 2. search subdirectories as well -> other filespec
	lstrcpy(searchpath,BaseDir);
	if(searchpath[lstrlen(searchpath)-1] != '\\') lstrcat(searchpath,"\\");
	lstrcat(searchpath,"*.*");

	hSearch = FindFirstFile(searchpath, &FileData); 
	while(hSearch != INVALID_HANDLE_VALUE)
	{
		// found some files..... process them
		if(FileData.cFileName[0] != '.')
		{
			// don't process current and previous directory markers (. and ..)
			// note this will also skip directories starting with a . which is not common on windows platforms
			if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// longfilename path construction
				lstrcpy(subpath,BaseDir);
				if(subpath[lstrlen(subpath)-1] != '\\') lstrcat(subpath,"\\");
				lstrcat(subpath,FileData.cFileName);
				// 8.3 filename path construction
				lstrcpy(subpath2,BaseDir2);
				if(subpath2[lstrlen(subpath2)-1] != '\\') lstrcat(subpath2,"\\");
				if(lstrlen(FileData.cAlternateFileName))
				{
					lstrcat(subpath2,FileData.cAlternateFileName);
				}
				else
				{
					lstrcat(subpath2,FileData.cFileName);
				}
//				printf("\r\n#### going one level down to: %s",subpath);
//				printf("\r\nsubpath  = %s",subpath);
//				printf("\r\nsubpath2 = %s",subpath2);
//				printf("\r\n");
				fae(subpath,subpath2);
			}
		}
		if(!FindNextFile(hSearch, &FileData)) break;
	}
	FindClose(hSearch);

	return 0;
}


int main(int argc, char* argv[])
{
	char szCWD1[MAX_PATH];
	char szCWD2[MAX_PATH];

	if(argc<3)
	{
		printf("\n\r");
		printf("\n\rFindAndExecute 0.1.1 (c)2007 Peter D'Hoye");
		printf("\n\r=========================================");
		printf("\n\rUsage: FAE <filespec> <command to execute>");
		printf("\n\r  filespec: pattern to match the filename");
		printf("\n\r  command: action to perform between quotes.");
		printf("\n\r           within the command string, use:");
		printf("\n\r           #F = complete path + filename");
		printf("\n\r           #D = complete path + 8.3 filename");
		printf("\n\r           #p = complete path without trailing backslash");
		printf("\n\r           #f = complete filename");
		printf("\n\r           #d = complete 8.3 filename");
		printf("\n\r           #n = filename without extension");
		printf("\n\r           #e = extension");
		printf("\n\r           #q = quotes");
		printf("\n\rThis software is GPL - visit http://sourceforge.net/projects/fae/");
        printf("\n\r\n\rExample commandline: (renames all BMP files to *_old.bmp)");
        printf("\n\rFAE *.bmp \"move #q#F#q #q#p\#f_old.bmp#q\"");
		return -1;
	}

	GetCurrentDirectory(255,szCWD2);
	GetLongPathName(szCWD2,szCWD1,MAX_PATH-1);
	GetShortPathName(szCWD1,szCWD2,MAX_PATH-1);
	lstrcpy(szSPEC,argv[1]);
	lstrcpy(szCMD,argv[2]);
	return fae(szCWD1,szCWD2);
}
