/******************************************************************************

	m68000.c

	M68000 CPU���󥿥ե������ؿ�

******************************************************************************/

#include "m68000.h"
#include "c68k/c68k.h"
#include "../x68k/memory.h"

int m68000_ICountBk;
int ICount;

/******************************************************************************
	CPS2�Ź沽ROM��
******************************************************************************/

#if (EMU_SYSTEM == CPS2)
static UINT32 m68k_encrypt_start;
static UINT32 m68k_encrypt_end;
static UINT8  *m68k_decrypted_rom;

/*--------------------------------------------------------
	�Ź沽ROM�ϰ�����
--------------------------------------------------------*/

void m68000_set_encrypted_range(UINT32 start, UINT32 end, void *decrypted_rom)
{
	m68k_encrypt_start = start;
	m68k_encrypt_end   = end;
	m68k_decrypted_rom = (UINT8 *)decrypted_rom;
}


/*--------------------------------------------------------
	PC��¸����꡼�� (byte)
--------------------------------------------------------*/

static UINT8 m68000_read_pcrelative_8(UINT32 offset)
{
	if (offset >= m68k_encrypt_start && offset <= m68k_encrypt_end)
		return m68k_decrypted_rom[offset ^ 1];
	else
		return Memory_ReadB(offset);
}


/*--------------------------------------------------------
	PC��¸����꡼�� (word)
--------------------------------------------------------*/

static UINT16 m68000_read_pcrelative_16(UINT32 offset)
{
	if (offset >= m68k_encrypt_start && offset <= m68k_encrypt_end)
		return *(UINT16 *)&m68k_decrypted_rom[offset];
	else
		return Memory_ReadW(offset);
}
#endif


/******************************************************************************
	M68000���󥿥ե������ؿ�
******************************************************************************/

/*--------------------------------------------------------
	CPU�����
--------------------------------------------------------*/
s32 my_irqh_callback(s32 level);

void m68000_init(void)
{
    C68k_Init(&C68K, my_irqh_callback);
#if 0
    C68k_Set_ReadB(&C68K, Memory_ReadB);
    C68k_Set_ReadW(&C68K, Memory_ReadW);
    C68k_Set_WriteB(&C68K, Memory_WriteB);
    C68k_Set_WriteW(&C68K, Memory_WriteW);
#else
    C68k_Set_ReadB(&C68K, (C68K_READ*)Memory_ReadB);
    C68k_Set_ReadW(&C68K, (C68K_READ*)Memory_ReadW);
    C68k_Set_WriteB(&C68K, (C68K_WRITE*)Memory_WriteB);
    C68k_Set_WriteW(&C68K, (C68K_WRITE*)Memory_WriteW);
#endif
        C68k_Set_Fetch(&C68K, 0x000000, 0xbfffff, (pointer)MEM);
        C68k_Set_Fetch(&C68K, 0xc00000, 0xc7ffff, (pointer)GVRAM);
        C68k_Set_Fetch(&C68K, 0xe00000, 0xe7ffff, (pointer)TVRAM);
        C68k_Set_Fetch(&C68K, 0xea0000, 0xea1fff, (pointer)SCSIIPL);
        C68k_Set_Fetch(&C68K, 0xed0000, 0xed3fff, (pointer)SRAM);
        C68k_Set_Fetch(&C68K, 0xf00000, 0xfbffff, (pointer)FONT);
        C68k_Set_Fetch(&C68K, 0xfc0000, 0xffffff, (pointer)IPL);
}


/*--------------------------------------------------------
	CPU�ꥻ�å�
--------------------------------------------------------*/

void m68000_reset(void)
{
	C68k_Reset(&C68K);
}


/*--------------------------------------------------------
	CPU���
--------------------------------------------------------*/

void m68000_exit(void)
{
}


/*--------------------------------------------------------
	CPU�¹�
--------------------------------------------------------*/

int m68000_execute(int cycles)
{
	return C68k_Exec(&C68K, cycles);
}


