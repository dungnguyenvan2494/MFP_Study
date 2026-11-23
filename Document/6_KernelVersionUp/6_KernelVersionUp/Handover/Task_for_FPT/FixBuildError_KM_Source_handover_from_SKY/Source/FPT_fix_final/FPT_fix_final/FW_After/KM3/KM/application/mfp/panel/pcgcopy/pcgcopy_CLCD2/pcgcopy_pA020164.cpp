//------------------------------------------------------------------------------
/**
 * @file pcgcopy_pA020164.cpp
 * @brief 画面クラスの実装（ポップアップ画面）.
 */
//------------------------------------------------------------------------------

//================================================================================================
//  インクルード・ファイル.
//================================================================================================
#include <panel/plibh/plibh_CLCD/plibh_Mtui.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pcgcopy_pA020164.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/plibh/plibh_CLCD2/plibhinclude_CLCD2.h>
#include <panel/plibh/plibh_CLCD2/piglib_pA010194.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OutsideKeyCallDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_SoftSlideMenuTenKeyTextDispLib.h"

/*================================================================================================
  定数・型の宣言.
================================================================================================*/

/*================================================================================================
  列挙型変数のタグ宣言.
================================================================================================*/

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/
//------------------------------------------------------------------------------
/**
* @brief 共通処理 : コンストラクタ.
*/
//------------------------------------------------------------------------------
PCPC_AuxilPageInsertEdit_copy_com::PCPC_AuxilPageInsertEdit_copy_com(
		PDisplayParts** partsTable, 
		short partsNumber, 
		Bool isNoWarningGamen, 
		PPopupAnimationGroup* pc_AnimationComponent,
		short ss_PosX,
		short ss_PosY, 
		short ss_SizeX, 
		short ss_SizeY, 
		PDisplayParts* pc_FunctionBadgeButton
	): PPopupDecisionButtonCom( partsTable, partsNumber, isNoWarningGamen, pc_AnimationComponent, ss_PosX, ss_PosY, ss_SizeX, ss_SizeY, pc_FunctionBadgeButton)
{
	m_uc_NowPage = PNLE_AuxilPageInsertEditComPageNumber01;
	m_ss_InputPageNumber = 0;
	m_ss_InputPageNumberBackup = 0;

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号の設定.
		m_s_PageNumberStatus[i].ss_PageNumber = 0;

		// 入力済みフラグの設定.
		m_s_PageNumberStatus[i].b_Input = False;

		// ページ番号の選択状態を設定.
		m_s_PageNumberStatus[i].b_Select = False;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::startGamen()
{
	PPopupDecisionButtonCom::startGamen();

	m_uc_NowPage = PNLE_AuxilPageInsertEditComPageNumber01;
	m_ss_InputPageNumber = 0;
	m_ss_InputPageNumberBackup = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : テンキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::proc10Key(uchar option)
{
	Bool b_Error = False;

	// 保持値が０の場合は０入力禁止.
	if( 0 == m_ss_InputPageNumber && 0 == option ) {
		PBasicMessageProc::setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( option <= 9 ) {
		// テンキーで入力された値の算出.
		m_ss_InputPageNumber = ( short )proc10KeyInput( ( int32_t )( m_ss_InputPageNumber ),PCGD_MaxPageNumber, option, False, EP_InputMaxLimitPattern_Refuse, &b_Error );
		// エラー時は警告音を鳴らす.
		if( b_Error ) {
			PBasicMessageProc::setBuzzerType( PCGE_BuzzerType_InputNg );
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : クリアー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::procClear(uchar option)
{
	if( m_ss_InputPageNumber == 0 && checkPageSelectStatus() == False ) {

		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// ページ番号の選択を解除する.
	clearPageSelectStatus();

	// 入力エリアの値を初期化する.
	clearInputPageNumber();

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::getPageNumber( short* const pss_PageNumber )
{
	// 検索開始位置を設定する.
	const uchar uc_StartIndex = ((m_uc_NowPage == PNLE_AuxilPageInsertEditComPageNumber01) ? uchar(0) : uchar(PNLD_InsertPageNumber_HALF));

	for( uchar i = 0, j = uc_StartIndex; PNLD_InsertPageNumber_HALF > i; i++, j++) {

		// ページ番号を取得する.
		pss_PageNumber[i] = m_s_PageNumberStatus[j].ss_PageNumber;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::setPageNumber( const short* pss_PageNumber )
{
	// 引数のNULLチェック.
	if( NULL == pss_PageNumber ) {

		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号の設定.
		m_s_PageNumberStatus[i].ss_PageNumber = pss_PageNumber[i];

		// 入力済みフラグの設定.
		m_s_PageNumberStatus[i].b_Input = ((0 == pss_PageNumber[i]) ? (False) : (True));

		// ページ番号の選択状態を設定.
		m_s_PageNumberStatus[i].b_Select = False;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の選択状態を取得する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageInsertEdit_copy_com::getPageSelectStatus( uchar uc_Index )
{
	if( m_uc_NowPage == PNLE_AuxilPageInsertEditComPageNumber02 ) {

		uc_Index = ( uchar )( uc_Index + PNLD_InsertPageNumber_HALF );

	}

	// ページ番号の選択状態を取得する.
	return m_s_PageNumberStatus[uc_Index].b_Select;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の選択状態を設定する.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::setPageSelectStatus( PNLE_AuxilPageInsertEditComSelectPageNumber e_SelectPageNumber )
{
	uchar uc_Index = e_SelectPageNumber;

	if( m_uc_NowPage == PNLE_AuxilPageInsertEditComPageNumber02 ) {

		uc_Index = ( uchar )( uc_Index + PNLD_InsertPageNumber_HALF );

	}

	if( m_s_PageNumberStatus[uc_Index].b_Select == True ) {

		m_s_PageNumberStatus[uc_Index].b_Select = False;

	} else {

		m_s_PageNumberStatus[uc_Index].b_Select = True;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号を非選択にする.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::clearPageSelectStatus()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {
		// ページ番号を非選択にする.
		m_s_PageNumberStatus[i].b_Select = False;
	}

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号が選択されているかどうか判定する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageInsertEdit_copy_com::checkPageSelectStatus()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			return True;

		}
	}

	return False;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ数を取得する.
*/
//------------------------------------------------------------------------------
uchar PCPC_AuxilPageInsertEdit_copy_com::getNowPage()
{
	uchar uc_PageNumber = PNLE_AuxilPageInsertEditComPageNumber01;
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			if( PNLD_InsertPageNumber_HALF <= i ) {

				uc_PageNumber = PNLE_AuxilPageInsertEditComPageNumber02;
			}
			break;
		}
	}

	return uc_PageNumber;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の登録数を取得する.
*/
//------------------------------------------------------------------------------
uchar PCPC_AuxilPageInsertEdit_copy_com::getPageNumberCount()
{
	// ページ番号の登録数.
	uchar uc_count = 0;

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号が設定されている場合.
		if( m_s_PageNumberStatus[i].ss_PageNumber != 0 ) {

			// ページ番号の登録数をインクリメントする.
			uc_count++;
		}
	}

	return uc_count;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::getPageNumberSetting( short* const pss_PageNumber )
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		// ページ番号を取得する.
		pss_PageNumber[i] = m_s_PageNumberStatus[i].ss_PageNumber;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 追加されたページ番号の位置を検索する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageInsertEdit_copy_com::SearchPageNumber()
{
	Bool b_ret = False;
	uchar i;
	for( i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].ss_PageNumber == m_ss_InputPageNumberBackup ) {

			break;

		}
	}

	if( PNLD_InsertPageNumber_HALF <= i ) {

		b_ret = True;

	}

	return b_ret;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 重複しているページ番号があるかどうか判定する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageInsertEdit_copy_com::checkPageNumber()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].ss_PageNumber == m_ss_InputPageNumber ) {

			return True;

		}
	}

	return False;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 入力エリアの値を取得.
*/
//------------------------------------------------------------------------------
short PCPC_AuxilPageInsertEdit_copy_com::getInputPageNumber()
{
	return m_ss_InputPageNumber;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 入力エリアの値をクリア.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::clearInputPageNumber()
{
	m_ss_InputPageNumber = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の更新.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::renewPageNumber()
{
	for (uchar i = 0; PNLD_InsertPageNumber_Max > i; i++) {

		// ページ番号が0の場合.
		if( m_s_PageNumberStatus[i].ss_PageNumber == 0 ) {
			
			// ページ番号を設定する.
			m_s_PageNumberStatus[i].ss_PageNumber = m_ss_InputPageNumber;
			m_s_PageNumberStatus[i].b_Input = True;

			// 入力エリアの値をバックアップする.
			m_ss_InputPageNumberBackup = m_ss_InputPageNumber;

			// 入力エリアの値を初期化する.
			clearInputPageNumber();

			break;
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の削除.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::deletePageNumber()
{
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			// ページ番号を削除する.
			m_s_PageNumberStatus[i].ss_PageNumber = 0;
			m_s_PageNumberStatus[i].b_Input = False;
			m_s_PageNumberStatus[i].b_Select = False;

		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ソート.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageInsertEdit_copy_com::procSortExec()
{
	short ss_temp;
	Bool  bl_temp;
	ushort i,j;
	ushort us_Number_Cnt_Max = PNLD_InsertPageNumber_Max;

	for( i=0; i < ( ushort )( us_Number_Cnt_Max-1 ); i++ ) {

		for( j=0; j < ( ushort )( us_Number_Cnt_Max-1-i ); j++ ) {

			if( ((m_s_PageNumberStatus[j+1].b_Input == True) && (m_s_PageNumberStatus[j+1].ss_PageNumber != 0)) &&
				((m_s_PageNumberStatus[j].b_Input == False) ||
				(m_s_PageNumberStatus[j].ss_PageNumber == 0) ||
				((m_s_PageNumberStatus[j].ss_PageNumber != 0) && (m_s_PageNumberStatus[j].ss_PageNumber > m_s_PageNumberStatus[j+1].ss_PageNumber))) ) {

				ss_temp = m_s_PageNumberStatus[j].ss_PageNumber;
				m_s_PageNumberStatus[j].ss_PageNumber = m_s_PageNumberStatus[j+1].ss_PageNumber;
				m_s_PageNumberStatus[j+1].ss_PageNumber = ss_temp;

				bl_temp = m_s_PageNumberStatus[j].b_Input;
				m_s_PageNumberStatus[j].b_Input = m_s_PageNumberStatus[j+1].b_Input;
				m_s_PageNumberStatus[j+1].b_Input = bl_temp;
					
				bl_temp = m_s_PageNumberStatus[j].b_Select;
				m_s_PageNumberStatus[j].b_Select = m_s_PageNumberStatus[j+1].b_Select;
				m_s_PageNumberStatus[j+1].b_Select = bl_temp;

			}
		}
	}
}
