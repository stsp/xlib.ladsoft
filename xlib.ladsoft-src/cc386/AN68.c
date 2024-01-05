/* 
CC386 C Compiler
Copyright 1994-2011 David Lindauer.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

This program is derived from the cc68k complier by 
Matthew Brandt (mailto::mattb@walkingdog.net) 

You may contact the author of this derivative at:

mailto::camille@bluegrass.net
 */
/*
 * 22-Aug-1999 (SJS) Modified for ColdFire movem instruction restrictions.
 * 07-May-2001 (SJS) Modified alloc/free of data/address registers.
 */
#include <stdio.h>
#include <string.h>
#include "lists.h"
#include "expr.h"
#include "c.h"
#include "gen68.h"
#include "diag.h"
#include "rtti.h"

/* pc-relative expressions not optimized */
extern int linkreg, basereg;
extern AMODE push[], pop[], mvma7[], mvma7i[];
extern OCODE *peep_tail;
extern SYM *currentfunc;
extern int prm_stackcheck, prm_phiform, prm_linkreg, prm_datarel, prm_smalldata,
    prm_68020;
extern int prm_coldfire;
extern int floatregs, dataregs, addrregs;
extern long framedepth, stackdepth;
extern int cf_maxfloat, cf_maxaddress, cf_maxdata;
extern int cf_freedata, cf_freeaddress;
extern CSE *olist; /* list of optimizable expressions */
extern int prm_smartframes;
extern int nextlabel;
extern int prm_xcept, conscount, xceptoffs;
extern TRYBLOCK *try_block_list;
extern int prm_cplusplus, prm_xcept;

int floatstack_mode;
long lc_maxauto;
int save_mask, fsave_mask;
int rmaskcnt;
OCODE *frame_ins;
void reserveregs(int *datareg, int *addreg, int *floatreg)
/*
 * Reserve regs goes through and reserves a register for variables with
 * the REGISTER keyword.  Note that it currently does register allocation
 * backwards...
 */
{
    CSE *csp = olist;

    while (csp)
    {
        switch (csp->exp->nodetype)
        {
            case en_fcomplexref:
            case en_rcomplexref:
            case en_lrcomplexref:
            case en_ll_ref:
            case en_ull_ref:
                break;
            case en_fimaginaryref:
            case en_rimaginaryref:
            case en_lrimaginaryref:
            case en_floatref:
            case en_doubleref:
            case en_longdoubleref:
            case en_b_ref:
            case en_bool_ref:
            case en_w_ref:
            case en_l_ref:
            case en_ub_ref:
            case en_uw_ref:
            case en_ul_ref:
			case en_i_ref:
			case en_ui_ref:
			case en_a_ref: case en_ua_ref:
                if (csp->exp->v.p[0]->nodetype != en_autoreg)
                    break;
                if (csp->size > 6)
                {
                    if (*floatreg < cf_maxfloat && floatregs)
                        csp->reg = (*floatreg)++;
                }
                else if (csp->size != 6 && csp->size !=  - 6)
                {
                    if ((*datareg < cf_maxdata) && (csp->duses <= csp->uses / 4)
                        && dataregs)
                        csp->reg = (*datareg)++;
                    else if (!(csp->size == 1 || csp->size ==  - 1) && (*addreg
                        < cf_maxaddress) && addrregs)
                        csp->reg = (*addreg)++;
                }
                if (csp->reg !=  - 1)
                {
                    ((SYM*)csp->exp->v.p[0]->v.p[0])->inreg = TRUE;
                    ((SYM*)csp->exp->v.p[0]->v.p[0])->value.i =  - csp->reg -
                        csp->size *256;
                }
                break;
        }
        csp = csp->next;
    }
}

//-------------------------------------------------------------------------

