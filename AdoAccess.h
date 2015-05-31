#import "msado15.dll" no_namespace rename ("EOF", "adoEOF"),rename ("BOF", "adoBOF")

#ifndef __ADO_ACCESS_H__
#define __ADO_ACCESS_H__
#include <afx.h>

class ADOAccess {
public:
    _ConnectionPtr  m_pConnection;
    ADOAccess();
    ~ADOAccess();
    void DoInitADOConn();
    CString Query(CString strSQL, char* column[], size_t column_n);
    bool IdentityCheck(char* card, char* pwd);
    CString GetPaoCao(char* card);
    CString GetHealthScore(char* card);
    void ExitConnect();
};

#endif
