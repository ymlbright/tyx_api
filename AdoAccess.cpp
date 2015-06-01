#include <afx.h>
#include <stdio.h>
#include "AdoAccess.h"
#include "Config.h"

ADOAccess::ADOAccess(){
    DoInitADOConn();
}
ADOAccess::~ADOAccess(){
    ExitConnect();
}

inline CString GetCollect(_RecordsetPtr pRecord, char* collect){
    _variant_t var;
    var = pRecord->GetCollect(collect);
    if ( var.vt != VT_NULL )
        return (LPCSTR)_bstr_t(var);
    return CString("");
}

CString ADOAccess::Query(CString strSQL, char* column[], size_t column_n){
	_RecordsetPtr pQueryRecordset;
    pQueryRecordset.CreateInstance("ADODB.Recordset");
    try{
        pQueryRecordset->Open(_variant_t(strSQL), m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);
        if ( pQueryRecordset->adoBOF && pQueryRecordset->adoEOF ){
            pQueryRecordset->Close();
            return CString("");
        } else {
            CString result;
            for ( size_t i = 0; i<column_n-1; i++)
                result += GetCollect(pQueryRecordset, column[i]) + CString(",");
            result += GetCollect(pQueryRecordset, column[column_n-1]);
            pQueryRecordset->Close();
            return result;
        }
    } catch (_com_error e) {
        printf("DB error when execute '%s'\n  %s", strSQL, e.ErrorMessage());
        pQueryRecordset->Close();
        return CString("");
    }
}

bool ADOAccess::IdentityCheck(char* card, char* pwd){
    CString strSQL;
    static char* column="studentNo";
    strSQL.Format("SELECT studentNo FROM STUDENT WHERE studentNo='%s' AND passWord='%s'", card, pwd);
    return Query(strSQL, &column, 1)!=CString("");
}

CString ADOAccess::GetPaoCao(char* card){
    CString strSQL;
    static char* column="C1";
    strSQL.Format("SELECT C1 FROM SportCheck WHERE studentNo='%s'", card);
    return Query(strSQL, &column, 1);
}

CString ADOAccess::GetHealthScore(char* card){
    CString strSQL;
    static char* column[24];
    column[0] = "stature";                  //身高
    column[1] = "avoirdupois";              //体重
    column[2] = "vitalCapacity";            //肺活量
    column[3] = "vitalCapacityScore";       //肺活量分数 
    column[4] = "vitalCapacityConclusion";  //肺活量评价
    column[5] = "fiftyMeter";               //50米 s
    column[6] = "fiftyMeterScore";          //50米分数
    column[7] = "fiftyMeterConclusion";     //50米评价
    column[8] = "standingLongJump";         //立定跳远
    column[9] = "standingLongJumpScore";    //立定跳远分数
    column[10] = "standingLongJumpConclusion";  //立定跳远评价
    column[11] = "BMI";                     //BMI
    column[12] = "BMIScore";                //BMI分数
    column[13] = "BMIConclusion";           //BMI评价
    column[14] = "kiloMeter";               //800/1000米
    column[15] = "kiloMeterScore";          //800/1000米分数
    column[16] = "kiloMeterConclusion";     //800/1000米评价
    column[17] = "bend";                    //坐体前屈
    column[18] = "bendScore";               //坐体前屈分数
    column[19] = "bendConclusion";          //坐体前屈评价
    column[20] = "lie";                     //仰卧起坐/引体向上
    column[21] = "lieScore";                //仰卧起坐/引体向上分数
    column[22] = "lieConclusion";           //仰卧起坐/引体向上评价
    column[23] = "conclusion";              //总分
    strSQL.Format("SELECT * FROM healthscore WHERE studentNo='%s'", card);
    return Query(strSQL, column, 24);
}

void ADOAccess::DoInitADOConn(){
    ::CoInitialize(NULL);
    try{
        m_pConnection.CreateInstance("ADODB.Connection");
		_bstr_t strConnect = CONFIG_ConnectString;
        m_pConnection->Open(strConnect, "", "", adModeUnknown);
        //printf("Connect to DB successed.\n");
    } catch (_com_error e) {
        printf("Connect to DB failed.\n");
        exit(-2);
    }
}

void ADOAccess::ExitConnect(){
    m_pConnection->Close();
    ::CoUninitialize();
    //printf("Disconnect from DB.\n");
}