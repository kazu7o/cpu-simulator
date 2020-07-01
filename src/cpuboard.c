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

/* アドレッシングモード */
enum operand_b {
    ACC = 0x00,
    IX  = 0x01,
    IMMEDIATE_ADDR = 0x02,
    ABS_ADDR_TEXT = 0x04,
    ABS_ADDR_DATA = 0x05,
    IX_MOD_ADDR_TEXT = 0x06,
    IX_MOD_ADDR_DATA = 0x07
};

/* Shift&Rotate Mode */
enum shift_mode {
    RA = 0x00,
    LA = 0x01,
    RL = 0x02,
    LL = 0x03
};

/* プロトタイプ宣言 */
Uword decrypt_instruction(Cpub *);
Uword decrypt_operandA(Cpub *);
Uword fetch_operandA(Cpub *);
Uword decrypt_operandB(Cpub *);
Uword fetch_operandB(Cpub *);
Bit carry_flag(Uword, Uword);
Bit overflow_flag(Uword, Uword);
Bit negative_flag(Uword);
Bit zero_flag(Uword);
void out(Cpub *);
void in(Cpub *);
void reset_cf(Cpub *);
void set_cf(Cpub *);
void load(Cpub *);
int store(Cpub *);
void add(Cpub *);
void adc(Cpub *);
void sub(Cpub *);
void sbc(Cpub *);
void compare(Cpub *);
void and(Cpub *);
void or(Cpub *);
void eor(Cpub *);
void shift(Cpub *);
void rotate(Cpub *);
void branch(Cpub *);
void jal(Cpub *);
void jr(Cpub *);
void err_mesg(char *);

/*=============================================================================
 *   Simulation of a Single Instruction
 *===========================================================================*/