void allocate(int datareg, int addreg, int floatreg, SNODE *block)
/*
 *      allocate will allocate registers for the expressions that have
 *      a high enough desirability.
 */
{
    CSE *csp;
    ENODE *exptr,  *mvn1,  *mvn2;
    unsigned mask, i, fmask, size;
    AMODE *ap,  *ap2,  *ap3;
    framedepth = 4+lc_maxauto;
    rmaskcnt = 0;
    mask = 0;
    fmask = 0;
    for (i = cf_freedata; i < datareg; i++)
    {
        framedepth += 4;
        mask = mask | (1 << i);
    }
    for (i = cf_freeaddress + 16; i < addreg; i++)
    {
        framedepth += 4;
        mask = mask | (1 << (i - 8));
    }
    while (bsort(&olist))
        ;
     /* sort the expression list */
    csp = olist;
    while (csp != 0)
    {
        if (csp->reg ==  - 1 && !(csp->exp->cflags &DF_VOL) && !csp->voidf)
        {
            if (desire(csp) < 3)
                csp->reg =  - 1;
            else
            {
                if (csp->size > 4)
                {
                    if (floatreg < cf_maxfloat && floatregs)
                        csp->reg = floatreg++;
                }
                else if ((datareg < cf_maxdata) && (csp->duses <= csp->uses / 4)
                    && dataregs)
                    csp->reg = datareg++;
                else if (!(csp->size == 1 || csp->size ==  - 1) && (addreg <
                    cf_maxaddress) && addrregs)
                    csp->reg = addreg++;
            }
        }
        if (csp->reg !=  - 1)
        {
            if (lvalue(csp->exp) && !((SYM*)csp->exp->v.p[0]->v.p[0])->funcparm)
            {
                ((SYM*)csp->exp->v.p[0]->v.p[0])->inreg = TRUE;
                ((SYM*)csp->exp->v.p[0]->v.p[0])->value.i =  - csp->reg - csp
                    ->size *256;
            }
            if (csp->reg < 16)
            {
                framedepth += 4;
                mask = mask | (1 << csp->reg);
                rmaskcnt++;
            }
            else if (csp->reg < 32)
            {
                framedepth += 4;
                mask = mask | (1 << (csp->reg - 8));
                rmaskcnt++;
            }
            else
            {
                framedepth += 12;
                fmask = fmask | (1 << (csp->reg - 32));
            }
        }
        csp = csp->next;
    }
    allocstack(); /* Allocate stack space for the local vars */
    floatstack_mode = 0; /* no space for floating point temps */
    if (currentfunc->tp->lst.head != 0 && currentfunc->tp->lst.head != (SYM*) -
        1)
    {
        if (prm_phiform || currentfunc->intflag)
        {
            mask |= (1 << (linkreg + 8));
            framedepth += 4;
        }
        if (currentfunc->intflag)
        {
            mask |= 0xffff;
            framedepth = lc_maxauto;
        }
    }
    if ((conscount || try_block_list || currentfunc->value.classdata.throwlist
        && currentfunc->value.classdata.throwlist->data) && prm_xcept)
    {
        xceptoffs = lc_maxauto += sizeof(XCEPTDATA);
    }
    if (prm_cplusplus && prm_xcept || prm_linkreg && !currentfunc->intflag && 
        (currentfunc->tp->lst.head && currentfunc->tp->lst.head != (SYM*) - 1 
        || lc_maxauto) || !prm_smartframes || (currentfunc
        ->value.classdata.cppflags &PF_MEMBER) && !(currentfunc
        ->value.classdata.cppflags &PF_STATIC))
    {
        gen_codes(op_link, !prm_smalldata && !prm_68020 ? 2 : 4, makeareg
            (linkreg), make_immed( - lc_maxauto));
        frame_ins = peep_tail;
    }
    else
        frame_ins = 0;
    if (mask != 0)
    {
        if (prm_coldfire)
        {
            if (rmaskcnt > 1)
            {
                mvn2 = xalloc(sizeof(struct enode));
                mvn2->nodetype = en_icon;
                mvn2->v.i = (rmaskcnt << 2);
                mvn1 = xalloc(sizeof(struct enode));
                mvn1->nodetype = en_uminus;
                mvn1->v.p[0] = mvn2;
                ap3 = xalloc(sizeof(struct amode));
                ap3->mode = am_indx;
                ap3->preg = ap3->preg = 7;
                ap3->offset = mvn1;
                gen_codes(op_lea, 0, ap3, mvma7i);
                gen_codes(op_movem, 4, make_mask(mask, 0), mvma7);
                freeop(ap3);
            }
            else
                gen_codes(op_move, 4, make_mask(mask, 0), push);
        }
        else
            gen_codes(op_movem, 4, make_mask(mask, 0), push);
    }
    save_mask = mask;
    if (fmask != 0)
        gen_codes(op_fmovem, 10, make_mask(fmask, 1), push);
    fsave_mask = fmask;
    if ((prm_phiform || currentfunc->intflag) && currentfunc->tp->lst.head &&
        currentfunc->tp->lst.head != (SYM*) - 1)
    {
        gen_codes(op_move, 4, makeareg(0), makeareg(linkreg));
    } if ((!prm_linkreg || currentfunc->intflag) && lc_maxauto)
    {
        AMODE *ap = xalloc(sizeof(AMODE));
        ap->mode = am_indx;
        ap->offset = makeintnode(en_icon,  - lc_maxauto);
        ap->preg = 7;
        gen_codes(op_lea, 0, ap, makeareg(7));
    }

    if (prm_stackcheck)
    {
        AMODE *ap1;
        ap = set_symbol("__stackerror", 1);
        ap1 = set_symbol("__stackbottom", 0);
        if (prm_datarel)
        {
            ap1->mode = am_indx;
            ap1->preg = basereg;
        }
        else
        {
            ap1->mode = am_adirect;
            if (prm_smalldata)
                ap1->preg = 2;
            else
                ap1->preg = 4;
        }
        gen_codes(op_cmp, 4, ap1, makeareg(7));
        gen_codes(op_bhi, 0, ap, 0);
    }
    AddProfilerData();
    if ((conscount || try_block_list || currentfunc->value.classdata.throwlist
        && currentfunc->value.classdata.throwlist->data) && prm_xcept)
    {
        currentfunc->value.classdata.conslabel = nextlabel++;
        currentfunc->value.classdata.destlabel = nextlabel++;
        gen_codes(op_move, 4, make_label(nextlabel - 2), makedreg(0));
        call_library2("__InitExceptBlock", 0);
        gen_label(nextlabel - 1);
    }
}

