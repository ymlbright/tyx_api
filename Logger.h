#ifndef _LOGFILE_H
#define _LOGFILE_H

#include <afx.h>
#include <windows.h>

class Logger {
public:
    Logger(const char *_szPath);
    ~Logger();
    const char * GetFileName();
    const char * GetPath();
    void Log(LPCVOID lpBuffer, DWORD dwLength);
    void Log(const char *szText);

protected:
    CRITICAL_SECTION _csLock;
    HANDLE _hFile;
    char * _szFileName;
    char *_szPath;
    char _szLastDate[10];
    
    bool OpenFile();
    inline DWORD Write(LPCVOID lpBuffer, DWORD dwLength);
    void WriteLog( LPCVOID lpBuffer, DWORD dwLength);
    inline bool IsOpen();
    void Close();
    void Lock();
    void Unlock(); 
    void SetFileName(const char *szName);
    void SetPath(const char *szPath);
private://屏蔽函数
    Logger(const Logger&);
    Logger &operator = (const Logger&);
};

#endif