int
step(Cpub *cpub)
{
    int return_status = RUN_STEP;
    /* 命令フェッチとPC更新 */
    cpub->mar = cpub->pc;
    cpub->pc++;
    cpub->ir = cpub->mem[cpub->mar];

    /* 命令解読 */
    Uword decrypted_code = decrypt_instruction(cpub);

    /* 命令実行 */
    switch (decrypted_code) {
        case NOP:
            return_status = RUN_STEP;
            break;
        case HLT:
            return_status = RUN_HALT;
            break;
        case OUT:
            out(cpub);
            break;
        case IN:
            in(cpub);
            break;
        case RCF:
            reset_cf(cpub);
            break;
        case SCF:
            set_cf(cpub);
            break;
        case LD:
            load(cpub);
            break;
        case ST:
            return_status = store(cpub);
            break;
        case ADD:
            add(cpub);
            break;
        case ADC:
            adc(cpub);
            break;
        case SUB:
            sub(cpub);
            break;
        case SBC:
            sbc(cpub);
            break;
        case CMP:
            compare(cpub);
            break;
        case AND:
            and(cpub);
            break;
        case OR:
            or(cpub);
            break;
        case EOR:
            eor(cpub);
            break;
        case Ssm:
            shift(cpub);
            break;
        case Rsm:
            rotate(cpub);
            break;
        case Bbc:
            branch(cpub);
            break;
        case JAL:
            jal(cpub);
            break;
        case JR:
            jr(cpub);
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

    /* JAL, JR */
    if (cpub->ir == JAL) return JAL;
    if (cpub->ir == JR ) return JR;

    /* NOP, HLT */
    if (UPPER_4BIT == 0x00) {
        Uword bit4 = cpub->ir & 0x08;       /* 下位4bit目 */
        if (bit4 == 0) {
            return NOP;
        } else {
            return HLT;
        }
    }

    /* RCF, SCF */
    if (UPPER_4BIT == 0x10) {
        Uword bit4 = cpub->ir & 0x08;
        if (bit4 == 0) {
            return RCF;
        } else {
            return SCF;
        }
    }

    /* Ssm, Rsm */
    if (UPPER_4BIT == 0x40) {
        Uword bit3 = cpub->ir & 0x04;       /* 下位3bit目 */
        if (bit3 == 0) {
            return Ssm;
        } else {
            return Rsm;
        }
    }

    /* それ以外 */
    return UPPER_4BIT;

    //switch (UPPER_4BIT) {
    //    case LD:
    //        return LD;
    //    case ST:
    //        return ST;
    //    case ADD:
    //        return ADD;
    //    case ADC:
    //        return ADC;
    //    case SUB:
    //        return SUB;
    //    case 0x80:
    //        return SBC;
    //    case 0xf0:
    //        return CMP;
    //    case 0xe0:
    //        return AND;
    //    case 0xd0:
    //        return OR;
    //    case 0xc0:
    //        return EOR;
    //    case 0x40:
    //        bit3 = cpub->ir & 0x40;
    //        if (bit3 == 0) {
    //            return Ssm;
    //        } else {
    //            return Rsm;
    //        }
    //    case 0x30:
    //        return Bbc;
    //    default:
    //        err_mesg("decrypt_instruction()");
    //        break;
    //}
}

/* オペランド解読(A) */
Uword decrypt_operandA(Cpub *cpub) {
    if (cpub->ir & 0x08) {
        return 1;   /* IX */
    } else {
        return 0;   /* ACC */
    }
}

/* オペランドフェッチ(A) */
Uword fetch_operandA(Cpub *cpub) {
    if (cpub->ir & 0x08) {
        return cpub->ix;    /* IX */   
    } else {
        return cpub->acc;   /* ACC */
    }
}

/* オペランド解読(B) */
Uword decrypt_operandB(Cpub *cpub) {
    Uword decrypted_opB = cpub->ir & 0x07;
    if (decrypted_opB == 0x03) decrypted_opB = 0x02;    // 即値アドレスのときは下位1bitを0に設定
    return decrypted_opB;
}

/* オペランドフェッチ(B) */
Uword fetch_operandB(Cpub *cpub) {
    Uword decrypted_opB = decrypt_operandB(cpub);
    Uword second_word;
    switch (decrypted_opB) {
        case ACC:  /* ACC */
            return cpub->acc;
        case IX:  /* IX */
            return cpub->ix;
        case IMMEDIATE_ADDR:  /* 即値アドレス */
            cpub->mar = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[cpub->mar];
            return second_word;
        case ABS_ADDR_TEXT:  /* 絶対アドレス(プログラム領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[cpub->mar];
            return cpub->mem[second_word];
        case ABS_ADDR_DATA:  /* 絶対アドレス(データ領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[cpub->mar];
            return cpub->mem[0x100 + second_word];
        case IX_MOD_ADDR_TEXT:  /* IX修飾アドレス(プログラム領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[cpub->mar];
            return cpub->mem[second_word + cpub->ix];
        case IX_MOD_ADDR_DATA:  /* IX修飾アドレス(データ領域) */
            cpub->mar = cpub->pc;
            cpub->pc++;
            second_word = cpub->mem[cpub->mar];
            return cpub->mem[0x100 + second_word + cpub->ix];
    }
}

/* キャリーフラグ判定 */
Bit carry_flag(Uword opA, Uword opB) {
    Uword msb_A = opA >> 7;
    Uword msb_B = opB >> 7;
    Uword CY = ((opA & 0x7f) + (opB & 0x7f)) >> 7;
    if ((msb_A & msb_B) | (msb_A & CY) | (msb_B & CY)) {
        return 1;
    } else {
        return 0;
    }
}

/* オーバーフローフラグ判定 */
Bit overflow_flag(Uword opA, Uword opB) {
    Uword msb_A = opA >> 7;
    Uword msb_B = opB >> 7;
    Uword CY = ((opA & 0x7f) + (opB & 0x7f)) >> 7;
    if ((msb_A & msb_B & !CY) | (!msb_A & !msb_B & CY)) {
        return 1;
    } else {
        return 0;
    }
}

/* ネガティブフラグ判定 */
Bit negative_flag(Uword result) {
    if (result & 0x80) {
        return 1;
    } else {
        return 0;
    }
}

/* ゼロフラグ判定 */
Bit zero_flag(Uword result) {
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
        cpub->ix = fetched_opB;
    } else {
        cpub->acc = fetched_opB;
    }
}

