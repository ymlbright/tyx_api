#pragma comment(lib, "Ws2_32.lib")
#include <afx.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "ListenSocket.h"
#include "Config.h"
#include "AdoAccess.h"
#include "Logger.h"

#define MAX_FUNC 2


typedef struct _RemoteAPIData{
    unsigned char funcId;
    char apiKey[32];
    unsigned char cardLen;
    char* card;
    unsigned char pwdLen;
    char* pwd;
}RemoteAPIData;

struct _GlobalInfo{
    SOCKET socketConn;
    HANDLE hPipeRead, hPipeWrite;
    HANDLE hListenProcess;
    HANDLE hWorkerProcess[CONFIG_WorkerProcessNum];
	Logger* logger;
    unsigned int (*func[MAX_FUNC])(ADOAccess &, RemoteAPIData *, char *);
}GlobalInfo;

void init_listenprocess();
DWORD WINAPI listen_process(LPVOID);
void init_workerprocess();
DWORD WINAPI worker_process(LPVOID);

bool init_apidata(SOCKET &, RemoteAPIData* );
void destroy_apidata(RemoteAPIData*);

unsigned int get_paocao(ADOAccess &, RemoteAPIData *, char *);
unsigned int get_healthscore(ADOAccess &, RemoteAPIData *, char *);

void init_socket(){
	GlobalInfo.logger = new Logger(CONFIG_LogPath);

    WSADATA wsaData;
    int err = WSAStartup( MAKEWORD(1, 1), &wsaData);
    if ( err != 0 ){
        printf("failed in starting socket.[%d]\n", GetLastError());
        exit(-1);
    }

    GlobalInfo.socketConn = socket(AF_INET,SOCK_STREAM,0);
    if (GlobalInfo.socketConn==INVALID_SOCKET){
        printf("create socket error[%d].\n", GetLastError());
        exit(-1);
    }
    SOCKADDR_IN sin;
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin.sin_port=htons(CONFIG_ListenPort);
    if ( bind(GlobalInfo.socketConn, (LPSOCKADDR)&sin, sizeof(sin))==SOCKET_ERROR ){
        printf("socket bind error[%d].\n", GetLastError());
        exit(-1);
    }

    int timeout=100;
    setsockopt(GlobalInfo.socketConn, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(int));
    setsockopt(GlobalInfo.socketConn, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(int));

    SECURITY_ATTRIBUTES ai;
    ai.nLength = sizeof(SECURITY_ATTRIBUTES);
    ai.bInheritHandle = true ;
    ai.lpSecurityDescriptor = NULL ;
    if (!CreatePipe(&(GlobalInfo.hPipeRead), &(GlobalInfo.hPipeWrite), &ai , 0)){
        printf("create pipe error[%d].\n", GetLastError());
        exit(-1);
    }

    init_listenprocess();
    init_workerprocess();

	GlobalInfo.logger->Log("TYX API service init finished.");
}
void socket_listen(){
    listen(GlobalInfo.socketConn, 1024);
    for ( int i = 0; i<CONFIG_WorkerProcessNum; i++)
        if ( GlobalInfo.hWorkerProcess[i] != NULL )
            ResumeThread(GlobalInfo.hWorkerProcess[i]);
    ResumeThread(GlobalInfo.hListenProcess);
	GlobalInfo.logger->Log("TYX API service started.");
}

void close_socket(){
    TerminateThread(GlobalInfo.hListenProcess, 0);
    CloseHandle(GlobalInfo.hListenProcess);
    printf("listen process stoped, waitting for worker process finished ...\n");
    
    Sleep(6000);

    printf("worker process finished,  stopping worker process ...\n");
    for ( int i = 0; i<CONFIG_WorkerProcessNum; i++){
        if ( GlobalInfo.hWorkerProcess[i] != NULL ){
            TerminateThread(GlobalInfo.hWorkerProcess[i], 0);
            CloseHandle(GlobalInfo.hWorkerProcess[i]);
        }
    }
    
    CloseHandle(GlobalInfo.hPipeRead);
    CloseHandle(GlobalInfo.hPipeWrite);

    WSACleanup();

	GlobalInfo.logger->Log("TYX API service stoped.");

	delete GlobalInfo.logger;
}

void init_listenprocess(){
    GlobalInfo.hListenProcess = CreateThread(
        NULL,
        0,
        listen_process,
        (LPVOID)&GlobalInfo,
        CREATE_SUSPENDED,
        NULL);
    if ( GlobalInfo.hListenProcess == NULL ){
        printf("create listen process error.\n");
        exit(-1);
    }
}

void init_workerprocess(){
    for ( int i = 0; i<CONFIG_WorkerProcessNum; i++){
        GlobalInfo.hWorkerProcess[i] = CreateThread(
            NULL,
            0,
            worker_process,
            (LPVOID)(&GlobalInfo),
            CREATE_SUSPENDED,
            NULL);
        if ( GlobalInfo.hWorkerProcess[i] == NULL )
            printf("create worker process %d error.\n", i);
    }

    GlobalInfo.func[0] = get_paocao;
    GlobalInfo.func[1] = get_healthscore;
}

unsigned int get_paocao(ADOAccess &ADB, RemoteAPIData *apiData, char *buffer){
    CString ret;
    ret = ADB.GetPaoCao(apiData->card);
    if ( ret.GetLength() > CONFIG_BufferLength ){
        strcpy(buffer, CONFIG_SocketStackOverflow);
        return CONFIG_SocketStackOverflowLen;
    } else {
        strcpy(buffer, ret.GetBuffer(0));
        return ret.GetLength();
    }
}

