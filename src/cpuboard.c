/*
 *	Project-based Learning II (CPU)
 *
 *	Program:	instruction set simulator of the Educational CPU Board
 *	File Name:	cpuboard.c
 *	Descrioption:	simulation(emulation) of an instruction
 */

#include    <stdio.h>
#include	"cpuboard.h"

/* 命令コード */
enum instruction_code {
    NOP = 0x00,
    HLT = 0x0f,
    OUT = 0x10,
    IN  = 0x1f,
    RCF = 0x20,
    SCF = 0x2f,
    LD  = 0x60,
    ST  = 0x70,
    ADD = 0xb0,
    ADC = 0x90,
    SUB = 0xa0,
    SBC = 0x80,
    CMP = 0xf0,
    AND = 0xe0,
    OR  = 0xd0,
    EOR = 0xc0,
    Ssm = 0x40,
    Rsm = 0x44,
    Bbc = 0x30,
    JAL = 0x0a,
    JR  = 0x0b
};

/* オペランドB */
enum operand_b {
    ACC = 0x00,
    IX  = 0x01,
    IMMEDIATE_ADDR = 0x02,
    ABS_ADDR_TEXT = 0x04,
    ABS_ADDR_DATA = 0x05,
    IX_MOD_ADDR_TEXT = 0x06,
    IX_MOD_ADDR_DATA = 0x07
};

/* プロトタイプ宣言 */
Uword decrypt_instruction(Cpub *);
Uword decrypt_operandA(Cpub *);
Uword fetch_operandA(Cpub *);
Uword decrypt_operandB(Cpub *);
Uword fetch_operandB(Cpub *);
Bit carry_flag(short);
Bit overflow_flag(short, Uword, Uword);
Bit negative_flag(short);
Bit zero_flag(short);
void out(Cpub *);
void in(Cpub *);
void reset_cf(Cpub *);
void set_cf(Cpub *);
void load(Cpub *);
int store(Cpub *);
void add(Cpub *);
void adc(Cpub *);
void sub(Cpub *);
void err_mesg(char *);

/*=============================================================================
 *   Simulation of a Single Instruction
 *===========================================================================*/
int
step(Cpub *cpub)
{
    int return_status;
    // 命令フェッチとPC更新
    cpub->mar = cpub->pc;
    cpub->pc++;
    cpub->ir = cpub->mem[(Addr)cpub->mar];

    // 命令解読
    Uword decrypted_code = decrypt_instruction(cpub);

    // 命令実行
    switch (decrypted_code) {
        case NOP:
            puts("NOP");
            return_status = RUN_STEP;
            break;
        case HLT:
            puts("HLT");
            return_status = RUN_HALT;
            break;
        case OUT:
            puts("OUT");
            out(cpub);
            return_status = RUN_STEP;
            break;
        case IN:
            puts("IN");
            in(cpub);
            return_status = RUN_STEP;
            break;
        case RCF:
            puts("RCF");
            reset_cf(cpub);
            return_status = RUN_STEP;
            break;
        case SCF:
            puts("SCF");
            set_cf(cpub);
            return_status = RUN_STEP;
            break;
        case LD:
            puts("LD");
            load(cpub);
            return_status = RUN_STEP;
            break;
        case ST:
            puts("ST");
            return_status = store(cpub);
            break;
        case ADD:
            puts("ADD");
            //add(cpub);
            break;
        case ADC:
            puts("ADC");
            //adb(cpub);
            break;
        case SUB:
            puts("SUB");
            //sub(cpub);
            break;
        case SBC:
            puts("SBC");
            //sbc(cpub);
            break;
        case CMP:
            puts("CMP");
            //cmp(cpub);
            break;
        case AND:
            puts("AND");
            //and(cpub);
            break;
        case OR:
            puts("OR");
            //or(cpub);
            break;
        case EOR:
            puts("EOR");
            //eor(cpub);
            break;
        case Ssm:
            puts("Ssm");
            //ssm(cpub);
            break;
        case Rsm:
            puts("Rsm");
            //rsm(cpub);
            break;
        case Bbc:
            puts("Bbc");
            //bbc(cpub);
            break;
        case JAL:
            puts("JAL"); //jal(cpub);
            break;
        case JR:
            puts("JR");
            //jr(cpub);
            break;
        default:
            return_status = RUN_HALT;
            break;
    }
    return return_status;
}

