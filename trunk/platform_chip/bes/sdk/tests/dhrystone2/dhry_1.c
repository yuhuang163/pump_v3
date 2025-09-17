/*       gcc dhry_1.c dhry_2.c cpuidc64.o cpuida64.o -m64 -lrt -lc -lm -o dhry2
 *************************************************************************
 *
 *                   "DHRYSTONE" Benchmark Program
 *                   -----------------------------
 *
 *  Version:    C, Version 2.1
 *
 *  File:       dhry_1.c (part 2 of 3)
 *
 *  Date:       May 25, 1988
 *
 *  Author:     Reinhold P. Weicker
 *
 *************************************************************************
 *
 *     #define options not used
 */
#include <stdlib.h>
#include "dhry.h"
#include "dhry_test.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "cmsis.h"

#ifdef CNNT
    #define options   "Non-optimised"
    #define opt "0"
#else
    #define options   "Optimised"
    #define opt ""
#endif

#define dhry_printf(str, ...)           hal_trace_printf(TR_ATTR_NO_TS|TR_ATTR_NO_ID|TR_ATTR_NO_LF, str, ##__VA_ARGS__)

/* Global Variables: */
Rec_Pointer     Ptr_Glob,
                Next_Ptr_Glob;
int             Int_Glob;
Boolean         Bool_Glob;
char            Ch_1_Glob,
                Ch_2_Glob;
int             Arr_1_Glob [50];
int             Arr_2_Glob [50] [50];

char Reg_Define[40] = "Register option      Selected.";

Enumeration Func_1(Capital_Letter Ch_1_Par_Val, Capital_Letter Ch_2_Par_Val);
/*
forward declaration necessary since Enumeration may not simply be int
*/
#ifndef ROPT
    #define REG
    /* REG becomes defined as empty */
    /* i.e. no register variables   */
#else
    #define REG register
#endif

void Proc_1(REG Rec_Pointer Ptr_Val_Par);
void Proc_2(One_Fifty *Int_Par_Ref);
void Proc_3(Rec_Pointer *Ptr_Ref_Par);
void Proc_4(void);
void Proc_5(void);
void Proc_6(Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val,
            One_Fifty *Int_Par_Ref);
void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref,
            int Int_1_Par_Val, int Int_2_Par_Val);

Boolean Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);

/* variables for time measurement: */
#define Too_Small_Time TEST_MIN_TIME_US     //us
/* Measurements should last at least 2 seconds */
uint32_t        User_Time;
double          Microseconds,
                Dhrystones_Per_Second,
                Vax_Mips;
/* end of variables for time measurement */

static Rec_Type gNext_Ptr_Glob_Buf;
static Rec_Type gPtr_Glob;

