
#include "SKB_RomajiConverter.h"
#include "SKB_JapaneseInputEngine.h"

#include <QtGlobal>
#include <QTextCodec>

#include <unicode/translit.h>

/**
 * @brief コンストラクタ.
 *
 */
SKBC_JapaneseInputEngine::SKBC_JapaneseInputEngine()
	: m_bl_KeyAccepted( false )
	, m_bl_CommitStringChanged( false )
	, m_bl_PreeditStringChanged( false )
	, m_bl_CandidateListChanged( false )
	, m_bl_enableL2Converter( false )
	, m_ss_PreeditCursorPos( -1 )
	, m_sl_CandidateItemIndex( -1 )
	, m_ss_CandidateItemCount( 0 )
	, m_ss_CommitCount( 0 )
	, m_e_TargetLayer( SKBC_ComposingText::SKBE_LAYER1 )
{
	::memset( m_sc_CandidateBuffer, 0, sizeof( m_sc_CandidateBuffer ) );
	m_c_CChg.SetVjeModeOn();
}

/**
 * @brief デストラクタ.
 *
 */
SKBC_JapaneseInputEngine::~SKBC_JapaneseInputEngine()
{
	m_c_CChg.SetVjeModeOff();
}

/**
 * @brief 入力種別の切り替えを受け付ける場合 True を返す.
 *
 * @param e_Type 変更する入力種別をあらわす.
 * @return Bool 切り替えを受け付ける場合 True を返す.
 */
Bool SKBC_JapaneseInputEngine::setInputMode(IMFE_KEYBOARD_INPUTMODE_TYPE e_Type)
{
	Bool bl_Accepted = False;
	switch ( e_Type ) {
	case IMFE_KEYBOARD_INPUTMODE_HIRAGANA:
	case IMFE_KEYBOARD_INPUTMODE_KATAKANA:
	case IMFE_KEYBOARD_INPUTMODE_LATIN:
	case IMFE_KEYBOARD_INPUTMODE_NUMERIC:
	case IMFE_KEYBOARD_INPUTMODE_CODE:
		fitInputType();
		bl_Accepted = True;
		break;
	default:
		break;
	}
	return ( bl_Accepted );
}

/**
 * @brief 入力種別(小文字/大文字)の切り替えを受け付ける場合 True を返す.
 *
 * @param e_Type 変更する入力種別(小文字/大文字)をあらわす.
 * @return Bool 切り替えを受け付ける場合 True を返す.
 */
Bool SKBC_JapaneseInputEngine::setTextCase(IMFE_KEYBOARD_TEXTCASE_TYPE e_Type)
{
	Q_UNUSED(e_Type)
	// 小文字/大文字の切替にて未確定文字列、確定文字列、変換候補は変更しない.
	m_bl_CommitStringChanged = false;
	m_bl_PreeditStringChanged = false;
	m_bl_CandidateListChanged = false;
	return ( True );
}

/**
 * @brief 入力種別(小文字/大文字)の切り替えを受け付ける場合 True を返す.
 *
 * @param e_Type 変更する入力種別(小文字/大文字)をあらわす.
 * @return Bool 切り替えを受け付ける場合 True を返す.
 */
Bool SKBC_JapaneseInputEngine::setFullHalf(IMFE_KEYBOARD_FULLHALF_TYPE e_Type)
{
	Q_UNUSED(e_Type)
	fitInputType();
	return ( True );
}

/**
 * @brief 入力種別(通常/特殊)の切り替えを受け付ける場合 True を返す.
 *
 * @param e_Type 変更する入力種別(通常/特殊)をあらわす.
 * @return Bool 切り替えを受け付ける場合 True を返す.
 */
Bool SKBC_JapaneseInputEngine::setCharMode(IMFE_KEYBOARD_INPUTCHAR_TYPE e_Type)
{
	Q_UNUSED(e_Type)
	// 通常/特殊の切替にて未確定文字列、確定文字列、変換候補は変更しない.
	m_bl_CommitStringChanged = false;
	m_bl_PreeditStringChanged = false;
	m_bl_CandidateListChanged = false;
	return ( True );
}

/**
 * @brief キー入力を受け付ける場合 True を返す.それ以外の場合 False を返す.
 *
 * @return Bool キー入力を受け付ける場合 True を返す.それ以外の場合 False を返す.
 */
Bool SKBC_JapaneseInputEngine::keyAccepted()
{
	Bool bl_Accepted = False;
	if ( m_bl_KeyAccepted ) {
		bl_Accepted = True;
	}
	return ( bl_Accepted );
}

/**
 * @brief 入力キー情報とコンテキスト情報を元に更新する.
 *
 * @param sl_Key　入力キーをあらわす.IMFE_KEYBOARD_KEY_ID 、あるいはUTF-8にて保持する.
 * @param sc_Text　入力キーにより入力される文字をあらわす.キー押下による入力文字が無い場合(矢印キーなど)、空文字を指定する.
 * @param e_Modifiers　修飾キーをあらわす.
 * @return Bool キー入力を処理した場合 True を返す.それ以外の場合 False を返す.
 */