/* 命令解読 */
Uword decrypt_instruction(Cpub *cpub) {
    Uword UPPER_4BIT = cpub->ir & 0xf0;     /* 上位4bit */
    Uword LOWER_4BIT;                       /* 下位4bit */

    if (cpub->ir == NOP) return NOP;
    if (cpub->ir == HLT) return HLT;
    if (cpub->ir == OUT) return OUT;
    if (cpub->ir == IN ) return IN;
    if (cpub->ir == RCF) return RCF;
    if (cpub->ir == SCF) return SCF;
    if (cpub->ir == JAL) return JAL;
    if (cpub->ir == JR ) return JR;

    switch (UPPER_4BIT) {
        case 0x60:
            return LD;
        case 0x70:
            return ST;
        case 0xb0:
            return ADD;
        case 0x90:
            return ADC;
        case 0xa0:
            return SUB;
        case 0x80:
            return SBC;
        case 0xf0:
            return CMP;
        case 0xe0:
            return AND;
        case 0xd0:
            return OR;
        case 0xc0:
            return EOR;
        case 0x40:
            return Ssm;
            //LOWER_4BIT = cpub->ir & 0x0f;
            //if ((LOWER_4BIT & 0x04) == 0x40) {
            //    return Ssm;
            //} else if ((LOWER_4BIT & 0x04) == 0x44) {
            //    return Rsm;
            //} else {
            //    err_mesg("decrypt_instruction()");
            //    break;
            //}
        case 0x30:
            return Bbc;
        default:
            err_mesg("decrypt_instruction()");
            break;
    }
}

/* オペランド解読(A) */
Uword decrypt_operandA(Cpub *cpub) {
    if (cpub->ir & 0x08) {
        return 1;   /* ACC */
    } else {
        return 0;   /* IX */
    }
}

/* オペランドフェッチ(A) */
Uword fetch_operandA(Cpub *cpub) {
    if (cpub->ir & 0x08) {
        return cpub->acc;   /* ACC */
    } else {
        return cpub->ix;    /* IX */
    }
}

/* オペランド解読(B) */
Uword decrypt_operandB(Cpub *cpub) {
    Uword decrypted_opB = cpub->ir & 0x07;
    return decrypted_opB;
}

