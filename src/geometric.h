#ifndef	__INC_GEOMETRIC_H__
#define	__INC_GEOMETRIC_H__

#include	"types.h"

#ifndef	Coordinate
typedef	struct	{
	int		x;
	int		y;
}	_Coordinate;
#define	Coordinate	_Coordinate
#endif

extern	Bool	CheckPointToOrigine(Coordinate p1, Coordinate p2, int x, int y);

#endif