Bool SKBC_JapaneseInputEngine::keyEvent(int32_t sl_Key, const char *pc_Text, IMFE_KEYBOARD_MODIFIERS e_Modifiers)
{
	Q_UNUSED( e_Modifiers )

	Bool bl_Accepted = False;

	SKBC_LetterConverter* pc_PreConverter = currentPreConverter();
	m_c_CChg.VjeCNVCancel();
	if ( NULL == pc_PreConverter ) {
		return ( false );
	}

	if ( NULL != m_pc_CurrentContext ) {
		m_sl_CandidateItemIndex = m_pc_CurrentContext->getCandidateListSelectedIndex();
	}
	m_bl_KeyAccepted = false;
	m_c_CommitString.clear();
	m_c_PreeditString.clear();
	m_ss_PreeditCursorPos = -1;
	m_bl_CommitStringChanged = false;
	m_bl_PreeditStringChanged = false;
	m_bl_CandidateListChanged = false;

	QString c_Text = QString::fromUtf8( pc_Text );
	switch ( sl_Key ) {
	case IMFE_KEYBOARD_KEY_LEFT:
		if ( true == m_bl_enableL2Converter && 0 < m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) ) {
			bl_Accepted = ( true == processLeftKey() ) ? True : False;
		} else if ( 0 < m_c_ComposingText.size(SKBC_ComposingText::SKBE_LAYER0) ) {
			commitText();
			m_bl_KeyAccepted = true;
			bl_Accepted = True;
		}
		break;
	case IMFE_KEYBOARD_KEY_RIGHT:
		if ( true == m_bl_enableL2Converter && 0 < m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) ) {
			bl_Accepted = ( true == processRightKey() ) ? True : False;
		} else if ( 0 < m_c_ComposingText.size(SKBC_ComposingText::SKBE_LAYER0) ) {
			commitText();
			m_bl_KeyAccepted = true;
			bl_Accepted = True;
		}
		break;
	case IMFE_KEYBOARD_KEY_BACKSPACE:
		if ( 0 < m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) ) {
			bl_Accepted = True;
			m_bl_KeyAccepted = true;
			if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
				m_c_ComposingText.setCursor(
							SKBC_ComposingText::SKBE_LAYER1
							, m_c_ComposingText.getString( SKBC_ComposingText::SKBE_LAYER1 ).length()
							);
				m_sl_CandidateItemIndex = -1;
			} else {
				if ( 1 == m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) && 0 != m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER1 ) ) {
					initialize();
					m_bl_CommitStringChanged = true;
					m_bl_PreeditStringChanged = true;
					m_bl_CandidateListChanged = true;
					return ( true );
				} else {
					m_c_ComposingText.deleteAt( SKBC_ComposingText::SKBE_LAYER1 );
				}
			}
			updateContext( SKBC_ComposingText::SKBE_LAYER1, true, true );
		}
		break;
	case IMFE_KEYBOARD_KEY_SPACE:
		if ( 0 == m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER0 ) ) {
			// 文字が入力されていない場合、カスタムエンジンにてキーは受け付けない.共通エンジンで処理する.
		} else {
			if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
				updateL2Segment( focusCandidate( m_sl_CandidateItemIndex + 1 ) );
			} else if ( true == m_bl_enableL2Converter ) {
				updateL2Segment( startConvert() );
			} else {
				commitText();
			}
			m_bl_KeyAccepted = true;
			bl_Accepted = True;
		}
		break;
	case IMFE_KEYBOARD_KEY_RETURN:
		if ( 0 < m_c_ComposingText.size(SKBC_ComposingText::SKBE_LAYER0) ) {
			commitText();
			m_bl_KeyAccepted = true;
			bl_Accepted = True;
		}
		break;
	// AR1027289対応.
	case IMFE_KEYBOARD_KEY_IME_CONTROL:
		// IME表示モードに変更した場合は未確定文字列を削除する.
		initialize();
		m_bl_CommitStringChanged = true;
		m_bl_PreeditStringChanged = true;
		m_bl_CandidateListChanged = true;
		m_bl_KeyAccepted = true;
		bl_Accepted = True;
		break;
	default:
		if ( IMFE_KEYBOARD_KEY_FUNC > sl_Key && true != c_Text.isEmpty() && true == c_Text.at( 0 ).isPrint() ) {
			if ( SKBD_CCHG_JP_MAX_PREEDIT > m_c_ComposingText.size(SKBC_ComposingText::SKBE_LAYER1) ) {
				if ( true == m_bl_enableL2Converter ) {
					commitConvertingText();
					m_c_ComposingText.insertStrSegment( SKBC_ComposingText::SKBE_LAYER0, SKBC_ComposingText::SKBE_LAYER1, c_Text );
					pc_PreConverter->convert( m_c_ComposingText );
					m_ss_CommitCount = 0;
					updateContext( SKBC_ComposingText::SKBE_LAYER1, true, true );
				} else {
					m_c_ComposingText.insertStrSegment( SKBC_ComposingText::SKBE_LAYER0, SKBC_ComposingText::SKBE_LAYER1, c_Text );
					QString c_StrLayer1 = m_c_ComposingText.getString( SKBC_ComposingText::SKBE_LAYER1 );
					if ( true != isAlphabetLastChar( c_StrLayer1 ) ) {
						commitText();
					} else {
						if ( pc_PreConverter->convert( m_c_ComposingText ) ) {
							commitTextWithoutLastAlphabet();
						} else {
							updateContext( SKBC_ComposingText::SKBE_LAYER1, true, true );
						}
					}
				}
				m_bl_KeyAccepted = true;
			}
			bl_Accepted = True;
		}
		break;
	}

	return ( bl_Accepted );
}

/**
 * @brief keyEventにより生成された確定文字を返す.
 *
 * @param pc_Text 文字列を格納する領域の先頭アドレスをあらわす.
 * @param sl_Buflen 文字列を格納する領域の長さをあらわす.
 */
