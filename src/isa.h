/***************************************************************/
/*                                                             */
/*   ARMv4-32 Instruction Level Simulator                      */
/*                                                             */
/*                                                             */
/*                                                             */
/*                                                             */
/***************************************************************/

#ifndef _SIM_ISA_H_
#define _SIM_ISA_H_
#define N_CUR ( (CURRENT_STATE.CPSR>>31) & 0x00000001 )
#define Z_CUR ( (CURRENT_STATE.CPSR>>30) & 0x00000001 )
#define C_CUR ( (CURRENT_STATE.CPSR>>29) & 0x00000001 )
#define V_CUR ( (CURRENT_STATE.CPSR>>28) & 0x00000001 )
#define N_NXT ( (NEXT_STATE.CPSR>>31) & 0x00000001 )
#define Z_NXT ( (NEXT_STATE.CPSR>>30) & 0x00000001 )
#define C_NXT ( (NEXT_STATE.CPSR>>29) & 0x00000001 )
#define V_NXT ( (NEXT_STATE.CPSR>>28) & 0x00000001 )
#define GET_BIT(Reg,BitNum) ((Reg)>>(BitNum))&1

#define N_N 0x80000000
#define Z_N 0x40000000
#define C_N 0x20000000
#define V_N 0x10000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

int detect_overflow (int val1 , int val2 , int Carry , int op) {

  int z;

  switch(op) { 
    case 0: 
      z = (val1==val2) & 0x80000000; 
      z = z & ((( val1^z) + val2 + Carry) == val2 ); 
    break; 
    case 1: 
      z = (val1^val2) & 0x80000000; 
      z = z & ((( val1^z) - val2 - Carry) ^ val2 );
    break; 
  }
  return 0;
}
// Detect carry out 
int detect_cout (int val1 , int val2 , int Carry , int op) {

  unsigned int sum;
  switch(op) { 
    case 0: 
      sum = val1 + val2 + Carry; 
      return (sum < val1 ); 
    break; 
    case 1: sum = val1 - val2 - Carry; 
      return (sum < val1 );
      break; 
  }
  return 0;
}

// ADD
int ADD (int Rd, int Rn, int Operand2, int I, int S, int CC) {

  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;

}

/* Arithmetic Shift Right function.
 *
 * asr() takes in 2 inputs:
 * val       - t
 * val       - t
 * shift_amt - the amount to shift val by.
 *
 * Returns the shifted value.
 */

int asr (int val, int shift_amt) {
  int cur = 0;
  int sign = 0;

  //if the sign bit is set to 1, replace it on the shift
  if (((val & (1 << 31)) >> 31) == 1)
   sign = 1;  

  cur = val >> shift_amt; 
  if (sign == 1)
    cur |= ((1 << shift_amt) - 1) << (32 - shift_amt);      
  
  return cur;
}

// ADC Add with carry
int ADC (int Rd, int Rn, int Operand2, int I, int S, int CC) { 
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
    (CURRENT_STATE.REGS[Rm] << shamt5) + C_CUR;
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
    (CURRENT_STATE.REGS[Rm] >> shamt5) + C_CUR;
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + 
    (CURRENT_STATE.REGS[Rm] >> shamt5) + C_CUR;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))) + C_CUR;
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs])  + C_CUR;
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) + C_CUR;
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) + C_CUR;
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))) + C_CUR;
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate))) + C_CUR;
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  } 
  return 0;
}

