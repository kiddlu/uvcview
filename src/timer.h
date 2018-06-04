#ifndef	__INC_TIMER_H__
#define	__INC_TIMER_H__

#ifndef	TimeFiller
typedef	struct {
	int64_t	next;
}	_TimeFiller;
#define	TimeFiller	_TimeFiller

extern	TimeFiller	*InitTimeFiller(void);
extern	void		SleepTimeFiller(TimeFiller *micro, int ms);
extern	void		FreeTimeFiller(TimeFiller *micro);
#endif

#endif