void SKBC_JapaneseInputEngine::getCommitString(char *pc_Text, int32_t sl_Buflen)
{
	if ( NULL != pc_Text ) {
		QByteArray c_StrArray = QTextCodec::codecForName( "UTF-8" )->fromUnicode( m_c_CommitString );
		::strncpy(
					pc_Text
					, c_StrArray.constData()
					, sl_Buflen
					);
	}
}

/**
 * @brief keyEventにより生成された未確定文字を返す.
 *
 * @param pc_Text 文字列を格納する領域の先頭アドレスをあらわす.
 * @param sl_Buflen 文字列を格納する領域の長さをあらわす.
 */
void SKBC_JapaneseInputEngine::getPreeditString(char *pc_Text, int32_t sl_Buflen)
{
	if ( NULL != pc_Text ) {
		QByteArray c_StrArray = QTextCodec::codecForName( "UTF-8" )->fromUnicode( m_c_PreeditString );
		::strncpy(
					pc_Text
					, c_StrArray.constData()
					, sl_Buflen
					);
	}
}

/**
 * @brief 確定文字列のカーソル位置を返す.
 * 先頭を0とする.
 *
 * @return short 先頭からのカーソル位置をあらわす(default = 0).
 */
short SKBC_JapaneseInputEngine::getCommitedCursorPosition()
{
	short ss_CursorPos = 0;
	if ( NULL != m_pc_CurrentContext ) {
		ss_CursorPos = m_pc_CurrentContext->getCommitedCursorPosition();
	}
	ss_CursorPos += m_c_CommitString.length();
	return ( ss_CursorPos );
}

/**
 * @brief 未確定文字列のカーソル位置を返す.
 * 先頭を0とする.
 *
 * @return short 先頭からのカーソル位置をあらわす(default = 0).
 */
short SKBC_JapaneseInputEngine::getPreeditCursorPosition()
{
	return ( m_ss_PreeditCursorPos );
}

/**
 * @brief keyEventにより確定文字列が変更した場合、 True を返す.
 *
 * @return Bool keyEventにより確定文字列が変更した場合、 True を返す.
 */
Bool SKBC_JapaneseInputEngine::CommitStringChanged()
{
	return ( m_bl_CommitStringChanged ? True : False );
}

/**
 * @brief keyEventにより未確定文字列が変更した場合、 True を返す.
 *
 * @return Bool keyEventにより未確定文字列が変更した場合、 True を返す.
 */
Bool SKBC_JapaneseInputEngine::PreeditStringChanged()
{
	return ( m_bl_PreeditStringChanged ? True : False );;
}

/**
 * @brief 変換候補の数をあらわす.
 *
 * @return int32_t 変換候補の数をあらわす.
 */
int32_t SKBC_JapaneseInputEngine::getCandidateListItemCount()
{
	int32_t sl_Count = 0;
	if ( 0 < m_ss_CandidateItemCount ) {
		sl_Count = SKBE_CUSTOM_CANDIDATE_FRONT_MAX + SKBE_CUSTOM_CANDIDATE_END_MAX + m_ss_CandidateItemCount -1;
	}
	return ( sl_Count );
}

/**
 * @brief インデクスにて指定された変換候補文字を返す.
 *
 * @param sl_Index インデクスをあらわす.
 * @param pc_Text 変換候補文字を格納する領域の先頭アドレスをあらわす.
 * @param sl_Buflen 変換候補文字を格納する領域の長さをあらわす.
 */
void SKBC_JapaneseInputEngine::getCandidateListData(int32_t sl_Index, char *pc_Text, int32_t sl_Buflen)
{
	QTextCodec* pc_Codec = QTextCodec::codecForName( "shift-jis" );
	int32_t sl_CandidateListItemCount= getCandidateListItemCount();
		
	if ( NULL != pc_Codec && 0 <= sl_Index && sl_CandidateListItemCount > sl_Index ) {
		QByteArray c_StrArray;
		if ( SKBE_CUSTOM_CANDIDATE_HIRAGANA == sl_Index ) {
			c_StrArray = m_c_CandidateHiraganaItem.toUtf8();
		} else if ( SKBE_CUSTOM_CANDIDATE_FULL_KATAKANA == sl_Index ) {
			c_StrArray = str2FullKatakana( m_c_CandidateHiraganaItem ).toUtf8();
		} else if ( SKBE_CUSTOM_CANDIDATE_STROKE == sl_Index ) {
			c_StrArray = m_c_CandidateStrokeItem.toUtf8();
		} else if  ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_HALF_KATAKANA )  == sl_Index ) {
			c_StrArray = str2HalfKatakana( m_c_CandidateHiraganaItem ).toUtf8();
		} else if  ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_FULL )  == sl_Index ) {
			c_StrArray = str2FullLatin( m_c_CandidateStrokeItem ).toUtf8();
		} else if ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_LOWER )  == sl_Index ) {
			c_StrArray = m_c_CandidateStrokeItem.toLower().toUtf8();
		} else if ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_UPPER )  == sl_Index ) {
			c_StrArray = m_c_CandidateStrokeItem.toUpper().toUtf8();
		} else {
			c_StrArray = pc_Codec->toUnicode( m_c_CandidateList.at( sl_Index - SKBE_CUSTOM_CANDIDATE_FRONT_MAX ) ).toUtf8();
		}
		::strncpy(
					pc_Text
					, c_StrArray.constData()
					, sl_Buflen
					);
	}
}

/**
 * @brief インデクスにて指定された変換候補文字を選択する.
 *
 * @param sl_Index インデクスをあらわす.
 */
