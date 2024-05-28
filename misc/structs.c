
struct _win_st
{
	NCURSES_SIZE_T _cury, _curx; /* current cursor position */

	/* window location and size */
	NCURSES_SIZE_T _maxy, _maxx; /* maximums of x and y, NOT window size */
	NCURSES_SIZE_T _begy, _begx; /* screen coords of upper-left-hand corner */

	short   _flags;		/* window state flags */

	/* attribute tracking */
	attr_t  _attrs;		/* current attribute for non-space character */
	chtype  _bkgd;		/* current background char/attribute pair */

	/* option values set by user */
	bool	_notimeout;	/* no time out on function-key entry? */
	bool	_clear;		/* consider all data in the window invalid? */
	bool	_leaveok;	/* OK to not reset cursor on exit? */
	bool	_scroll;	/* OK to scroll this window? */
	bool	_idlok;		/* OK to use insert/delete line? */
	bool	_idcok;		/* OK to use insert/delete char? */
	bool	_immed;		/* window in immed mode? (not yet used) */
	bool	_sync;		/* window in sync mode? */
	bool	_use_keypad;	/* process function keys into KEY_ symbols? */
	int	_delay;		/* 0 = nodelay, <0 = blocking, >0 = delay */

	struct ldat *_line;	/* the actual line data */

	/* global screen state */
	NCURSES_SIZE_T _regtop;	/* top line of scrolling region */
	NCURSES_SIZE_T _regbottom; /* bottom line of scrolling region */

	/* these are used only if this is a sub-window */
	int	_parx;		/* x coordinate of this window in parent */
	int	_pary;		/* y coordinate of this window in parent */
	WINDOW	*_parent;	/* pointer to parent if a sub-window */

	/* these are used only if this is a pad */
	struct pdat
	{
	    NCURSES_SIZE_T _pad_y,      _pad_x;
	    NCURSES_SIZE_T _pad_top,    _pad_left;
	    NCURSES_SIZE_T _pad_bottom, _pad_right;
	} _pad;

	NCURSES_SIZE_T _yoffset; /* real begy is _begy + _yoffset */

#if NCURSES_WIDECHAR
	cchar_t  _bkgrnd;	/* current background char/attribute pair */
#if 1
	int	_color;		/* current color-pair for non-space character */
#endif
#endif
};