/*--------------------------------------------------------
	CPU�¹� (NEOGEO CDZ����: ���ɲ�����)
--------------------------------------------------------*/

#if (EMU_SYSTEM == NCDZ)
void m68000_execute2(UINT32 start, UINT32 break_point)
{
	int nest_counter = 0;
	UINT32 pc, old_pc, opcode;
	c68k_struc C68K_temp;

	old_pc = C68k_Get_PC(&C68K);

	memcpy(&C68K_temp, &C68K, sizeof(c68k_struc));

//	C68k_Set_Reg(&C68K_temp, C68K_PC, start);
	C68k_Set_PC(&C68K_temp, start);
	C68K_temp.A[5] = 0x108000;
	C68K_temp.A[7] -= 4 * 8 * 2;

//	while ((pc = C68k_Get_Reg(&C68K_temp, M68K_PC)) != break_point)
	while ((pc = C68k_Get_PC(&C68K_temp)) != break_point)
	{
		opcode = Memory_ReadW(pc);
		if (opcode == 0x4e75)
		{
			// rts
			nest_counter--;
			if (nest_counter < 0) break;
		}
		else if (opcode == 0x6100)
		{
			// bsr 16
			nest_counter++;
		}
		else if ((opcode & 0xff00) == 0x6100)
		{
			// bsr 8
			nest_counter++;
		}
		else if ((opcode & 0xffc0) == 0x4e80)
		{
			// jsr
			nest_counter++;
		}

		C68k_Exec(&C68K_temp, 1);
	}

//	C68k_Set_Reg(&C68K, C68K_PC, old_pc);
	C68k_Set_PC(&C68K, old_pc);
}
#endif


/*--------------------------------------------------------
	�����߽���
--------------------------------------------------------*/

void m68000_set_irq_line(int irqline, int state)
{
//	if (irqline == IRQ_LINE_NMI)
//		irqline = 7;

//	C68k_Set_IRQ(&C68K, irqline, state);
	C68k_Set_IRQ(&C68K, irqline);
}


/*--------------------------------------------------------
	�����ߥ�����Хå��ؿ�����
--------------------------------------------------------*/

void m68000_set_irq_callback(int (*callback)(int line))
{
//	C68k_Set_IRQ_Callback(&C68K, callback);
}


/*--------------------------------------------------------
	�쥸��������
--------------------------------------------------------*/