//MOV
int MOV (int Rd, int Rn, int Operand2,int I,int S,int CC) {
 
   int cur = 0;
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
  if (I == 1) { // Move 
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  else // if I == 0, perform other shift operations
  {
    int signBit = CURRENT_STATE.REGS[Rm] & 0x80000000;
    int shift = CURRENT_STATE.REGS[Rm] >> shamt5;
    int num = shift | signBit;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = (CURRENT_STATE.REGS[Rm] << shamt5); //LSL
      break;
      case 1: cur = (CURRENT_STATE.REGS[Rm] >> shamt5); //LSR
      break;
      case 2: cur = num; // ASR
        break;
      case 3: cur = ((CURRENT_STATE.REGS[Rm] >> shamt5) |
           (CURRENT_STATE.REGS[Rm] << (32 - shamt5))); // ROR
      break;
      }     
    else
      switch (sh) {
      case 0: cur = (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
      break;
      case 1: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
      break;
      case 2: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
      break;
      case 3: cur = ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
           (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
      break;
      }      
  }
  NEXT_STATE.REGS[Rd] = cur;
  return 0;
}

// SUB
int SUB (int Rd, int Rn, int Operand2,int I,int S,int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] >> shamt5);
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] -
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))) ;
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur; 
  return 0;
}

// B Branch
int B (int Imm24) {
  printf("\nImm24: %d\n",Imm24);

  NEXT_STATE.PC = 0xFFFFFF & ( (CURRENT_STATE.PC + 4) + (Imm24 << 2) ); 
  return 0;
}

// BL Branch with link
int BL(int Imm24) {
  NEXT_STATE.REGS[14] = CURRENT_STATE.PC + 8;
  B(Imm24);
  return 0;
}

// CMP Compare
int CMP(int Rd, int Rn, int Operand2, int I, int S, int CC) {
	int cur = 0;
	if(I == 0)
	{
		int shift = Operand2 >> 4;
		int Rm = Operand2 & 0x0000000F;
		cur = CURRENT_STATE.REGS[Rn] - (CURRENT_STATE.REGS[Rm]<<shift);
	}
	if(I == 1)
	{
		int rotate = Operand2 >> 8;
		int Imm = Operand2 & 0x000000FF;
		cur = CURRENT_STATE.REGS[Rn] - (Imm<<rotate);
	}
	
	if(S == 1)
	{
		if(cur < 0)
			NEXT_STATE.CPSR |= N_N;
		if(cur == 0)
			NEXT_STATE.CPSR |= Z_N;
	}
	return 0;
}

// CMN Compare if negative
int CMN(int Rd, int Rn, int Operand2, int I, int S) {
	int cur = 0;
	if(I == 0)
	{
		int shift = Operand2 >> 4;
		int Rm = Operand2 & 0x0000000F;
		cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm]<<shift);
	}
	if(I == 1)
	{
		int rotate = Operand2 >> 8;
		int Imm = Operand2 & 0x000000FF;
		cur = CURRENT_STATE.REGS[Rn] + (Imm<<rotate);
	}
	
	if(S == 1)
	{
		if(cur < 0)
			NEXT_STATE.CPSR |= N_N;
		if(cur == 0)
			NEXT_STATE.CPSR |= Z_N;
	}
	return 0;
}

// MVN Move Not
int MVN (int Rd, int Rn, int Operand2, int I, int S, int CC) {
 
     int cur = 0;
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
  if (I == 1) { // Move 
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  else // if I == 0, perform other shift operations
  {
    int signBit = CURRENT_STATE.REGS[Rm] & 0x80000000;
    int shift = CURRENT_STATE.REGS[Rm] >> Operand2;
    int num = shift | signBit;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = (CURRENT_STATE.REGS[Rm] << shamt5); //LSL
      break;
      case 1: cur = (CURRENT_STATE.REGS[Rm] >> shamt5); //LSR
      break;
      case 2: cur = num; // ASR
        break;
      case 3: cur = ((CURRENT_STATE.REGS[Rm] >> shamt5) |
           (CURRENT_STATE.REGS[Rm] << (32 - shamt5))); // ROR
      break;
      }     
    else
      switch (sh) {
      case 0: cur = (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
      break;
      case 1: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
      break;
      case 2: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
      break;
      case 3: cur = ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
           (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
      break;
      }      
  }
  NEXT_STATE.REGS[Rd] = ~cur;
  return 0;
}

// OR Logical OR
int ORR (int Rd, int Rn, int Operand2,int I,int S,int CC) {
	int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] |
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] | 
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] | 
    (CURRENT_STATE.REGS[Rm] >> shamt5) ;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] | 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] | 
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] | 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] | 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] | 
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] | (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    } 
  return 0;
}

