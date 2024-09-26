#include "spimcore.h"

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch (ALUControl)
    {
    case 0: // add
        *ALUresult = A + B;
        break;
    case 1: // subtract
        *ALUresult = A - B;
        break;
    case 2: // less than
        /*
        This one I think we're supposed to cast
        it to int because it's originally unsigned.
        */
        *ALUresult = ((int) A < (int) B) ? 1: 0;
        break;
    case 3: // less than (unsigned int)
        /*
        Since this one is unsigned, I think we just
        leave it as is because it's passed into the
        function as unsigned.
        */
        *ALUresult = (A < B) ? 1 : 0; // leo was here :)
        break;
    case 4: // bw AND
        *ALUresult = A & B;
        break;
    case 5: // bw XOR
        *ALUresult = A ^ B;
        break;
    case 6: // B left shift 16 bits
        *ALUresult = B << 16; // leo was here :)
        break;
    case 7: // NOT A
        *ALUresult = ~A;
        break;
    }

    // assigning Zero based on ALUresult
    *Zero = (*ALUresult == 0) ? 1: 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // in spimcore it shows memory size 65536 which can cause a halt if exceeded
    // address must be word aligned by being a multiple of 4.
    if (PC > 65535 || PC % 4 != 0) {
        // halt occured
        return 1;
    }

    // hint said use PC >> 2 for arr idx... 
    *instruction = Mem[PC >> 2];

    return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    *op = (instruction >> 26) & 0x0000003F; // [31-26] so go to bit 26 and read 6 bits since 31-26=5 and add 1.
    *r1 = (instruction >> 21) & 0x0000001F; // [21-25] go to bit 21 and read 5 bits since 25-21=4 and add 1.
    *r2 = (instruction >> 16) & 0x0000001F; // [20-16] same as last, read 5 bits but this time start at 16.
    *r3 = (instruction >> 11) & 0x0000001F; // [15-11] same as last, read 5 bits but this time start at 11.
    *funct = (instruction) & 0x0000003F; // [5-0] dont need bitshift since starting at 0, read 5 bits since 5-0=5 and add 1.
    *offset = (instruction) & 0x0000FFFF; // [15-0] dont need bitshift since starts @ 0, read 16 bits since 15-0=15 and add 1.
    *jsec = (instruction) & 0x03FFFFFF; // [25-0] dont need bitshift since starts @ 0, read 26 bits since 25-0=25 and add 1.
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    switch(op)
    {
    case 0: // mips R type
        controls->RegDst = 1;
        controls->RegWrite = 1;
        controls->ALUSrc = 0;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 7;
        break;
    case 2: // mips J type
        controls->RegDst = 0;
        controls->RegWrite = 0;
        controls->ALUSrc = 0;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 0;
        controls->Jump = 1;
        controls->Branch = 0;
        controls->ALUOp = 0;
        break;
    case 4: // mips I type beq
        controls->RegDst = 2;
        controls->RegWrite = 0;
        controls->ALUSrc = 0;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 2;
        controls->Jump = 0;
        controls->Branch = 1;
        controls->ALUOp = 1;
        break;
    case 8: // mips I type addi
        controls->RegDst = 0;
        controls->RegWrite = 1;
        controls->ALUSrc = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 0;
        break;
    case 10: // mips I type slt
        controls->RegDst = 0;
        controls->RegWrite = 1;
        controls->ALUSrc = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 2;
        break;
    case 11: // mips I type sltu
        controls->RegDst = 0;
        controls->RegWrite = 1;
        controls->ALUSrc = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 3;
        break;
    case 15: // mips I type lui
        controls->RegDst = 0;
        controls->RegWrite = 1;
        controls->ALUSrc = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->MemtoReg = 0;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 6;
        break;
    case 35: // mips I type lw
        controls->RegDst = 0;
        controls->RegWrite = 1;
        controls->ALUSrc = 1;
        controls->MemRead = 1;
        controls->MemWrite = 0;
        controls->MemtoReg = 1;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 0;
        break;
    case 43: // mips I type sw
        controls->RegDst = 2;
        controls->RegWrite = 0;
        controls->ALUSrc = 1;
        controls->MemRead = 0;
        controls->MemWrite = 1;
        controls->MemtoReg = 2;
        controls->Jump = 0;
        controls->Branch = 0;
        controls->ALUOp = 0;
        break;
    default:
        // if halt occurs
        return 1;
    }

    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    // writing the read values from these registers r1,r2 to data1,data2
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // if negative MSB, extended with 1s for upper half, else leave as is so 0s in upper half.
    *extended_value = (offset & 0x00008000)? (offset | 0xFFFF0000): offset;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    // applying ALU operations on data1, and data2/extended_value
    if (ALUSrc == 1) data2 = extended_value;
    if (ALUOp == 7) 
    {
        switch (funct)
        {
            case 32: // add
                ALUOp = 0;
                break;
            case 34: // sub
                ALUOp = 1;
                break;
            case 42: // slt
                ALUOp = 2;
                break;
            case 43: // sltu
                ALUOp = 3;
                break;
            case 36: // and
                ALUOp = 4;
                break;
            case 38: // xor
                ALUOp = 5;
                break;
            case 4: // shift left
                ALUOp = 6;
                break;
            case 39: // not
                ALUOp = 7;
                break;
            default:
            // halt occurs
                return 1;
        }
        ALU(data1, data2, ALUOp, ALUresult, Zero);
    } else {
        ALU(data1, data2, ALUOp, ALUresult, Zero);
    }

    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if (MemRead == 1 && (ALUresult % 4) == 0) {
        // reads memory if occuring and no halt from ALUresult alignment occurs
        *memdata = Mem[ALUresult >> 2];
    } else if (MemRead == 1) {
        // halt
        return 1;
    }
    
    if (MemWrite == 1 && (ALUresult % 4) == 0) {
        // writes memory if occuring and no halt from ALUresult alignment occurs
        Mem[ALUresult >> 2] = data2;
    } else if (MemWrite == 1) {
        // halt
        return 1;
    }

    return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite == 1) {
        // writes data to register r2/3
        if (MemtoReg == 1 && RegDst == 0) {
            // transfers memory data to reg 2 since Dst is set to off
            Reg[r2] = memdata;
        }
        else if (MemtoReg == 1 && RegDst == 1) {
            // transfers memory data to reg 3 since Dst is set to on
            Reg[r3] = memdata;
        }
        else if (MemtoReg == 0 && RegDst == 0) {
            // transfers ALU result to reg 2 since Dst is set to off
            Reg[r2] = ALUresult;
        }
        else if (MemtoReg == 0 && RegDst == 1) {
            // transfers ALU result to reg 2 since Dst is set to on
            Reg[r3] = ALUresult;
        }
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // updates memory and stays word aligned by incrementing by 4.
    *PC += 4;

    // 
    if (Zero == 1 && Branch == 1) {
        // if branch statement occurs w/ zero set then adjusts address by extended val
        *PC += extended_value << 2;
    } else if (Jump == 1) {
        // if a jump statement occurs it adjusts address
        *PC = (jsec << 2) | (*PC & 0xf0000000);
    }
}