unsigned int get_healthscore(ADOAccess &ADB, RemoteAPIData *apiData, char *buffer){
    CString ret;
    if ( ADB.IdentityCheck(apiData->card, apiData->pwd) ){
        ret = ADB.GetHealthScore(apiData->card);
        if ( ret.GetLength() > CONFIG_BufferLength ){
            strcpy(buffer, CONFIG_SocketStackOverflow);
            return CONFIG_SocketStackOverflowLen;
        } else {
            strcpy(buffer, ret.GetBuffer(0));
            return ret.GetLength();
        }
    } else {
        strcpy(buffer, CONFIG_SocketPWDError);
        return CONFIG_SocketPWDErrorLen;
    }
}

bool init_apidata(SOCKET &sock, RemoteAPIData* apiData){
    recv(sock, (char*)&(apiData->funcId), 1, 0);
    recv(sock, &(apiData->apiKey[0]), 32, 0);
    recv(sock, (char*)&(apiData->cardLen), 1, 0);
    if ( apiData->cardLen > 254 )
        return false;
    apiData->card = new char[apiData->cardLen+1];
    recv(sock, apiData->card, apiData->cardLen, 0);
    apiData->card[apiData->cardLen] = '\0';
    recv(sock, (char*)&(apiData->pwdLen), 1, 0);
    if ( apiData->pwdLen > 254 ){
        delete[] apiData->card;
        return false;
    }
    apiData->pwd = new char[apiData->pwdLen+1];
    recv(sock, apiData->pwd, apiData->pwdLen, 0);
    apiData->pwd[apiData->pwdLen] = '\0';
    return true;
}

void destroy_apidata(RemoteAPIData* apiData){
    delete[] apiData->card;
    delete[] apiData->pwd;
}

DWORD WINAPI worker_process(LPVOID lpParam){
    _GlobalInfo* GlobalInfo = (_GlobalInfo*)lpParam;
    SOCKET acceptSocket;
    struct sockaddr_in clientAttr;
    int clientAttrLen = sizeof(clientAttr);
    ADOAccess ADB;
    RemoteAPIData apiData;
    char buffer[CONFIG_BufferLength];
    unsigned int dataLen;
	DWORD dwByte;
    char logData[581];

    while(true){
       if ( ReadFile(GlobalInfo->hPipeRead, (LPVOID)&acceptSocket, sizeof(SOCKET), &dwByte, NULL) ) {
            getpeername(acceptSocket, (struct sockaddr*)&clientAttr, &clientAttrLen);
            if ( init_apidata(acceptSocket, &apiData) ){	
                if ( !strncmp(apiData.apiKey, CONFIG_APISecret, 32) ){
                    if ( apiData.funcId < MAX_FUNC ) {
                        sprintf(logData, "%s ACCEPT FuncId:%d Card:%s PwdLength:%d", 
                            inet_ntoa(clientAttr.sin_addr), 
                            apiData.funcId,
                            apiData.card,
                            apiData.pwdLen );
                        GlobalInfo->logger->Log(logData);
                        dataLen = GlobalInfo->func[apiData.funcId](ADB, &apiData, buffer);
                        send(acceptSocket, buffer, dataLen, 0);
                    } else {
                        sprintf(logData, "%s FuncIdError FuncId:%d Card:%s PwdLength:%d", 
                            inet_ntoa(clientAttr.sin_addr), 
                            apiData.funcId,
                            apiData.card,
                            apiData.pwdLen );
                        GlobalInfo->logger->Log(logData);
                        send(acceptSocket, CONFIG_SocketFuncError, CONFIG_SocketFuncErrorLen, 0);
                    }
                } else {
                    sprintf(logData, "%s KeyError FuncId:%d Card:%s PwdLength:%d", 
                        inet_ntoa(clientAttr.sin_addr), 
                        apiData.funcId,
                        apiData.card,
                        apiData.pwdLen );
                    GlobalInfo->logger->Log(logData);
                    send(acceptSocket, CONFIG_SocketAPIKeyError, CONFIG_SocketAPIKeyErrorLen, 0);
                }
                destroy_apidata(&apiData);
            } else {
                sprintf(logData, "%s FormatError", inet_ntoa(clientAttr.sin_addr) );
                GlobalInfo->logger->Log(logData);
                send(acceptSocket, CONFIG_SocketFormatError, CONFIG_SocketFormatErrorLen, 0);
            }
            closesocket(acceptSocket);
        }
    }
}

DWORD WINAPI listen_process(LPVOID lpParam){
    _GlobalInfo* GlobalInfo = (_GlobalInfo*)lpParam;
    SOCKET acceptSocket;
	DWORD dwByte;

    while(true){
        acceptSocket = accept(GlobalInfo->socketConn, NULL, NULL);
        if ( acceptSocket!= INVALID_SOCKET ){
            if ( !WriteFile(GlobalInfo->hPipeWrite, (LPVOID)&acceptSocket, sizeof(SOCKET), &dwByte, NULL) ) {
                send(acceptSocket, CONFIG_SocketErrorMsg, CONFIG_SocketErrorMsgLen, 0);
                closesocket(acceptSocket);
            }
        }
    }
}