//------------------------------------------------------------------------------
/**
 *	@file	Local_Com_Line.h.
 *	@brief	ラインパーツ共通部 実装クラス.
 */
//------------------------------------------------------------------------------

#ifndef __LOCAL_COM_LINE_H__
#define __LOCAL_COM_LINE_H__

/*==============================================================================
	インクルード.
===============================================================================*/
#include "Local_ScalableQWidget.h"
#include "Local_ScalableQPainter.h"
#include <QBrush>
#include <QLine>
#include <QRect>
#include <QPen>
#include <QColor>

#include "GFWC_Point.h"
#include "GFWC_Size.h"
#include "GFWC_Com_Line.h"
#include "Local_If_Base.h"

/*==============================================================================
	定義.
===============================================================================*/

/**
 *	@brief	引数で与えられた値のうち小さい方を返すマクロ.
 */
/*#define	min(a,b)	(		\
	{						\
		typeof(a)	_a;		\
		typeof(b)	_b;		\
		_a = a;				\
		_b = b;				\
		_a < _b ? _a : _b;	\
	}						\
)*/
#define	min(a,b)			\
	{						\
		typeof(a)	_a;		\
		typeof(b)	_b;		\
		_a = a;				\
		_b = b;				\
		_a < _b ? _a : _b;	\
	}						\


/**
 *	@brief	引数で与えられた値のうち大きい方を返すマクロ.
 */
#define	max(a,b)	(		\
	{						\
		typeof(a)	_a;		\
		typeof(b)	_b;		\
		_a = a;				\
		_b = b;				\
		_a > _b ? _a : _b;	\
	}						\
)

//------------------------------------------------------------------------------
/**
 *	@brief	ラインパーツ共通部 実装クラス.
 *	@brief		親クラス：QWidget.
 *	@brief		親クラス：Local_If_Base.
 */
//------------------------------------------------------------------------------
class Local_Com_Line : public Local_ScalableQWidget, virtual public Local_If_Base
{
	Q_OBJECT

public:
	/**
	 *	@brief	コンストラクタ.
	 *	@param[in]	pc_ParentWidget	親ウィジェット(QWidget*).
	 */
	Local_Com_Line( QWidget* pc_ParentWidget );

	/**
	 *	@brief	デストラクタ.
	 */
	virtual ~Local_Com_Line( void ) {}

	/**
	 *	@brief	パーツ描画色を設定する.
	 *	@param[in]	e_Color	描画色(GFWE_LINE_COLOR).
	 */
	virtual void setColor( GFWE_LINE_COLOR e_Color );

	/**
	 *	@brief	パーツ描画色を取得する.
	 *	@return	設定されている描画色(GFWE_LINE_COLOR).
	 */
	virtual GFWE_LINE_COLOR getColor( void ) const;

	/**
	 *	@brief	パーツ背景色を設定する.
	 *	@param[in]	e_Color	背景色(GFWE_LINE_COLOR).
	 */
	virtual void setBackgroundColor( GFWE_LINE_COLOR e_Color );

	/**
	 *	@brief	パーツ背景色を取得する.
	 *	@return	設定されている背景色(GFWE_LINE_COLOR).
	 */
	virtual GFWE_LINE_COLOR getBackgroundColor( void ) const;

	/**
	 *	@brief	自インスタンス取得（基底）.
	 *	@return	QObject.
	 */
	virtual QObject* getQObject( void );

	/**
	*	@brief	親パーツを設定する（基底）.
	*/
	virtual void setParentParts( QWidget* parent );

	/**
	*	@brief	最前面に移動する（基底）.
	*/
	virtual void bringToFront( void );

	/**
	*	@brief	最背面に移動する（基底）.
	*/
	virtual void sendToBack( void );

private:
	/**
	 *	@brief	色変換関数(GFWE_LINE_COLOR → ARGB).
	 */
	uint32_t _convertColorValToARGB( GFWE_LINE_COLOR e_Color );

protected:
	/**
	 *	@brief	メンバ変数.
	 */
	GFWE_LINE_COLOR		m_e_Color;				//!< 線描画色.
	QColor				m_q_drawColor;			//!< 線描画色(QColor形式).
	GFWE_LINE_COLOR		m_e_BackgroundColor;	//!< 背景色.
	QColor				m_q_backColor;			//!< 背景色(QColor形式).
	int					m_i_penWidth;			//!< 描画線幅.
};

#endif // __LOCAL_COM_LINE_H__.