// Logical AND
int AND (int Rd, int Rn, int Operand2,int I,int S,int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] & 
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] &
    (CURRENT_STATE.REGS[Rm] >> shamt5);
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] &
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] &
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] &
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] &
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))) ;
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
     if (cur == 0)
       NEXT_STATE.CPSR |= Z_N;
  } 
  return 0;
}

// EOR Exclusive OR
int EOR (int Rd, int Rn, int Operand2,int I,int S,int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> shamt5) ;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] ^ (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  } 
  return 0;
}

// LSL Logical shift left
int LSL (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] <<
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] <<
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] <<
    (CURRENT_STATE.REGS[Rm] >> shamt5) ;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] <<
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] <<
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] <<
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] <<
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] <<
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    break;
      }      
  }
  
  NEXT_STATE.REGS[Rd] = cur;
  return 0;
} 

// LSR Logical shift right
int LSR (int Rd, int Rn, int Operand2, int I, int S, int CC){

  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] >>
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] >>
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] >>
    (CURRENT_STATE.REGS[Rm] >> shamt5) ;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] >>
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] >>
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] >>
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] >>
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] >>
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    break;
      }      
  }
  
  NEXT_STATE.REGS[Rd] = cur;
  return 0;
} 

// SBC 
int SBC (int Rd, int Rn, int Operand2,int I,int S,int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
    (CURRENT_STATE.REGS[Rm] << shamt5) - C_CUR;
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
    (CURRENT_STATE.REGS[Rm] >> shamt5) - C_CUR;
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] >> shamt5) - C_CUR;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))) - C_CUR;
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]) - C_CUR;
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) - C_CUR;
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] -
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) - C_CUR;
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] -
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])))  - C_CUR;
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate))) - C_CUR;
  }
  NEXT_STATE.REGS[Rd] = cur;
	if (S == 1) {
		if (cur < 0)
			NEXT_STATE.CPSR |= N_N;
		if (cur == 0)
			NEXT_STATE.CPSR |= Z_N;
    } 
  return 0;
}

// TST 
int TST (int Rd, int Rn, int Operand2,int I,int S,int CC)
{
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
        case 0: cur = CURRENT_STATE.REGS[Rn] & 
        (CURRENT_STATE.REGS[Rm] << shamt5);
        break;
        case 1: cur = CURRENT_STATE.REGS[Rn] & 
        (CURRENT_STATE.REGS[Rm] >> shamt5);
        break;
        case 2: cur = CURRENT_STATE.REGS[Rn] &
        (CURRENT_STATE.REGS[Rm] >> shamt5);
        break;
        case 3: cur = CURRENT_STATE.REGS[Rn] & 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
         (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
        break;
      }     
      else
        switch (sh) {
          case 0: cur = CURRENT_STATE.REGS[Rn] &
          (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
          case 1: cur = CURRENT_STATE.REGS[Rn] &
          (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
          case 2: cur = CURRENT_STATE.REGS[Rn] &
          (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
          case 3: cur = CURRENT_STATE.REGS[Rn] &
          ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
           (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))) ;
          break;
        }      
      }
      if (I == 1) {
        int rotate = Operand2 >> 8;
        int Imm = Operand2 & 0x000000FF;
        cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
      }
	if (cur < 0) {
		NEXT_STATE.CPSR |= N_N;
	}
	if (cur == 0) {
		NEXT_STATE.CPSR |= Z_N;
	}
	return 0;
		printf("\n",cur);  
}

// TEQ
int TEQ (int Rd,int Rn,int Operand2,int I,int S,int CC)
{
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> shamt5) ;
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] ^ 
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] ^ (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  if (cur < 0)
    NEXT_STATE.CPSR |= N_N;
  if (cur == 0)
    NEXT_STATE.CPSR |= Z_N;
  return 0;
}