//-------------------------------------------------------------------------

void loadregs(void)
/*
 * Initailze allocated regs
 *
 */
{
    CSE *csp;
    ENODE *exptr;
    unsigned mask, i, fmask, size;
    AMODE *ap,  *ap2;
    csp = olist;
    while (csp != 0)
    {
        int sz;
        if (csp->reg !=  - 1)
        {
             /* see if preload needed */
            exptr = csp->exp;
            if (!lvalue(exptr) || ((SYM*)exptr->v.p[0]->v.p[0])->funcparm)
            {
                exptr = csp->exp;
                initstack();
                sz = csp->size;
                ap = gen_expr(exptr, FALSE, TRUE, sz);
                if (sz == 0 && ap->mode == am_immed)
                    sz = 4;
                if (csp->reg < 16)
                {
                    if (sz == 0 && ap->mode == am_immed)
                        sz = 4;
                    /* might get both a move and an and */
                    if (ap->mode == am_dreg)
                    {
                        peep_tail->oper2->preg = csp->reg;
                        if (peep_tail->opcode != op_move)
                        {
                            OCODE *peep_pos = peep_tail->back;
                            peep_pos->oper2->preg = csp->reg;
                        }
                    }
                    else
                    {
                        ap2 = makedreg(csp->reg);
                        gen_codes(op_move, sz, ap, ap2);
                        do_extend(ap2, 4, F_DREG);
                    }
                }
                else
                if (csp->reg < 32)
                {
                    if (sz == 0 && ap->mode == am_immed)
                        sz = 4;
                    if (ap->mode == am_areg)
                        peep_tail->oper2->preg = csp->reg - 16;
                    else
                    {
                        ap2 = makeareg(csp->reg - 16);
                        gen_codes(op_move, 4, ap, ap2);
                    }
                }
                else
                {
                    if (sz == 0 && ap->mode == am_immed)
                        sz = 8;
                    if (ap->mode == am_freg)
                        peep_tail->oper2->preg = csp->reg - 32;
                    else
                    {
                        ap2 = makefreg(csp->reg - 32);
                        size = 8;
                        if (exptr->nodetype == en_floatref || exptr->nodetype 
                            == en_fimaginaryref)
                            size = 7;
                        if (exptr->nodetype == en_longdoubleref || exptr
                            ->nodetype == en_lrimaginaryref)
                            size = 10;
                        gen_codes(op_fmove, size, ap, ap2);
                    }
                }
                freeop(ap);
                if (lvalue(exptr) && ((SYM*)exptr->v.p[0]->v.p[0])->funcparm)
                {
                    ((SYM*)exptr->v.p[0]->v.p[0])->inreg = TRUE;
                    ((SYM*)exptr->v.p[0]->v.p[0])->value.i =  - csp->reg - csp
                        ->size *256;
                }
            }
        }
        csp = csp->next;
    }
}

//-------------------------------------------------------------------------