void SKBC_JapaneseInputEngine::selectCandidateListItem(int32_t sl_Index)
{
	// 変換中は選択候補箇所を選択された場合においても確定処理をすすめる.
	// 変換中でない場合は同一インデクスの選択は処理しない.
	m_bl_CommitStringChanged = false;
	m_bl_PreeditStringChanged = false;
	m_bl_CandidateListChanged = false;

	if ( SKBC_ComposingText::SKBE_LAYER2 != m_e_TargetLayer && m_sl_CandidateItemIndex == sl_Index ) {
		m_bl_KeyAccepted = false;
		return;
	}

	m_c_CommitString.clear();
	m_c_PreeditString.clear();
	m_ss_PreeditCursorPos = -1;

	m_sl_CandidateItemIndex = sl_Index;
	if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
		updateL2Segment( focusCandidate( m_sl_CandidateItemIndex ) );
		commitText();
		m_bl_KeyAccepted = true;
	} else if ( true == m_bl_enableL2Converter ) {
		updateL2Segment( startConvert( false ) );
		commitText();
		m_bl_KeyAccepted = true;
	} else {
		// no op.
		m_bl_KeyAccepted = false;
	}
}

/**
 * @brief 現在の入力モードに従い、変換エンジンの設定を変更する.
 *
 */
void SKBC_JapaneseInputEngine::fitInputType()
{
	// 入力モードの変更に従い、確定文字列・未確定文字列のローカル情報をクリアする.
	// クリアしたため変更フラグを true に設定する.
	initialize();
	m_bl_CommitStringChanged = true;
	m_bl_PreeditStringChanged = true;
	m_bl_CandidateListChanged = true;
}

/**
 * @brief keyEventにより変換候補が変更した場合、 True を返す.
 *
 * @return Bool keyEventにより変換候補が変更した場合、 True を返す(default = False).
 */
Bool SKBC_JapaneseInputEngine::CandidateListChanged()
{
	return ( m_bl_CandidateListChanged ? True : False );;
}

/**
 * @brief 選択されているインデクスを返す.
 *
 * @return int32_t インデクスをあらわす(default = -1).
 */
int32_t SKBC_JapaneseInputEngine::getCandidateSelectedIndex()
{
	return ( m_sl_CandidateItemIndex );
}

/**
 * @brief 確定文字列のカーソル位置を変更する.
 *
 * @param ss_CursorPosition　文字列先頭からのカーソル位置をあらわす.先頭を0とする.
 */
void SKBC_JapaneseInputEngine::setCommitCursorPosition(int32_t sl_CursorPosition)
{
	Q_UNUSED( sl_CursorPosition )
	m_bl_KeyAccepted = True;
}

/**
 * @brief 未確定文字列のカーソル位置を変更する.
 *
 * @param ss_CursorPosition　文字列先頭からのカーソル位置をあらわす.先頭を0とする.
 */
void SKBC_JapaneseInputEngine::setPreeditCursorPosition(int32_t sl_CursorPosition)
{
	m_bl_KeyAccepted = false;
	m_c_CommitString.clear();
	m_c_PreeditString.clear();
	m_ss_PreeditCursorPos = -1;
	m_bl_CommitStringChanged = false;
	m_bl_PreeditStringChanged = false;
	m_bl_CandidateListChanged = false;

	if ( true == m_bl_enableL2Converter && 0 < m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) ) {
		if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
			// 変換状態ならば、タップによるカーソル移動は受け付けない.
		} else {
			if ( 0 <= sl_CursorPosition && m_c_ComposingText.getStringLayer( SKBC_ComposingText::SKBE_LAYER1 ).length() >= sl_CursorPosition ) {
				m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, sl_CursorPosition - m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER1 ) );
				m_bl_KeyAccepted = true;
				// 変換対象が変わったため、再度変換をかける.
				m_ss_CommitCount = 0;
				updateContext( m_e_TargetLayer, true, true );
				if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
					updateL2Segment( focusCandidate( m_sl_CandidateItemIndex ) );
				} else {
					// no op.
				}
			} else {
				// no op.
			}
		}
	}
}

/**
 * @brief 事前変換エンジンを返す.
 *
 * @return SKBC_LetterConverter 事前変換エンジンをあらわす.
 */
SKBC_LetterConverter *SKBC_JapaneseInputEngine::currentPreConverter()
{
	static SKBC_RomajiConverter c_RomajiConverter;
	static SKBC_RomajiFullKatakanaConverter c_RomajiFullKatakanaConverter;
	static SKBC_RomajiHalfKatakanaConverter c_RomajiHalfKatakanaConverter;

	SKBC_LetterConverter* pc_PreConverter = NULL;
	if ( NULL != m_pc_CurrentContext ) {
		IMFE_KEYBOARD_INPUTMODE_TYPE e_InputMode = m_pc_CurrentContext->getKeyboardInputMode();
		IMFE_KEYBOARD_FULLHALF_TYPE e_FullHalf = m_pc_CurrentContext->getKeyboardFullHalf();
		switch ( e_InputMode ) {
		case IMFE_KEYBOARD_INPUTMODE_HIRAGANA:
			pc_PreConverter = &c_RomajiConverter;
			m_bl_enableL2Converter = true;
			break;
		case IMFE_KEYBOARD_INPUTMODE_KATAKANA:
			m_bl_enableL2Converter = false;
			if ( IMFE_KEYBOARD_FULLHALF_FULL == e_FullHalf ) {
				pc_PreConverter = &c_RomajiFullKatakanaConverter;
			} else if ( IMFE_KEYBOARD_FULLHALF_HALF == e_FullHalf ) {
				pc_PreConverter = &c_RomajiHalfKatakanaConverter;
			} else {
				// no op.
			}
			break;
		default:
			m_bl_enableL2Converter = false;
			break;
		}
	}
	return ( pc_PreConverter );
}