UINT32 m68000_get_reg(int regnum)
{
	switch (regnum)
	{
#if 0
	case M68K_PC:  return C68k_Get_Reg(&C68K, C68K_PC);
	case M68K_USP: return C68k_Get_Reg(&C68K, C68K_USP);
	case M68K_MSP: return C68k_Get_Reg(&C68K, C68K_MSP);
	case M68K_SR:  return C68k_Get_Reg(&C68K, C68K_SR);
	case M68K_D0:  return C68k_Get_Reg(&C68K, C68K_D0);
	case M68K_D1:  return C68k_Get_Reg(&C68K, C68K_D1);
	case M68K_D2:  return C68k_Get_Reg(&C68K, C68K_D2);
	case M68K_D3:  return C68k_Get_Reg(&C68K, C68K_D3);
	case M68K_D4:  return C68k_Get_Reg(&C68K, C68K_D4);
	case M68K_D5:  return C68k_Get_Reg(&C68K, C68K_D5);
	case M68K_D6:  return C68k_Get_Reg(&C68K, C68K_D6);
	case M68K_D7:  return C68k_Get_Reg(&C68K, C68K_D7);
	case M68K_A0:  return C68k_Get_Reg(&C68K, C68K_A0);
	case M68K_A1:  return C68k_Get_Reg(&C68K, C68K_A1);
	case M68K_A2:  return C68k_Get_Reg(&C68K, C68K_A2);
	case M68K_A3:  return C68k_Get_Reg(&C68K, C68K_A3);
	case M68K_A4:  return C68k_Get_Reg(&C68K, C68K_A4);
	case M68K_A5:  return C68k_Get_Reg(&C68K, C68K_A5);
	case M68K_A6:  return C68k_Get_Reg(&C68K, C68K_A6);
	case M68K_A7:  return C68k_Get_Reg(&C68K, C68K_A7);
#else
	case M68K_PC:  return C68k_Get_PC(&C68K);
	case M68K_USP: return C68k_Get_USP(&C68K);
	case M68K_MSP: return C68k_Get_MSP(&C68K);
	case M68K_SR:  return C68k_Get_SR(&C68K);
	case M68K_D0:  return C68k_Get_DReg(&C68K, 0);
	case M68K_D1:  return C68k_Get_DReg(&C68K, 1);
	case M68K_D2:  return C68k_Get_DReg(&C68K, 2);
	case M68K_D3:  return C68k_Get_DReg(&C68K, 3);
	case M68K_D4:  return C68k_Get_DReg(&C68K, 4);
	case M68K_D5:  return C68k_Get_DReg(&C68K, 5);
	case M68K_D6:  return C68k_Get_DReg(&C68K, 6);
	case M68K_D7:  return C68k_Get_DReg(&C68K, 7);
	case M68K_A0:  return C68k_Get_AReg(&C68K, 0);
	case M68K_A1:  return C68k_Get_AReg(&C68K, 1);
	case M68K_A2:  return C68k_Get_AReg(&C68K, 2);
	case M68K_A3:  return C68k_Get_AReg(&C68K, 3);
	case M68K_A4:  return C68k_Get_AReg(&C68K, 4);
	case M68K_A5:  return C68k_Get_AReg(&C68K, 5);
	case M68K_A6:  return C68k_Get_AReg(&C68K, 6);
	case M68K_A7:  return C68k_Get_AReg(&C68K, 7);
#endif

	default: return 0;
	}
}


/*--------------------------------------------------------
	�쥸��������
--------------------------------------------------------*/

void m68000_set_reg(int regnum, UINT32 val)
{
	switch (regnum)
	{
#if 0
	case M68K_PC:  C68k_Set_Reg(&C68K, C68K_PC, val); break;
	case M68K_USP: C68k_Set_Reg(&C68K, C68K_USP, val); break;
	case M68K_MSP: C68k_Set_Reg(&C68K, C68K_MSP, val); break;
	case M68K_SR:  C68k_Set_Reg(&C68K, C68K_SR, val); break;
	case M68K_D0:  C68k_Set_Reg(&C68K, C68K_D0, val); break;
	case M68K_D1:  C68k_Set_Reg(&C68K, C68K_D1, val); break;
	case M68K_D2:  C68k_Set_Reg(&C68K, C68K_D2, val); break;
	case M68K_D3:  C68k_Set_Reg(&C68K, C68K_D3, val); break;
	case M68K_D4:  C68k_Set_Reg(&C68K, C68K_D4, val); break;
	case M68K_D5:  C68k_Set_Reg(&C68K, C68K_D5, val); break;
	case M68K_D6:  C68k_Set_Reg(&C68K, C68K_D6, val); break;
	case M68K_D7:  C68k_Set_Reg(&C68K, C68K_D7, val); break;
	case M68K_A0:  C68k_Set_Reg(&C68K, C68K_A0, val); break;
	case M68K_A1:  C68k_Set_Reg(&C68K, C68K_A1, val); break;
	case M68K_A2:  C68k_Set_Reg(&C68K, C68K_A2, val); break;
	case M68K_A3:  C68k_Set_Reg(&C68K, C68K_A3, val); break;
	case M68K_A4:  C68k_Set_Reg(&C68K, C68K_A4, val); break;
	case M68K_A5:  C68k_Set_Reg(&C68K, C68K_A5, val); break;
	case M68K_A6:  C68k_Set_Reg(&C68K, C68K_A6, val); break;
	case M68K_A7:  C68k_Set_Reg(&C68K, C68K_A7, val); break;
#else
	case M68K_PC:  C68k_Set_PC(&C68K, val); break;
	case M68K_USP: C68k_Set_USP(&C68K, val); break;
	case M68K_MSP: C68k_Set_MSP(&C68K, val); break;
	case M68K_SR:  C68k_Set_SR(&C68K, val); break;
	case M68K_D0:  C68k_Set_DReg(&C68K, 0, val); break;
	case M68K_D1:  C68k_Set_DReg(&C68K, 1, val); break;
	case M68K_D2:  C68k_Set_DReg(&C68K, 2, val); break;
	case M68K_D3:  C68k_Set_DReg(&C68K, 3, val); break;
	case M68K_D4:  C68k_Set_DReg(&C68K, 4, val); break;
	case M68K_D5:  C68k_Set_DReg(&C68K, 5, val); break;
	case M68K_D6:  C68k_Set_DReg(&C68K, 6, val); break;
	case M68K_D7:  C68k_Set_DReg(&C68K, 7, val); break;
	case M68K_A0:  C68k_Set_AReg(&C68K, 0, val); break;
	case M68K_A1:  C68k_Set_AReg(&C68K, 1, val); break;
	case M68K_A2:  C68k_Set_AReg(&C68K, 2, val); break;
	case M68K_A3:  C68k_Set_AReg(&C68K, 3, val); break;
	case M68K_A4:  C68k_Set_AReg(&C68K, 4, val); break;
	case M68K_A5:  C68k_Set_AReg(&C68K, 5, val); break;
	case M68K_A6:  C68k_Set_AReg(&C68K, 6, val); break;
	case M68K_A7:  C68k_Set_AReg(&C68K, 7, val); break;
#endif
	default: break;
	}
}


