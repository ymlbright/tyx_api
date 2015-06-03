#include "Logger.h"
#include <afx.h>
#include <windows.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>

Logger::Logger(const char *_szPath){
    _szFileName = NULL;
    _hFile = INVALID_HANDLE_VALUE;
    ::InitializeCriticalSection(&_csLock);
    SetPath(_szPath);
}

Logger::~Logger(){
    Close();
    if( _szFileName )
        delete[] _szFileName;
    if( _szPath )
        delete[] _szPath;
	::DeleteCriticalSection(&_csLock);
}

void Logger::Log(LPCVOID lpBuffer, DWORD dwLength){
    assert(lpBuffer);
    char newName[10];
    char temp[MAX_PATH];
    __try{
        Lock();
        time_t now = time(NULL);
        strftime(newName, 10, "%Y%m%d", localtime(&now));
        if( strcmp(_szLastDate, newName) != 0 ){
            strcat(strcpy(temp, _szPath), "//");
            strcat(strcat(temp, newName), ".log");
            strcpy(_szLastDate, newName);
            SetFileName(temp);
        }
        if( !OpenFile() ){
            Unlock();
            return;
        }
        WriteLog(lpBuffer, dwLength);
    } __finally {
        Unlock();
    } 
}

void Logger::Log(const char *szText){
    Log(szText, strlen(szText));
}

const char * Logger::GetFileName(){
    return _szFileName;
}

const char * Logger::GetPath(){
    return _szPath;
}

bool Logger::OpenFile(){
    if( IsOpen() )
        return true;
    if( !_szFileName )
        return false;
    _hFile =  CreateFile(
        _szFileName, 
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
    if( !IsOpen() && GetLastError() == 2 ) //open failed, and file doesn't exist
        _hFile =  CreateFile(
            _szFileName, 
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL ); 
    if( IsOpen() )
        SetFilePointer(_hFile, 0, NULL, FILE_END);
    return IsOpen();
}

inline DWORD Logger::Write(LPCVOID lpBuffer, DWORD dwLength){
    DWORD dwWriteLength = 0;
    if( IsOpen() )
        WriteFile(_hFile, lpBuffer, dwLength, &dwWriteLength, NULL);
    return dwWriteLength;
}

void Logger::WriteLog( LPCVOID lpBuffer, DWORD dwLength){
    time_t now;
    char temp[11];

    time(&now);
    strftime(temp, 11, "[%H:%M:%S] ", localtime(&now));
    Write(temp, 11);
    Write(lpBuffer, dwLength);
    Write("\r\n", 2);
    FlushFileBuffers(_hFile);
}

void Logger::Lock(){
    ::EnterCriticalSection(&_csLock);
}

void Logger::Unlock(){
    ::LeaveCriticalSection(&_csLock);
}

void Logger::SetFileName(const char *szName){ //change log file name and close previous
    assert(szName);
    if(_szFileName)
        delete[] _szFileName;
    Close();
    _szFileName = new char[strlen(szName) + 1];
    assert(_szFileName);
    strcpy(_szFileName, szName);
}

inline bool Logger::IsOpen(){
    return _hFile != INVALID_HANDLE_VALUE;
}

void Logger::Close(){
    if( IsOpen() ){
        CloseHandle(_hFile);
        _hFile = INVALID_HANDLE_VALUE;
    }
}

void Logger::SetPath(const char *szPath){
    assert(szPath);
    WIN32_FIND_DATA wfd;
    char temp[MAX_PATH + 1] = {0};
    if( FindFirstFile(szPath, &wfd) == INVALID_HANDLE_VALUE && CreateDirectory(szPath, NULL) == 0) {
        printf("create log directory failed[%d]", GetLastError());
        exit(-1);
    } else {
        GetFullPathName(szPath, MAX_PATH, temp, NULL);
        _szPath = new char[strlen(temp) + 1];
        assert(_szPath);
        strcpy(_szPath, temp);
    }
}