/**
 * @brief 初期化する.
 *
 */
void SKBC_JapaneseInputEngine::initialize()
{
	m_c_CandidateList.clear();
	m_e_TargetLayer = SKBC_ComposingText::SKBE_LAYER1;
	m_sl_CandidateItemIndex = -1;
	m_ss_CandidateItemCount = 0;
	m_c_ComposingText.clear();
	m_c_PreeditString.clear();
	m_c_CommitString.clear();
	m_ss_PreeditCursorPos = 0;
	m_ss_CommitCount = 0;
	m_c_CChg.VjeCNVCancel();
}

/**
 * @brief 変換候補を確定する.
 *
 */
void SKBC_JapaneseInputEngine::commitConvertingText()
{
	if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
		// 変換中は無条件でレイヤ2の文字列を確定する.
		QString c_CommitString = m_c_ComposingText.getString( SKBC_ComposingText::SKBE_LAYER2 );
		// 文字列を確定したことにより内部メンバを初期化し、すべての要素を更新有りとする.
		initialize();
		m_c_CommitString = c_CommitString;
		m_bl_CommitStringChanged = true;
		m_bl_PreeditStringChanged = true;
		m_bl_CandidateListChanged = true;
	}
}

/**
 * @brief 最後の文字がアルファベットで終わる場合 true を返す.
 *
 * @param c_Str 検査対象の文字列をあらわす.
 * @return bool 最後の文字がアルファベットで終わる場合 true を返す.
 */
bool SKBC_JapaneseInputEngine::isAlphabetLastChar(const QString &c_Str)
{
	bool bl_Alphabet = false;
	if ( true != c_Str.isEmpty() ) {
		ushort us_Char = c_Str.at( c_Str.length() - 1 ).unicode();
		if ( ( 'A' <= us_Char && 'Z' >= us_Char ) || ( 'a' <= us_Char && 'z' >= us_Char ) ) {
			bl_Alphabet = true;
		}
	}
	return ( bl_Alphabet );
}

/**
 * @brief 対象レイヤの文字列をコミットする.
 *
 */
void SKBC_JapaneseInputEngine::commitTextWithoutLastAlphabet()
{
	QString c_StrLast = m_c_ComposingText.getStrSegment( m_e_TargetLayer, -1 ).getString();

	if ( isAlphabetLastChar( c_StrLast ) ) {
		m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, -1 );
		commitText();
		m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, 1 );
	} else {
		commitText();
	}
}

/**
 * @brief 対象レイヤの文字列をコミットする.
 * @return bool 対象レイヤのコミットが完了した場合 true を返す.
 */
bool SKBC_JapaneseInputEngine::commitText()
{
	bool bl_Commit = false;
	short ss_CursorPos = m_c_ComposingText.getCursor( m_e_TargetLayer );
	if ( 0 < ss_CursorPos ) {
		QString c_StrCommit = m_c_ComposingText.getString( m_e_TargetLayer, 0, ss_CursorPos - 1 );
		bl_Commit = commitText( c_StrCommit );
	}
	return ( bl_Commit );
}

/**
 * @brief 対象レイヤの文字列をコミットする.
 * @param c_Str コミット対象の文字列をあらわす.
 * @return bool 対象レイヤのコミットが完了した場合 true を返す.
 */
bool SKBC_JapaneseInputEngine::commitText(const QString &c_Str)
{
	SKBC_ComposingText::SKBE_TextLayer e_Layer = m_e_TargetLayer;

	m_c_CommitString = c_Str;
	m_bl_CommitStringChanged = true;

	short ss_CursorPos = m_c_ComposingText.getCursor( e_Layer );
	if ( 0 < ss_CursorPos ) {
		m_c_ComposingText.deleteStrSegment( e_Layer, 0, m_c_ComposingText.getCursor( e_Layer ) - 1 );
		m_c_ComposingText.setCursor( e_Layer, m_c_ComposingText.size( e_Layer ) );
	}

	m_ss_CommitCount++;

	// 未確定文字列, 変換候補 の設定.
	if ( SKBC_ComposingText::SKBE_LAYER2 == e_Layer && 0 == m_c_ComposingText.size( e_Layer ) ) {
		// すべての文字が確定した場合は、ターゲットレイヤをLayer1(ひらがな)に戻す.
		e_Layer = SKBC_ComposingText::SKBE_LAYER1;
		m_sl_CandidateItemIndex = -1;
		m_ss_CommitCount = 0;
	}
	if ( SKBC_ComposingText::SKBE_LAYER2 == e_Layer ) {
		m_ss_CommitCount = 0;
		updateContext( e_Layer, true, true );
	} else {
		m_ss_CommitCount = 0;
		updateContext( SKBC_ComposingText::SKBE_LAYER1, true, true );
	}

	bool bl_Commit = false;
	if ( 0 < m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER0 ) ) {
		bl_Commit = true;
	}
	return ( bl_Commit );
}

/**
 * @brief コンテキストを更新する.
 *
 * @param e_Layer 更新対象のレイヤを指定する.
 * @param bl_updateCandidates 予測候補を更新する場合 true を指定する.それ以外の場合 false を指定する.
 * @param updateText 未確定文字列を更新する場合 true を指定する.それ以外の場合 false を指定する.
 */