void dhrystone_main(void)
{
    /* main program, corresponds to procedures        */
    /* Main and Proc_0 in the Ada version             */
    One_Fifty   Int_1_Loc;
    REG   One_Fifty   Int_2_Loc;
    One_Fifty   Int_3_Loc;
    REG   char        Ch_Index;
    Enumeration Enum_Loc;
    Str_30      Str_1_Loc;
    Str_30      Str_2_Loc;
    REG   int         Run_Index;
    REG   int         Number_Of_Runs;
    uint32_t    start_time, end_time;
    uint32_t    lock;
    double      cpu_freq;

    /***********************************************************************
     *         Change for compiler and optimisation used                   *
     ***********************************************************************/
    Next_Ptr_Glob = (Rec_Pointer) &gNext_Ptr_Glob_Buf;
    Ptr_Glob = (Rec_Pointer) &gPtr_Glob;

    Ptr_Glob->Ptr_Comp                    = Next_Ptr_Glob;
    Ptr_Glob->Discr                       = Ident_1;
    Ptr_Glob->variant.var_1.Enum_Comp     = Ident_3;
    Ptr_Glob->variant.var_1.Int_Comp      = 40;
    strcpy(Ptr_Glob->variant.var_1.Str_Comp,
           "DHRYSTONE PROGRAM, SOME STRING");
    strcpy(Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");

    Arr_2_Glob [8][7] = 10;
    /* Was missing in published program. Without this statement,   */
    /* Arr_2_Glob [8][7] would have an undefined value.            */
    /* Warning: With 16-Bit processors and Number_Of_Runs > 32000, */
    /* overflow may occur for this array element.                  */

    Number_Of_Runs = NUMBER_OF_RUNS * dhrystone_get_freq_factor();
    if (Number_Of_Runs <= 0) {
        Number_Of_Runs = 1;
    }

    dhry_printf("\n---------------------------------------------------------------\n");
    dhry_printf("Dhrystone Benchmark, Version 2.1 (Language: C or C++)\n");
    dhry_printf("Optimisation    %s\n", options);
#ifdef ROPT
    dhry_printf("Register option selected\n\n");
#else
    dhry_printf("Register option not selected\n\n");
    strcpy(Reg_Define, "Register option  Not selected.");
#endif

    lock = int_lock();

    TRACE_FLUSH();

    /***************/
    /* Start timer */
    /***************/
    start_time = hal_fast_sys_timer_get();

    dhrystone_event_begin();

    for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index) {
        Proc_5();
        Proc_4();
        /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
        Int_1_Loc = 2;
        Int_2_Loc = 3;
        strcpy(Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
        Enum_Loc = Ident_2;
        Bool_Glob = ! Func_2(Str_1_Loc, Str_2_Loc);
        /* Bool_Glob == 1 */
        while (Int_1_Loc < Int_2_Loc) { /* loop body executed once */
            Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
            /* Int_3_Loc == 7 */
            Proc_7(Int_1_Loc, Int_2_Loc, &Int_3_Loc);
            /* Int_3_Loc == 7 */
            Int_1_Loc += 1;
        }   /* while */
        /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
        Proc_8(Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
        /* Int_Glob == 5 */
        Proc_1(Ptr_Glob);
        for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index) {
            /* loop body executed twice */
            if (Enum_Loc == Func_1(Ch_Index, 'C')) {
                /* then, not executed */
                Proc_6(Ident_1, &Enum_Loc);
                strcpy(Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
                Int_2_Loc = Run_Index;
                Int_Glob = Run_Index;
            }
        }
        /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
        Int_2_Loc = Int_2_Loc * Int_1_Loc;
        Int_1_Loc = Int_2_Loc / Int_3_Loc;
        Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
        /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
        Proc_2(&Int_1_Loc);
        /* Int_1_Loc == 5 */
    }   /* loop "for Run_Index" */

    /**************/
    /* Stop timer */
    /**************/
    dhrystone_event_end();

    end_time = hal_fast_sys_timer_get();

    int_unlock(lock);

    User_Time = FAST_TICKS_TO_US_SAFE(end_time - start_time);
    dhry_printf("%8u runs, total time %8u us\n", Number_Of_Runs, User_Time);

    dhry_printf("\n");
    dhry_printf("Final values (* implementation-dependent):\n");
    dhry_printf("Int_Glob     : ");
    if (Int_Glob == 5)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  ", Int_Glob);

    dhry_printf("\nBool_Glob    : ");
    if (Bool_Glob == 1)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d\n", Bool_Glob);

    dhry_printf("Ch_1_Glob    : ");
    if (Ch_1_Glob == 'A')
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%c  ", Ch_1_Glob);

    dhry_printf("\nCh_2_Glob    : ");
    if (Ch_2_Glob == 'B')
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%c\n",  Ch_2_Glob);

    dhry_printf("Arr_1_Glob[8]: ");
    if (Arr_1_Glob[8] == 7)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  ", Arr_1_Glob[8]);

    dhry_printf("\nArr_2_Glob8/7: ");
    if (Arr_2_Glob[8][7] == Number_Of_Runs + 10)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d\n", Arr_2_Glob[8][7]);

    dhry_printf("Ptr_Glob->\n");
    dhry_printf("  Ptr_Comp   :  *    %d\n", (int) Ptr_Glob->Ptr_Comp);

    dhry_printf("  Discr      : ");
    if (Ptr_Glob->Discr == 0)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  ", Ptr_Glob->Discr);

    dhry_printf("\n  Enum_Comp  : ");
    if (Ptr_Glob->variant.var_1.Enum_Comp == 2)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d\n", Ptr_Glob->variant.var_1.Enum_Comp);

    dhry_printf("  Int_Comp   : ");
    if (Ptr_Glob->variant.var_1.Int_Comp == 17)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d ", Ptr_Glob->variant.var_1.Int_Comp);

    dhry_printf("\n  Str_Comp   : ");
    if (strcmp(Ptr_Glob->variant.var_1.Str_Comp,
            "DHRYSTONE PROGRAM, SOME STRING") == 0)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%s\n", Ptr_Glob->variant.var_1.Str_Comp);

    dhry_printf("Next_Ptr_Glob->\n");
    dhry_printf("  Ptr_Comp   :  *    %d", (int) Next_Ptr_Glob->Ptr_Comp);
    dhry_printf(", same as above\n");

    dhry_printf("  Discr      : ");
    if (Next_Ptr_Glob->Discr == 0)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  ", Next_Ptr_Glob->Discr);

    dhry_printf("\n  Enum_Comp  : ");
    if (Next_Ptr_Glob->variant.var_1.Enum_Comp == 1)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d\n", Next_Ptr_Glob->variant.var_1.Enum_Comp);

    dhry_printf("  Int_Comp   : ");
    if (Next_Ptr_Glob->variant.var_1.Int_Comp == 18)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d ", Next_Ptr_Glob->variant.var_1.Int_Comp);

    dhry_printf("\n  Str_Comp   : ");
    if (strcmp(Next_Ptr_Glob->variant.var_1.Str_Comp,
            "DHRYSTONE PROGRAM, SOME STRING") == 0)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%s\n", Next_Ptr_Glob->variant.var_1.Str_Comp);

    dhry_printf("Int_1_Loc    : ");
    if (Int_1_Loc == 5)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  ", Int_1_Loc);

    dhry_printf("\nInt_2_Loc    : ");
    if (Int_2_Loc == 13)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d\n", Int_2_Loc);

    dhry_printf("Int_3_Loc    : ");
    if (Int_3_Loc == 7)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  ", Int_3_Loc);

    dhry_printf("\nEnum_Loc     : ");
    if (Enum_Loc == 1)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%d  \n", Enum_Loc);

    dhry_printf("Str_1_Loc    : ");
    if (strcmp(Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING") == 0)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%s\n", Str_1_Loc);

    dhry_printf("Str_2_Loc    : ");
    if (strcmp(Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING") == 0)
        dhry_printf("O.K.  ");
    else
        dhry_printf("WRONG ");
    dhry_printf("%s\n", Str_2_Loc);
    dhry_printf("\n");

    ASSERT(User_Time >= Too_Small_Time, "Number_Of_Runs %u is too small, running time (%u us) < min time (%u us)\n", Number_Of_Runs, User_Time, Too_Small_Time);

    Microseconds = (double)User_Time / (double)Number_Of_Runs;
    Dhrystones_Per_Second = (double)Number_Of_Runs / (User_Time / (double)1000000.0);
    Vax_Mips = (double)Dhrystones_Per_Second / (double)1757.0;
    cpu_freq = (double)(hal_sys_timer_calc_cpu_freq(5, 0) / (double)1000000.0);

#ifdef NOSTD
    dhry_printf("CPU Freq               :%u MHz\n", (unsigned)cpu_freq);
    dhry_printf("Run Dhrystone once     :%u us\n", (unsigned)Microseconds);
    dhry_printf("Dhrystones per Second  :%u.%03u\n", (unsigned)Dhrystones_Per_Second, (unsigned)((Dhrystones_Per_Second - (unsigned)Dhrystones_Per_Second) * 1000));
    dhry_printf("VAX MIPS rating(DMIPS) :%u.%03u\n", (unsigned)Vax_Mips, (unsigned)((Vax_Mips - (unsigned)Vax_Mips) * 1000));
    double mips_per_mhz = Vax_Mips / cpu_freq;
    dhry_printf("DMIPS/MHz              :%u.%03u\n", (unsigned)mips_per_mhz, (unsigned)((mips_per_mhz - (unsigned)mips_per_mhz) * 1000));
#else
    dhry_printf("CPU Freq               :%0.3f MHz\n", cpu_freq);
    dhry_printf("Run Dhrystone once     :%0.3f us\n", Microseconds);
    dhry_printf("Dhrystones per Second  :%0.3f\n", Dhrystones_Per_Second);
    dhry_printf("VAX MIPS rating(DMIPS) :%0.3f\n", Vax_Mips);
    dhry_printf("DMIPS/MHz              :%0.3f\n", Vax_Mips / cpu_freq);
#endif
    dhry_printf("---------------------------------------------------------------\n\n");
}

void Proc_1(REG Rec_Pointer Ptr_Val_Par)
/* executed once */
{
    REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;
    /* == Ptr_Glob_Next */
    /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
    /* corresponds to "rename" in Ada, "with" in Pascal           */

    structassign(*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
    Ptr_Val_Par->variant.var_1.Int_Comp = 5;
    Next_Record->variant.var_1.Int_Comp
        = Ptr_Val_Par->variant.var_1.Int_Comp;
    Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
    Proc_3(&Next_Record->Ptr_Comp);
    /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp
                        == Ptr_Glob->Ptr_Comp */
    if (Next_Record->Discr == Ident_1) {
        /* then, executed */
        Next_Record->variant.var_1.Int_Comp = 6;
        Proc_6(Ptr_Val_Par->variant.var_1.Enum_Comp,
               &Next_Record->variant.var_1.Enum_Comp);
        Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
        Proc_7(Next_Record->variant.var_1.Int_Comp, 10,
               &Next_Record->variant.var_1.Int_Comp);
    } else /* not executed */
        structassign(*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
} /* Proc_1 */

void Proc_2(One_Fifty *Int_Par_Ref)
/* executed once */
/* *Int_Par_Ref == 1, becomes 4 */
{
    One_Fifty  Int_Loc;
    Enumeration   Enum_Loc;

    Int_Loc = *Int_Par_Ref + 10;
    do {/* executed once */
        if (Ch_1_Glob == 'A') {
            /* then, executed */
            Int_Loc -= 1;
            *Int_Par_Ref = Int_Loc - Int_Glob;
            Enum_Loc = Ident_1;
        } /* if */
    } while (Enum_Loc != Ident_1); /* true */
} /* Proc_2 */

void Proc_3(Rec_Pointer *Ptr_Ref_Par)
/* executed once */
/* Ptr_Ref_Par becomes Ptr_Glob */
{
    if (Ptr_Glob != Null)
        /* then, executed */
        *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
    Proc_7(10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
} /* Proc_3 */

void Proc_4(void)  /* without parameters */
/* executed once */
{
    Boolean Bool_Loc;

    Bool_Loc = Ch_1_Glob == 'A';
    Bool_Glob = Bool_Loc | Bool_Glob;
    Ch_2_Glob = 'B';
} /* Proc_4 */

void Proc_5(void)  /* without parameters */
/* executed once */
{
    Ch_1_Glob = 'A';
    Bool_Glob = false;
} /* Proc_5 */

/* Procedure for the assignment of structures,          */
/* if the C compiler doesn't support this feature       */
#ifdef  NOSTRUCTASSIGN
memcpy(d, s, l)
register char   *d;
register char   *s;
register int    l;
{
    while (l--)
        *d++ = *s++;
}
#endif