int voidexpr(ENODE *node)
{
    CSE *csp;
    if (node == 0)
        return 0;
    switch (node->nodetype)
    {
        case en_structret:
            return 0;
        case en_conslabel:
        case en_destlabel:
        case en_movebyref:
            return voidexpr(node->v.p[0]);
        case en_rcon:
        case en_lrcon:
        case en_fcon:
        case en_rcomplexcon:
        case en_lrcomplexcon:
        case en_fcomplexcon:
        case en_rimaginarycon:
        case en_lrimaginarycon:
        case en_fimaginarycon:
            return 1;
        case en_icon:
        case en_lcon:
        case en_iucon:
        case en_lucon:
        case en_llcon:
        case en_llucon:
        case en_boolcon:
        case en_ccon:
        case en_cucon:
        case en_nacon:
        case en_napccon:
        case en_absacon:
        case en_autocon:
        case en_autoreg:
            return 0;
        case en_floatref:
        case en_doubleref:
        case en_longdoubleref:
        case en_fimaginaryref:
        case en_rimaginaryref:
        case en_lrimaginaryref:
        case en_fcomplexref:
        case en_rcomplexref:
        case en_lrcomplexref:
            return 1;
        case en_bool_ref:
        case en_ub_ref:
        case en_uw_ref:
        case en_b_ref:
        case en_w_ref:
        case en_l_ref:
        case en_ul_ref:
			case en_i_ref:
			case en_ui_ref:
        case en_ll_ref:
        case en_ull_ref:
			case en_a_ref: case en_ua_ref:
            return 0;
        case en_uminus:
        case en_bits:
        case en_asuminus:
        case en_ascompl:
        case en_not:
        case en_compl:
            return voidexpr(node->v.p[0]);
        case en_fcall:
        case en_pfcall:
        case en_pfcallb:
        case en_sfcall:
        case en_sfcallb:
        case en_fcallb:
            {
                SYM *sp;
                ENODE *node2 = node->v.p[1]->v.p[0]->v.p[0];
                if (node2->nodetype == en_nacon || node2->nodetype ==
                    en_napccon)
                {
                    sp = node2->v.sp;
                    if (sp && (sp->value.classdata.cppflags &PF_INLINE))
                        voidfloat(sp->value.classdata.inlinefunc->stmt);
                }
            }
        case en_cfc:
        case en_crc:
        case en_clrc:
        case en_cfi:
        case en_cri:
        case en_clri:
        case en_cb:
        case en_cub:
        case en_cbool:
        case en_cw:
        case en_cuw:
        case en_cl:
        case en_cul:
		case en_ci:
		case en_cui:
        case en_cll:
        case en_cull:
        case en_cf:
        case en_cd:
        case en_cp:
        case en_cld:
        case en_thiscall:
        case en_ainc:
        case en_adec:
        case en_add:
        case en_sub:
        case en_addstruc:
        en_addcast: case en_umul:
        case en_udiv:
        case en_umod:
        case en_mul:
        case en_div:
        case en_mod:
        case en_lsh:
        case en_asalsh:
        case en_asarsh:
        case en_alsh:
        case en_arsh:
        case en_arshd:
        case en_asarshd:
        case en_rsh:
        case en_and:
        case en_or:
        case en_xor:
        case en_land:
        case en_lor:
        case en_eq:
        case en_ne:
        case en_lt:
        case en_le:
        case en_ugt:
        case en_uge:
        case en_ult:
        case en_ule:
        case en_gt:
        case en_ge:
        case en_cond:
        case en_void:
        case en_voidnz:
        case en_dvoid:
        case en_pmul:
		case en_arrayindex:
        case en_cl_reg:
        case en_moveblock:
        case en_stackblock:
        case en_intcall:
        case en_pdiv:
        case en_repcons:
        case en_scallblock:
        case en_callblock:
        case en_pcallblock:
		case en_array:
            return voidexpr(node->v.p[0]) || voidexpr(node->v.p[1]);
        case en_substack:
        case en_trapcall:
        case en_clearblock:
		case en_loadstack:
		case en_savestack:
            return voidexpr(node->v.p[0]);
            //												return voidexpr(node->v.p[1]);
        case en_asadd:
        case en_assub:
        case en_asmul:
        case en_asdiv:
        case en_asor:
        case en_asand:
        case en_asxor:
        case en_asmod:
        case en_aslsh:
        case en_asumod:
        case en_asudiv:
        case en_asumul:
        case en_asrsh:
        case en_assign:
        en_lassign: case en_refassign:
            if (voidexpr(node->v.p[1]))
            {
                csp = searchnode(node->v.p[0]);
                if (csp)
                    csp->voidf = 1;
            }
            return voidexpr(node->v.p[0]);
        default:
            return 0;
    }

}