void SKBC_JapaneseInputEngine::updateContext(SKBC_ComposingText::SKBE_TextLayer e_Layer, bool bl_updateCandidates, bool updateText)
{
	m_e_TargetLayer = e_Layer;

	// 変換候補リストを更新する.
	if ( true == bl_updateCandidates ) {
		updateCandidate();
	}

	m_c_PreeditString.clear();
	m_c_PreeditString.insert( 0, m_c_ComposingText.getString( e_Layer ) );

	// 未確定文字列とカーソル位置を更新する.
	if ( true == updateText || true != m_c_PreeditString.isEmpty() ) {
		short ss_CursorIndex = m_c_ComposingText.getCursor( e_Layer );
		m_ss_PreeditCursorPos = static_cast< short >( m_c_ComposingText.getString( e_Layer, 0, ss_CursorIndex - 1 ).length() );
		m_bl_PreeditStringChanged = true;
	}
}

/**
 * @brief 変換候補リストを更新する.
 *
 */
void SKBC_JapaneseInputEngine::updateCandidate()
{
	if ( 0 == m_ss_CommitCount ) {
		convert();
	} else {
		// no op.
	}
	if ( 0 < m_ss_CandidateItemCount ) {
		// 変換後に候補が1つでもある場合は、レイヤ2の先頭要素後端をカーソル位置とする.
		m_c_ComposingText.setCursor( SKBC_ComposingText::SKBE_LAYER2, 1 );
	} else {
		// no op.
	}
	switch ( m_e_TargetLayer ) {
	case SKBC_ComposingText::SKBE_LAYER0:
	case SKBC_ComposingText::SKBE_LAYER1:
		// no op.
		break;
	case SKBC_ComposingText::SKBE_LAYER2:
		if ( 0 < m_ss_CandidateItemCount ) {
			// 変換中の場合は、変換候補を更新する都度、選択位置を先頭に移動する.
			updateL2Segment( focusCandidate( 0 ) );		// AR1032681対応.
		} else {
			// Compact-VJEを利用する場合、必ず変換候補が存在するため処理しない.
			// no op.
		}
		break;
	default:
		break;
	}
}

/**
 * @brief CChg を利用して変換する.
 *
 * @return int32_t 変換候補数を返す.
 */
int32_t SKBC_JapaneseInputEngine::convert()
{
	short ss_CursorPos = m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER1 );
	QString c_StrTarget;
	QString c_StrRest;

	m_c_CandidateList.clear();
	m_ss_CandidateItemCount = 0;
	m_c_CChg.VjeCNVCancel();
	m_bl_CandidateListChanged = true;

	if ( true != m_bl_enableL2Converter ) {
		// 変換を必要としていない入力モード(カタカナ)の場合は、変換候補のクリアのみを実施し、この関数を抜ける.
		return ( 0 );
	}

	if ( 0 < ss_CursorPos ) {
		// カーソルより前の文字列が変換対象とする.
		c_StrTarget = m_c_ComposingText.getString( SKBC_ComposingText::SKBE_LAYER1, 0, ss_CursorPos - 1 );
		m_c_CandidateHiraganaItem = c_StrTarget;
		// AR1032681対応 start.
		if ( "n" == m_c_CandidateHiraganaItem.right(1) ) {
			// 最後の1文字が"n"の場合、"ん”に変換する.
			m_c_CandidateHiraganaItem.replace( m_c_CandidateHiraganaItem.length() - 1, 1, "\u3093" );
		}
		// AR1032681対応 end.
		// カーソルより後ろの文字列は変換文字の次の構成要素とするため保持する.
		c_StrRest = m_c_ComposingText.getString( SKBC_ComposingText::SKBE_LAYER1, ss_CursorPos, m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) - 1);
		m_c_CandidateStrokeItem = m_c_ComposingText.getString( SKBC_ComposingText::SKBE_LAYER0, 0, m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER0 ) - 1 );
	}


	if ( true != c_StrTarget.isEmpty() ) {
		QTextCodec* pc_Codec = QTextCodec::codecForName( "shift-jis" );
		int32_t i = 0;
		if ( NULL != pc_Codec ) {
			for ( i = 0; c_StrTarget.length() > i; ++i ) {
				m_c_CChg.SetPreString( pc_Codec->fromUnicode( c_StrTarget.mid( i, 1 ) ).data() );
			}
			m_c_CChg.VjeRomeConv();
			bool bl_ReadFinished = false;
			while ( true != bl_ReadFinished ) {
				::memset( m_sc_CandidateBuffer, 0, sizeof( m_sc_CandidateBuffer ) );
				m_c_CChg.GetKouho( m_sc_CandidateBuffer );
				for ( i = 0 ; SKBD_CCHG_JP_MAX_KOUHO > i; ++i ) {
					if ( '\0' == m_sc_CandidateBuffer[i][0] ) {
						bl_ReadFinished = true;
						break;
					}
					m_c_CandidateList.append( QByteArray( m_sc_CandidateBuffer[i] ) );
					++m_ss_CandidateItemCount;
					if ( m_c_CChg.VjeData.cLBItemTotal <= m_ss_CandidateItemCount ) {
						bl_ReadFinished = true;
					}
				}
				m_c_CChg.VjeCNVNextBrock();
			}

			// Layer2の構成要素を更新する.
			QList< SKBC_StrSegment > c_StrSegment;
			if ( m_ss_CandidateItemCount > 0 ) {
				// 矢印キーにより、未確定文字列のカーソル位置が後端でなくなった場合.
				// カーソル前半の文字構成要素.
				short ss_Pos = 0;
				c_StrSegment.append(
							SKBC_StrSegment(
								m_c_CandidateStrokeItem
								, ss_Pos
								, static_cast< short >( ss_Pos + c_StrTarget.length() - 1 )
								) );
				ss_Pos += c_StrTarget.length();
				// カーソル後半の文字構成要素.
				if ( true != c_StrRest.isEmpty() ) {
					c_StrSegment.append(
								SKBC_StrSegment(
									c_StrRest
									, ss_Pos
									, static_cast< short >( ss_Pos + c_StrRest.length() - 1 )
									) );
				}
			}

			m_c_ComposingText.setCursor( SKBC_ComposingText::SKBE_LAYER2, m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER2 ) );
			m_c_ComposingText.replaceStrSegment( SKBC_ComposingText::SKBE_LAYER2, c_StrSegment, m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER2 ) );
		}
	}

	return ( m_ss_CandidateItemCount );
}

