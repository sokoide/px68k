#ifndef winx1_timer_h
#define winx1_timer_h

void Timer_Init(void);
void Timer_Reset(void);
WORD Timer_GetCount(void);
void Timer_SetCount(WORD);
void Timer_WaitCount(void);

#endif //winx1_timer_h