// BIC Bitwise clear
int BIC (int Rd, int Rn, int Operand2,int I,int S,int CC)
{
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] &~ 
    (CURRENT_STATE.REGS[Rm] << shamt5);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] &~ 
    (CURRENT_STATE.REGS[Rm] >> shamt5);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] &~
    (CURRENT_STATE.REGS[Rm] >> shamt5);
        break;
      case 3: cur = CURRENT_STATE.REGS[Rn] &~ 
        ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
    break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] &~
    (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    break;
      case 1: cur = CURRENT_STATE.REGS[Rn] &~
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 2: cur = CURRENT_STATE.REGS[Rn] &~
    (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    break;
      case 3: cur = CURRENT_STATE.REGS[Rn] &~
        ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))) ;
    break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & ~(Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
     if (cur == 0)
       NEXT_STATE.CPSR |= Z_N;
  } 
  return 0;
}

// STR 
int STR (int Rd, int Rn, int Imm12, int Funct) {
  int I_bar = (Funct & 0b100000) >> 5;
  int P =     (Funct & 0b010000) >> 4;
  int U =     (Funct & 0b001000) >> 3;
  int B_ =     (Funct & 0b000100) >> 2;
  int W =     (Funct & 0b000010) >> 1;
  int L =     (Funct & 0b000001);

  int offset = 0;
  if(I_bar == 0)
  { 
    offset = Imm12;
  }
  else
  { 
    int sh = (Imm12 & 0x00000060) >> 5;
    int shamt5 = (Imm12 & 0x00000F80) >> 7;
    int bit4 = (Imm12 & 0x00000010) >> 4;
    int Rm = Imm12 & 0x0000000F;
    switch (sh){
      case 0:
        offset = (CURRENT_STATE.REGS[Rm] << shamt5);
          
      case 1: 
        offset = (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
      case 2: 
        offset = (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
      case 3: 
        offset = ((CURRENT_STATE.REGS[Rm] >> shamt5) |(CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
    }
  }

  if(U == 0) offset = -offset;

  mem_write_32(CURRENT_STATE.REGS[Rn] + offset, CURRENT_STATE.REGS[Rd]);

  return 0;
}

// STRB yikes
int STRB (int Rd, int Rn, int Imm12, int Funct)
{
  return 0;
}

// LDR 
int LDR (int Rd, int Rn, int Imm12, int Funct) {
  
  int I_bar = (Funct & 0b100000) >> 5;
  int P =     (Funct & 0b010000) >> 4;
  int U =     (Funct & 0b001000) >> 3;
  int B_ =     (Funct & 0b000100) >> 2;
  int W =     (Funct & 0b000010) >> 1;
  int L =     (Funct & 0b000001);

  int offset = 0;
  if(I_bar == 0)
  { 
    offset = Imm12;
  }
  else
  { 
    int sh = (Imm12 & 0x00000060) >> 5;
    int shamt5 = (Imm12 & 0x00000F80) >> 7;
    int bit4 = (Imm12 & 0x00000010) >> 4;
    int Rm = Imm12 & 0x0000000F;
    switch (sh){
      case 0:
        offset = (CURRENT_STATE.REGS[Rm] << shamt5);
          
      case 1: 
        offset = (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
      case 2: 
        offset = (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
      case 3: 
        offset = ((CURRENT_STATE.REGS[Rm] >> shamt5) |(CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
    }
  }

  if(U == 0) offset = -offset;

  NEXT_STATE.REGS[Rd] = mem_read_32(CURRENT_STATE.REGS[Rn] + offset);

  return 0;
}

// LDRB
int LDRB (int Rd, int Rn, int Imm12, int Funct) {
  return 0;
}

//
int MUL(char* i_){
	return 0;}
int MLA(char* i_){
	return 0;}



int SWI (char* i_){return 0;}

#endif