/**
 * @brief レイヤ2の更新を開始する.
 * @param bl_Space スペースにて変換を開始した場合 true を指定する.
 * @return QString 選択された変換候補文字列を返す.
 */
QString SKBC_JapaneseInputEngine::startConvert( bool bl_Space /* = true */ )
{
	if ( true != m_bl_enableL2Converter ) {
		return QString();
	}
	updateContext( SKBC_ComposingText::SKBE_LAYER2, false, true );
	int32_t sl_SelectedIndex = m_sl_CandidateItemIndex;
	if ( true == bl_Space ) {
		++sl_SelectedIndex;
	}
	return ( focusCandidate( sl_SelectedIndex ) );
}

/**
 * @brief 変換候補リストを選択する.
 *
 * @param sl_Index 選択対象のインデクスを指定する.
 * @return QString 選択された変換候補文字列を返す.
 */
QString SKBC_JapaneseInputEngine::focusCandidate( int sl_Index )
{
	QString c_StrTarget;
	int32_t sl_CandidateListItemCount= getCandidateListItemCount();

	if ( 0 == m_ss_CandidateItemCount ) {
		return ( c_StrTarget );
	}
	m_sl_CandidateItemIndex = sl_Index;
	if ( m_sl_CandidateItemIndex >= sl_CandidateListItemCount ) {
		m_sl_CandidateItemIndex = 0;
	}
	QTextCodec* pc_Codec = QTextCodec::codecForName( "shift-jis" );
	if ( NULL != pc_Codec ) {
		if ( SKBE_CUSTOM_CANDIDATE_HIRAGANA == m_sl_CandidateItemIndex ) {
			c_StrTarget = m_c_CandidateHiraganaItem;
		} else if ( SKBE_CUSTOM_CANDIDATE_FULL_KATAKANA == m_sl_CandidateItemIndex ) {
			c_StrTarget = str2FullKatakana( m_c_CandidateHiraganaItem );
		} else if ( SKBE_CUSTOM_CANDIDATE_STROKE == m_sl_CandidateItemIndex ) {
			c_StrTarget = m_c_CandidateStrokeItem;
		} else if ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_LOWER ) == m_sl_CandidateItemIndex ) {
			c_StrTarget = m_c_CandidateStrokeItem.toLower();
		} else if ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_UPPER ) == m_sl_CandidateItemIndex ) {
			c_StrTarget = m_c_CandidateStrokeItem.toUpper();
		} else if  ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_HALF_KATAKANA )  == m_sl_CandidateItemIndex ) {
			c_StrTarget = str2HalfKatakana( m_c_CandidateHiraganaItem ).toUtf8();
		} else if  ( (sl_CandidateListItemCount - SKBE_CUSTOM_CANDIDATE_FULL )  == m_sl_CandidateItemIndex ) {
			c_StrTarget = str2FullLatin( m_c_CandidateStrokeItem ).toUtf8();
		} else {
			c_StrTarget = pc_Codec->toUnicode( m_c_CandidateList.at( m_sl_CandidateItemIndex - SKBE_CUSTOM_CANDIDATE_FRONT_MAX ) );
		}
	}

	return ( c_StrTarget );
}

/**
 * @brief レイヤ2の要素を更新する.
 * 変換対象の文字列のみ更新する.
 *
 * @param c_StrTarget 更新対象の文字列をあらわす.
 */
void SKBC_JapaneseInputEngine::updateL2Segment(const QString &c_StrTarget)
{
	// Layer2の構成要素を更新する.
	QList< SKBC_StrSegment > c_StrSegment;
	c_StrSegment.append( m_c_ComposingText.getStrSegment( SKBC_ComposingText::SKBE_LAYER2, 0 ) );
	c_StrSegment[ 0 ].setString( c_StrTarget );
	m_c_ComposingText.replaceStrSegment( SKBC_ComposingText::SKBE_LAYER2, c_StrSegment, 1 );
	updateContext( SKBC_ComposingText::SKBE_LAYER2, false, false );
}

/**
 * @brief 左矢印キー押下によりカーソルを移動する.
 *
 * @return bool キー操作を受け付けた場合 true を返す.
 */