//-------------------------------------------------------------------------

void voidfloat(SNODE *block)
/*
 * Scan through a block and void all CSEs which do asadd, asmul, asmodiv
 * of float to int
 */
{
    while (block != 0)
    {
        switch (block->stype)
        {
            case st_tryblock:
                break;
            case st_throw:
                if (block->exp)
                    voidexpr(block->exp);
                break;
            case st_return:
            case st_expr:
                voidexpr(block->exp);
                break;
            case st_while:
            case st_do:
                voidexpr(block->exp);
                voidfloat(block->s1);
                break;
            case st_for:
                voidexpr(block->label);
                voidexpr(block->exp);
                voidfloat(block->s1);
                voidexpr(block->s2);
                break;
            case st_if:
                voidexpr(block->exp);
                voidfloat(block->s1);
                voidfloat(block->s2);
                break;
            case st_switch:
                voidexpr(block->exp);
                voidfloat(block->s1);
                break;
            case st_case:
                voidfloat(block->s1);
                break;
            case st_block:
                voidfloat(block->exp);
                break;
        }
        block = block->next;
    }
}

//-------------------------------------------------------------------------

void asm_scannode(ENODE *node)
{
    CSE *csp;
    if (node->nodetype == en_add || node->nodetype == en_addstruc)
    {
        asm_scannode(node->v.p[0]);
        asm_scannode(node->v.p[1]);
    }
    else
    {
        switch (node->nodetype)
        {
            case en_boolcon:
            case en_icon:
            case en_lcon:
            case en_iucon:
            case en_lucon:
            case en_ccon:
            case en_cucon:
                break;
            case en_napccon:
            case en_nacon:
            case en_absacon:
            case en_autocon:
            case en_autoreg:
                csp = enternode(node, 0, 0);
                csp->voidf = TRUE;
                break;
            case en_labcon:
            case en_nalabcon:
                break;
            default:
                DIAG("Invalid node in assembler line");
                break;
        }
    }
}

//-------------------------------------------------------------------------

void asm_scan1(AMODE *ap)
{
    if (!ap)
        return ;
    if (ap->mode != am_mask && ap->mode != am_fmask && ap->offset)
        asm_scannode(ap->offset);
    if (!ap->offset2)
        return ;
    asm_scannode(ap->offset2);
}

//-------------------------------------------------------------------------

void asm_scan(OCODE *cd)
{
    asm_scan1(cd->oper1);
    asm_scan1(cd->oper2);
    asm_scan1(cd->oper3);
}

//-------------------------------------------------------------------------

void asm_repnode(ENODE **node)
{
    if ((*node)->nodetype == en_add || (*node)->nodetype == en_addstruc)
    {
        asm_repnode(&(*node)->v.p[0]);
        asm_repnode(&(*node)->v.p[1]);
    }
    else
    if ((*node)->nodetype == en_autocon || (*node)->nodetype == en_autoreg)
    {
        *node = makeintnode(en_icon, (*node)->v.sp->value.i);
    }
    else if ((*node)->nodetype == en_nacon || (*node)->nodetype == en_napccon)
    {
        *node = makenode((*node)->nodetype, (*node)->v.sp, 0);
    }
    else if ((*node)->nodetype == en_nalabcon)
    {
        *node = makenode((*node)->nodetype, (*node)->v.sp, 0);
    }
    else if ((*node)->nodetype == en_labcon)
    {
        *node = makenode(en_labcon, (char*)(*node)->v.i, 0);
    }
    else if ((*node)->nodetype == en_absacon)
    {
        *node = makeintnode(en_absacon, ((SYM*)(*node)->v.p[0])->value.i);

    }
}

//-------------------------------------------------------------------------

void asm_repcse1(AMODE *ap)
{
    if (!ap)
        return ;
    if (ap->mode != am_mask && ap->mode != am_fmask && ap->offset)
        asm_repnode(&ap->offset);
    if (!ap->offset2)
        return ;
    asm_repnode(&ap->offset2);
}

//-------------------------------------------------------------------------

void asm_repcse(OCODE *cd)
{
    asm_repcse1(cd->oper1);
    asm_repcse1(cd->oper2);
    asm_repcse1(cd->oper3);
}