/* ST命令 */
int store(Cpub *cpub) {
    Uword decrypted_opB = decrypt_operandB(cpub);
    Uword fetched_opA = fetch_operandA(cpub);
    cpub->mar = cpub->pc;
    cpub->pc++;
    Uword second_word = cpub->mem[cpub->mar];
    int return_status = RUN_STEP;

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
            break;
        case ABS_ADDR_DATA:  /* 絶対アドレス(データ領域) */
            cpub->mem[0x100 + second_word] = fetched_opA;
            break;
        case IX_MOD_ADDR_TEXT:  /* IX修飾アドレス(プログラム領域) */
            cpub->mem[second_word + cpub->ix] = fetched_opA;
            break;
        case IX_MOD_ADDR_DATA:  /* IX修飾アドレス(データ領域) */
            cpub->mem[0x100 + second_word + cpub->ix] = fetched_opA;
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
    Uword sum = fetched_opA + fetched_opB;

    cpub->cf = carry_flag(fetched_opA, fetched_opB);
    cpub->vf = (cpub->cf | overflow_flag(fetched_opA, fetched_opB));
    cpub->nf = negative_flag(sum);
    cpub->zf = zero_flag(sum);

    if (decrypt_operandA(cpub)) {
        cpub->ix = sum;
    } else {
        cpub->acc = sum;
    }
}

/* ADC命令 */
void adc(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword sum = fetched_opA + fetched_opB + cpub->cf;

    cpub->cf = carry_flag(fetched_opA, fetched_opB);
    cpub->vf = overflow_flag(fetched_opA, fetched_opB);
    cpub->nf = negative_flag(sum);
    cpub->zf = zero_flag(sum);

    if (decrypt_operandA(cpub)) {
        cpub->ix = sum;
    } else {
        cpub->acc = sum;
    }
}

/* SUB命令 */
void sub(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword sub = fetched_opA + (~fetched_opB + 0x01);

    cpub->cf = carry_flag(fetched_opA, ((~fetched_opB) + 0x01));
    cpub->vf = overflow_flag(fetched_opA, fetched_opB);
    cpub->nf = negative_flag(sub);
    cpub->zf = zero_flag(sub);

    if (decrypt_operandA(cpub)) {
        cpub->ix = sub;
    } else {
        cpub->acc = sub;
    }
}

/* SBC命令 */
void sbc(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword sbc = fetched_opA + (~fetched_opB + 0x01) + (~cpub->cf + 0x01);

    cpub->cf = carry_flag(fetched_opA, ((~fetched_opB + 0x01) + (~cpub->cf + 0x01)));
    cpub->vf = overflow_flag(fetched_opA, fetched_opB);
    cpub->nf = negative_flag(sbc);
    cpub->zf = zero_flag(sbc);

    if (decrypt_operandA(cpub)) {
        cpub->ix = sbc;
    } else {
        cpub->acc = sbc;
    }
}

/* CMP命令 */
void compare(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword result = fetched_opA + (~fetched_opB + 1);

    cpub->vf = overflow_flag(fetched_opA, (~fetched_opB + 1));
    cpub->nf = negative_flag(result);
    cpub->zf = zero_flag(result);
}

/* AND命令 */
void and(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword result = fetched_opA & fetched_opB;

    cpub->vf = 0;
    cpub->nf = negative_flag(result);
    cpub->zf = zero_flag(result);

    if (decrypt_operandA(cpub)) {
        cpub->ix = result;
    } else {
        cpub->acc = result;
    }
}

/* OR命令 */
void or(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword result = fetched_opA | fetched_opB;

    cpub->vf = 0;
    cpub->nf = negative_flag(result);
    cpub->zf = zero_flag(result);

    if (decrypt_operandA(cpub)) {
        cpub->ix = result;
    } else {
        cpub->acc = result;
    }
}

/* EOR命令 */
void eor(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword fetched_opB = fetch_operandB(cpub);
    Uword result = fetched_opA ^ fetched_opB;

    cpub->vf = 0;
    cpub->nf = negative_flag(result);
    cpub->zf = zero_flag(result);

    if (decrypt_operandA(cpub)) {
        cpub->ix = result;
    } else {
        cpub->acc = result;
    }
}

/* Shift命令 */
void shift(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword sm = cpub->ir & 0x03;
    Uword msb = fetched_opA & 0x80;
    Uword lsb = fetched_opA & 0x01;
    Uword shifted;

    switch (sm) {
        case RA:  /* SRA */
            shifted = fetched_opA >> 1;
            shifted = shifted | msb;
            cpub->cf = lsb;
            cpub->vf = 0;
            break;
        case LA:  /* SLA */
            shifted = fetched_opA << 1;
            cpub->cf = msb;
            cpub->vf = (fetched_opA ^ shifted) & 0x80; //符号bitが変わったら
            break;
        case RL:  /* SRL */
            shifted = fetched_opA >> 1;
            cpub->cf = lsb;
            cpub->vf = 0;
            break;
        case LL:  /* SLL */
            shifted = fetched_opA << 1;
            cpub->cf = msb;
            cpub->vf = 0;
            break;
    }

    cpub->nf = negative_flag(shifted);
    cpub->zf = zero_flag(shifted);

    if (decrypt_operandA(cpub)) {
        cpub->ix = shifted;
    } else {
        cpub->acc = shifted;
    }
}