/* オペランドフェッチ(B) */
Uword fetch_operandB(Cpub *cpub) {
    Uword decrypted_opB = decrypt_operandB(cpub);
    Uword second_word = cpub->mem[cpub->mar];
    switch (decrypted_opB) {
        case ACC:  /* ACC */
            return cpub->acc;
        case IX:  /* IX */
            return cpub->ix;
        case IMMEDIATE_ADDR:  /* 即値アドレス */
            cpub->mar = cpub->pc;
            cpub->pc++;
            return cpub->mem[decrypted_opB];
        case ABS_ADDR_TEXT:  /* 絶対アドレス(プログラム領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            return cpub->mem[second_word];
        case ABS_ADDR_DATA:  /* 絶対アドレス(データ領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            return cpub->mem[0x100 + second_word];
        case IX_MOD_ADDR_TEXT:  /* IX修飾アドレス(プログラム領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            return cpub->mem[second_word + cpub->ix];
        case IX_MOD_ADDR_DATA:  /* IX修飾アドレス(データ領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            return cpub->mem[0x100 + second_word + cpub->ix];
    }
}

/* キャリーフラグ判定 */
Bit carry_flag(short result) {
    if ((result >> 1) & 0x80) {
        return 1;
    } else {
        return 0;
    }
}

/* オーバーフローフラグ判定 */
Bit overflow_flag(short result, Uword opA, Uword opB) {
    Uword A = opA >> 7;
    Uword B = opB >> 7;
    Uword C = result >> 7;
    if ((A & B & !C) | (!A & !B & C)) {
        return 1;
    } else {
        return 0;
    }
}

/* ネガティブフラグ判定 */
Bit negative_flag(short result) {
    if (result & 0x80) {
        return 1;
    } else {
        return 0;
    }
}

/* ゼロフラグ判定 */
Bit zero_flag(short result) {
    if (result & 0xff) {
        return 0;
    } else {
        return 1;
    }
}

/* OUT命令 */
void out(Cpub *cpub) {
    cpub->obuf.buf = cpub->acc;
    cpub->obuf.flag = 1;
}

/* IN命令 */
void in(Cpub *cpub) {
    cpub->acc = cpub->ibuf->buf;
    cpub->ibuf->flag = 1;
}

/* RCF命令 */
void reset_cf(Cpub *cpub) {
    cpub->cf = 0;
}

/* SCF命令 */
void set_cf(Cpub *cpub) {
    cpub->cf = 1;
}

/* LD命令 */
void load(Cpub *cpub) {
    Uword fetched_opB = fetch_operandB(cpub);
    if (decrypt_operandA(cpub)) {
        cpub->acc = fetched_opB;
    } else {
        cpub->ix = fetched_opB;
    }
}

/* ST命令 */
int store(Cpub *cpub) {
    Uword decrypted_opB = decrypt_operandB(cpub);
    Uword fetched_opA = fetch_operandA(cpub);
    Uword second_word = cpub->mem[cpub->mar];
    int return_status;

    switch (decrypted_opB) {
        case ACC:   /* ACC */
            err_mesg("ACC is not defined in the ST instruction.");
            return_status = RUN_HALT;
            break;
        case IX:    /* IX */
            err_mesg("IX is not defined in the ST instruction.");
            return_status = RUN_HALT;
            break;
        case IMMEDIATE_ADDR:  /* 即値アドレス */
            err_mesg("Immediate Address is not defined in the ST instruction.");
            return_status = RUN_HALT;
            break;
        case ABS_ADDR_TEXT:  /* 絶対アドレス(プログラム領域) */
            cpub->mem[second_word] = fetched_opA;
            return_status = RUN_STEP;
            break;
        case ABS_ADDR_DATA:  /* 絶対アドレス(データ領域) */
            cpub->mem[0x100 + second_word] = fetched_opA;
            return_status = RUN_STEP;
            break;
        case IX_MOD_ADDR_TEXT:  /* IX修飾アドレス(プログラム領域) */
            cpub->mem[second_word + cpub->ix] = fetched_opA;
            return_status = RUN_STEP;
            break;
        case IX_MOD_ADDR_DATA:  /* IX修飾アドレス(データ領域) */
            cpub->mem[0x100 + second_word + cpub->ix] = fetched_opA;
            return_status = RUN_STEP;
            break;
        default:
            return_status = RUN_HALT;
            break;
    }
    return return_status;
}

/* ADD命令 */
void add(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    short sum = fetched_opA + fetched_opB;

    cpub->cf = carry_flag(sum);
    cpub->vf = (cpub->cf | overflow_flag(sum, fetched_opA, fetched_opB));
    cpub->nf = negative_flag(sum);
    cpub->zf = zero_flag(sum);

    if (decrypted_opA(cpub)) {
        cpub->acc = sum;
    } else {
        cpub->ix = sum;
    }
}

/* ADC命令 */
void adc(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    short sum = fetched_opA + fetched_opB + cpub->cf;

    cpub->cf = carry_flag(sum);
    cpub->vf = overflow_flag(sum, fetched_opA, fetched_opB);
    cpub->nf = negative_flag(sum);
    cpub->zf = zero_flag(sum);

    if (decrypted_opA(cpub)) {
        cpub->acc = sum;
    } else {
        cpub->ix = sum;
    }
}

/* SUB命令 */
void sub(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    short sub = fetched_opA + (~fetched_opB + 0x01);

    cpub->cf = carry_flag(sub);
    cpub->vf = overflow_flag(sub, fetched_opA, fetched_opB);
    cpub->nf = negative_flag(sub);
    cpub->zf = zero_flag(sub);

    if (decrypted_opA(cpub)) {
        cpub->acc = sub;
    } else {
        cpub->ix = sub;
    }
}

/* SBC命令 */
void sbc(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    short sbc = fetched_opA + (~fetched_opB + 0x01) + (~cpub->cf + 0x01);

    cpub->cf = !carry_flag(sbc);
    cpub->vf = overflow_flag(sbc, fetched_opA, fetched_opB);
    cpub->nf = negative_flag(sbc);
    cpub->zf = zero_flag(sbc);

    if (decrypted_opA(cpub)) {
        cpub->acc = sbc;
    } else {
        cpub->ix = sbc;
    }
}

/* エラーメッセージ表示用関数 */
void err_mesg(char *msg) {
    fprintf(stderr, "error: %s\n", msg);
}
