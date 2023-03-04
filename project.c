/*
Samin Zaman
Deodatus Marcellino 
CDA 3103 w/ Sarah Angell
MySPIM Project
19th November, 2021
*/


#include "spimcore.h"


/* ALU * by Deodatus/ 
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

switch((int)ALUControl){
        //Z = A+B
        case 0:
            *ALUresult = A + B;
            break;
        //Z = A-B
        case 1:
            *ALUresult = A - B;
            break;
        //If A<B, Z = 1, otherwise Z = 0
        case 2:
            if((signed) A < (signed) B)
                *ALUresult = 1;
            else
                *ALUresult = 0;
            break;
        //If A<B, Z = 1, otherwise Z = 0 UNSIGNED
        case 3:
            if(A < B)
                *ALUresult = 1;
            else
                *ALUresult = 0;
            break;
        //Z = A AND B
        case 4:
            *ALUresult = A & B;
            break;
        //Z = A OR B
        case 5:
            *ALUresult = A | B;
            break;
        //Shift left B by 16 bits
        case 6:
            *ALUresult = B << 16;   
            break;
        //Z = NOT A
        case 7:
            *ALUresult = ~A;
            break;
    }
    
    // Check if the result = zero, and set the zero bit.
    if(*ALUresult == 0)
        *Zero = 1;
    else
        *Zero = 0;
} // end function ALU


/* instruction fetch * by Deodatus/
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{

unsigned i = PC >> 2;
if(PC % 4 != 0) return 1;
*instruction = Mem[i];
return 0;

} // end function instruction_fetch

/* instruction partition * by Deodatus/
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) //6.c
{
// we will use these partitions to only change values we want
    
	unsigned a	= 0x1f;	
	unsigned b	= 0x0000003f;	
	unsigned c	= 0x0000ffff;	
	unsigned d	= 0x03ffffff;	

	// apply partitions and shifts
	*op		= (instruction >> 26) & b;	// instruction [31-26]
	*r1		= (instruction >> 21) & a; // instruction [25-21]
	*r2		= (instruction >> 16) & a; // instruction [20-16]
	*r3		= (instruction >> 11) & a; // instruction [15-11]
	*funct	= instruction & b; // instruction [5-0]
	*offset	= instruction & c; // instruction [15-0]
	*jsec	= instruction & d; // instruction [25-0]

} // end function instruction_partition

/* instruction decode * by Samin /
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    controls->RegDst=0;
	controls->Jump=0;
	controls->Branch=0;
	controls->MemRead=0;
	controls->MemtoReg=0;
	controls->ALUOp=0;
	controls->MemWrite=0;
	controls->ALUSrc=0;
	controls->RegWrite=0;

switch (op){

        //update instructions that are enabled
        case 0: //r-type
            controls->RegDst = 0x1;
            controls->ALUOp = 0x7;
            controls->RegWrite = 0x1;
            break;

        case 2: //jump
            controls->RegDst = 0x2;
            controls->Jump = 0x1;
            break;

        case 4: //beq
            controls->RegDst = 0x2;
            controls->Branch = 0x1;
            controls->MemtoReg = 0x2;
            break;

        case 8: //addi
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;

        case 10: //slti
            controls->ALUOp = 0x2;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;

        case 11: //sltiu
            controls->ALUOp = 0x3;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;

        case 15: //lui
            controls->ALUOp = 0x6;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;

        case 35: //lw
            controls->MemRead = 0x1;
            controls->MemtoReg = 0x1;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;

        case 43: //sw
            controls->RegDst = 0x2;
            controls->MemtoReg = 0x2;
            controls->MemWrite = 0x1;
            controls->ALUSrc = 0x1;
            break;
        
        default: 

        //Return 1 if a halt condition occurs; otherwise, return 0. 
        return 1;

    }
    
    return 0;
} // end function instruction_decode

/* Read Register * by Samin/
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    *data1 = Reg[r1];
    *data2 = Reg[r2];

} // end function read_register

/* Sign Extend *by Samin/
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

int bitmask = ((1 << 16) - 1) << 16;
*extended_value = offset;
if (offset & (1 << 15))
    *extended_value |= bitmask;

} // end function sign_extend

/* ALU operations * by Deodatus/
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
//ALUSrc multiplexor selects the extended value if it is one
    if (ALUSrc == 1)
    {
        data2 = extended_value;
    }
    //Changes ALUOp depending on the function code if OP code is an R-type
    if (ALUOp == 7) {
        if (funct == 32) ALUOp = 0; //Add
        else if (funct == 34) ALUOp = 1; //Sub
        else if (funct == 42) ALUOp = 2; //Sls set less than
        else if (funct == 43) ALUOp = 3; //Slu Set less than unsigned
        else if (funct == 36) ALUOp = 4; //And
        else if (funct == 37) ALUOp = 5; //Or
        else if (funct ==  6) ALUOp = 6; //Sle16 Shift Left extended value 16
        else if (funct == 39) ALUOp = 7; //Nor
        else return 1; // Invalid operation, halt, Send to ALU for non funct
    }
    //Run ALU command once ALUOp has been corrected if needed / 
    ALU(data1, data2, ALUOp, ALUresult, Zero);
    // No halt condition encountered, Send to ALU for funct
    return 0;

} // end function ALU_operations


/* Read / Write Memory * by Samin/
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

	if(ALUresult % 4 != 0)
		return 1;
	if (MemRead == 1) {
        *memdata = Mem[ALUresult >> 2];
    }
    if (MemWrite == 1) {
        Mem[ALUresult >> 2] = data2;
    }
    return 0;

} // end function rw_memory

/* Write Register * by Deodatus/
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	if(RegWrite == 1){
		 // If Mem to Register
		if (MemtoReg == 1 && RegDst == 0) {
			Reg[r2] = memdata;
		 }

		//If Mem to Register but r3
		else if(MemtoReg == 1 && RegDst == 1){
			 Reg[r3] = memdata;
		 }
     
		// If Result to Register
		else if (MemtoReg == 0 && RegDst == 0) {
			Reg[r2] = ALUresult;
		 }
                  
		// If Result to Register but next value
		else if (MemtoReg == 0 && RegDst == 1){
			Reg[r3] = ALUresult;
		 }
	} 
}// end function write_register

/* PC update * by Samin/
/* 10 Points */

void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{

// Shift left 2, combine with PC + 4
    unsigned newPC = *PC + 4;
 
    // Combine jsec shifted left twice with the upper 4 bits of PC
    unsigned jumpAddr = (jsec << 2) | (newPC & (15 << 28));
 
    // Extended value shifted left 2 + PC + 4
    unsigned ALUresult = (extended_value << 2) + (newPC);
 
    // Top right-most mux
    if (Jump) {
 
        // Jump to address
        *PC = jumpAddr;
 
    } else {
 
        // Left mux
        if (Zero && Branch) {
 
            // Instruction from ALU
            *PC = ALUresult;
 
        } else {
 
            // Next instruction
            *PC = newPC;
 
        }
 
    }

} // end function PC_update