bool SKBC_JapaneseInputEngine::processLeftKey()
{
	if ( 0 == m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) ) {
		return ( false );
	}

	m_bl_KeyAccepted = false;
	if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
		if ( 1 < m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER1 ) ) {
			m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, -1 );
			m_bl_KeyAccepted = true;
		}
	} else {
		m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, -1 );
		m_bl_KeyAccepted = true;
	}

	// 変換対象が変わったため、再度変換をかける.
	m_ss_CommitCount = 0;
	updateContext( m_e_TargetLayer, true, true );

	if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
		updateL2Segment( focusCandidate( m_sl_CandidateItemIndex ) );
	}

	return ( true );
}

/**
 * @brief 右矢印キー押下によりカーソルを移動する.
 *
 * @return bool キー操作を受け付けた場合 true を返す.
 */
bool SKBC_JapaneseInputEngine::processRightKey()
{
	if ( 0 == m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 ) ) {
		return ( false );
	}

	int32_t sl_TextSize = m_c_ComposingText.size( SKBC_ComposingText::SKBE_LAYER1 );
	if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
		if ( sl_TextSize == m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER1 ) ) {
			m_e_TargetLayer = SKBC_ComposingText::SKBE_LAYER1;
		} else {
			m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, 1 );
		}
		m_bl_KeyAccepted = true;
	} else {
		if ( sl_TextSize > m_c_ComposingText.getCursor( SKBC_ComposingText::SKBE_LAYER1 ) ) {
			m_c_ComposingText.moveCursor( SKBC_ComposingText::SKBE_LAYER1, 1 );
			m_bl_KeyAccepted = true;
		}
	}

	// 変換対象が変わったため、再度変換をかける.
	m_ss_CommitCount = 0;
	updateContext( m_e_TargetLayer, true, true );

	if ( SKBC_ComposingText::SKBE_LAYER2 == m_e_TargetLayer ) {
		updateL2Segment( focusCandidate( m_sl_CandidateItemIndex ) );
	}

	return ( true );
}

/**
 * @brief ひらがな から 半角カタカナ に変換する.
 *
 * @param c_String 変換対象のひらがなをあらわす.
 * @return QString 半角カタカナをあらわす.
 */
QString SKBC_JapaneseInputEngine::str2HalfKatakana(const QString &c_String)
{
	bool bl_Result = false;
	QString c_StrTmp = transliterate( QStringLiteral("Hiragana-Katakana"), c_String, &bl_Result );
	QString c_StrResult = transliterate( QStringLiteral("Fullwidth-Halfwidth"), c_StrTmp, &bl_Result );
	return ( c_StrResult );
}

/**
 * @brief ひらがな から 全角カタカナ に変換する.
 *
 * @param c_String 変換対象のひらがなをあらわす.
 * @return QString 半角カタカナをあらわす.
 */
QString SKBC_JapaneseInputEngine::str2FullKatakana(const QString &c_String)
{
	bool bl_Result = false;
	QString c_StrResult = transliterate( QStringLiteral("Hiragana-Katakana"), c_String, &bl_Result );
	return ( c_StrResult );
}

/**
 * @brief アルファベット から 全角アルファベット に変換する.
 *
 * @param c_String 変換対象のアルファベットをあらわす.
 * @return QString 全角アルファベットをあらわす.
 */
QString SKBC_JapaneseInputEngine::str2FullLatin(const QString &c_String)
{
	bool bl_Result = false;
	QString c_StrResult = transliterate( QStringLiteral("Halfwidth-Fullwidth"), c_String, &bl_Result );
	return ( c_StrResult );
}

/**
 * @brief ICU(International Component for Unicode)を利用して文字列を変換する.
 * 変換は以下のIDを利用する.
 *   ひらがな から 半角カタカナ: "Hiragana-Katakana", "Fullwidth-Halfwidth".
 *   ひらがな から 全角カタカナ: "Hiragana-Katakana".
 *   アルファベット から 全角アルファベット: "Halfwidth-Fullwidth".
 * *) http://userguide.icu-project.org/transforms/general
 *
 * @param c_ID 変換テーブルIDをあらわす.
 * @param c_String 変換対象の文字列をあらわす.
 * @param pbl_Result 変換結果をあらわす.変換された場合 true が設定される.それ以外の場合 false が設定される.
 * @return QString 変換後の文字列をあらわす.
 */
QString SKBC_JapaneseInputEngine::transliterate(const QString &c_ID, const QString &c_String, bool* pbl_Result)
{
	UErrorCode c_Status = U_ZERO_ERROR;
	QString c_RetStr = c_String;

	UnicodeString c_IDStr = UnicodeString::fromUTF8( c_ID.toUtf8().constData() );
	Transliterator* pc_Transliterator = Transliterator::createInstance( c_IDStr, UTRANS_FORWARD, c_Status );

	if ( U_FAILURE( c_Status ) ) {
		if ( NULL != pbl_Result ) {
			*pbl_Result = false;
		} else {
			// no op.
		}
	} else {
		UnicodeString c_TargetStr = UnicodeString::fromUTF8( c_String.toUtf8().constData() );
		if ( NULL != pc_Transliterator ) {
			pc_Transliterator->transliterate( c_TargetStr );
			int32_t sl_TextLen = c_TargetStr.length();
			c_RetStr = QString( sl_TextLen, Qt::Uninitialized );
			c_TargetStr.extract( 0, sl_TextLen, reinterpret_cast< UChar* >( c_RetStr.data() ) );
			if ( NULL != pbl_Result ) {
				*pbl_Result = true;
			} else {
				// no op.
			}
		} else {
			if ( NULL != pbl_Result ) {
				*pbl_Result = false;
			} else {
				// no op.
			}
		}
	}

	delete ( pc_Transliterator );
	return ( c_RetStr );
}