/*------------------------------------------------------
	������/���� ���ơ���
------------------------------------------------------*/

#ifdef SAVE_STATE

STATE_SAVE( m68000 )
{
	int i;
	UINT32 pc = C68k_Get_Reg(&C68K, C68K_PC);

	for (i = 0; i < 8; i++)
		state_save_long(&C68K.D[i], 1);
	for (i = 0; i < 8; i++)
		state_save_long(&C68K.A[i], 1);

	state_save_long(&C68K.flag_C, 1);
	state_save_long(&C68K.flag_V, 1);
	state_save_long(&C68K.flag_Z, 1);
	state_save_long(&C68K.flag_N, 1);
	state_save_long(&C68K.flag_X, 1);
	state_save_long(&C68K.flag_I, 1);
	state_save_long(&C68K.flag_S, 1);
	state_save_long(&C68K.USP, 1);
	state_save_long(&pc, 1);
	state_save_long(&C68K.HaltState, 1);
	state_save_long(&C68K.IRQLine, 1);
	state_save_long(&C68K.IRQState, 1);
}

STATE_LOAD( m68000 )
{
	int i;
	UINT32 pc;

	for (i = 0; i < 8; i++)
		state_load_long(&C68K.D[i], 1);
	for (i = 0; i < 8; i++)
		state_load_long(&C68K.A[i], 1);

	state_load_long(&C68K.flag_C, 1);
	state_load_long(&C68K.flag_V, 1);
	state_load_long(&C68K.flag_Z, 1);
	state_load_long(&C68K.flag_N, 1);
	state_load_long(&C68K.flag_X, 1);
	state_load_long(&C68K.flag_I, 1);
	state_load_long(&C68K.flag_S, 1);
	state_load_long(&C68K.USP, 1);
	state_load_long(&pc, 1);
	state_load_long(&C68K.HaltState, 1);
	state_load_long(&C68K.IRQLine, 1);
	state_load_long(&C68K.IRQState, 1);

	C68k_Set_Reg(&C68K, C68K_PC, pc);
}

#endif /* SAVE_STATE */