/* Rotate命令 */
void rotate(Cpub *cpub) {
    Uword fetched_opA = fetch_operandA(cpub);
    Uword sm = cpub->ir & 0x03;
    Uword msb = fetched_opA & 0x80;
    Uword lsb = fetched_opA & 0x01;
    Uword rotated;

    switch (sm) {
        case RA:  /* RRA */
            rotated = fetched_opA >> 1;
            rotated = rotated | (cpub->cf << 7);
            cpub->cf = lsb;
            cpub->vf = 0;
            break;
        case LA:  /* RLA */
            rotated = fetched_opA << 1;
            rotated = rotated | (cpub->cf >> 7); 
            cpub->cf = msb;
            cpub->vf = (fetched_opA ^ rotated) & 0x80; //符号bitが変わったら
            break;
        case RL:  /* RRL */
            rotated = fetched_opA >> 1;
            rotated = rotated | (lsb << 7);
            cpub->cf = lsb;
            cpub->vf = 0;
            break;
        case LL:  /* RLL */
            rotated = fetched_opA << 1;
            rotated = rotated | (msb >> 7);
            cpub->cf = msb;
            cpub->vf = 0;
            break;
    }

    cpub->nf = negative_flag(rotated);
    cpub->zf = zero_flag(rotated);

    if (decrypt_operandA(cpub)) {
        cpub->ix = rotated;
    } else {
        cpub->acc = rotated;
    }
}

/* Branch命令 */
void branch(Cpub *cpub) {
    Uword bc = cpub->ir & 0x0f;
    Uword B2;
    cpub->mar = cpub->pc;
    cpub->pc++;
    B2 = cpub->mem[cpub->mar];
    switch (bc) {
        case 0x00:  /* A */
            cpub->pc = B2;
            break;
        case 0x08:  /* VF */
            if (cpub->vf == 1) cpub->pc = B2;
            break;
        case 0x01:  /* NZ */
            if (cpub->zf == 0) cpub->pc = B2;
            break;
        case 0x09:  /* Z */
            if (cpub->zf == 1) cpub->pc = B2;
            break;
        case 0x02:  /* ZP */
            if (cpub->nf == 0) cpub->pc = B2;
            break;
        case 0x0a:  /* N */
            if (cpub->nf == 1) cpub->pc = B2;
            break;
        case 0x03:  /* P */
            if ((cpub->nf | cpub->zf) == 0) cpub->pc = B2;
            break;
        case 0x0b:  /* ZN */
            if ((cpub->nf | cpub->zf) == 1) cpub->pc = B2;
            break;
        case 0x04:  /* NI */
            if (cpub->ibuf->flag == 0) cpub->pc = B2;
            break;
        case 0x0c:  /* NO */
            if (cpub->obuf.flag == 1) cpub->pc = B2;
            break;
        case 0x05:  /* NC */
            if (cpub->cf == 0) cpub->pc = B2;
            break;
        case 0x0d:  /* C */
            if (cpub->cf == 1) cpub->pc = B2;
            break;
        case 0x06:  /* GE */
            if ((cpub->vf ^ cpub->nf) == 0) cpub->pc = B2;
            break;
        case 0x0e:  /* LT */
            if ((cpub->vf ^ cpub->nf) == 1) cpub->pc = B2;
            break;
        case 0x07:  /* GT */
            if (((cpub->vf ^ cpub->nf) | (cpub->zf)) == 0) cpub->pc = B2;
            break;
        case 0x0f:  /* LE */
            if (((cpub->vf ^ cpub->nf) | (cpub->zf)) == 1) cpub->pc = B2;
            break;
    }
}

/* JAL命令 */
void jal(Cpub *cpub) {
    Uword B2;
    cpub->mar = cpub->pc;
    cpub->pc++;
    B2 = cpub->mem[cpub->mar];
    cpub->acc = cpub->pc;
    cpub->pc = B2;
}

/* JR命令 */
void jr(Cpub *cpub) {
    cpub->pc = cpub->acc;
}

/* エラーメッセージ表示用関数 */
void err_mesg(char *msg) {
    fprintf(stderr, "error: %s\n", msg);
}
