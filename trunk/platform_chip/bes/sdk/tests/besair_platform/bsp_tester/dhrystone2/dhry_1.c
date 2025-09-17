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
 #include <stdio.h>
 #include "dhry.h"
 #include "hal_trace.h"
 #include "hal_timer.h"

 #ifdef CNNT
    #define options   "Non-optimised"
    #define opt "0"
 #else
//    #define options   "Optimised"
    #define options   "Opt 3 64 Bit"
    #define opt ""
 #endif
 

/* Global Variables: */
 
 Rec_Pointer    DHRYT_DATA_LOC Ptr_Glob,
                               Next_Ptr_Glob;
 int            DHRYT_DATA_LOC Int_Glob;
 Boolean        DHRYT_DATA_LOC Bool_Glob;
 char           DHRYT_DATA_LOC Ch_1_Glob,
                               Ch_2_Glob;
 int            DHRYT_DATA_LOC Arr_1_Glob [50];
 int            DHRYT_DATA_LOC Arr_2_Glob [50] [50];
 
 char           DHRYT_DATA_LOC Reg_Define[40] = "Register option      Selected.";
 

 Enumeration Func_1 (Capital_Letter Ch_1_Par_Val,
                                           Capital_Letter Ch_2_Par_Val);
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

 void Proc_1 (REG Rec_Pointer Ptr_Val_Par);
 void Proc_2 (One_Fifty *Int_Par_Ref);
 void Proc_3 (Rec_Pointer *Ptr_Ref_Par);
 void Proc_4 (); 
 void Proc_5 ();
 void Proc_6 (Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
 void Proc_7 (One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val,
                                              One_Fifty *Int_Par_Ref);
 void Proc_8 (Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref,
                               int Int_1_Par_Val, int Int_2_Par_Val);
                               
 Boolean Func_2 (Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);

 
 /* variables for time measurement: */
 
 #define Too_Small_Time 2000000
                 /* Measurements should last at least 2 seconds */
 
 uint32_t        DHRYT_DATA_LOC User_Time;
 
 double          DHRYT_DATA_LOC Microseconds,
                                Dhrystones_Per_Second,
                                Vax_Mips;
 
 /* end of variables for time measurement */
 
 Rec_Type        DHRYT_DATA_LOC gNext_Ptr_Glob_Buf;
 Rec_Type        DHRYT_DATA_LOC gPtr_Glob;
 
 void DHRYT_FUNC_LOC dhry_main (int argc, char *argv[])
 /*****/
 
   /* main program, corresponds to procedures        */
   /* Main and Proc_0 in the Ada version             */
 {
 
         One_Fifty   Int_1_Loc;
   REG   One_Fifty   Int_2_Loc;
         One_Fifty   Int_3_Loc;
   REG   char        Ch_Index;
         Enumeration Enum_Loc;
         Str_30      Str_1_Loc;
         Str_30      Str_2_Loc;
   REG   int         Run_Index;
   REG   int         Number_Of_Runs; 
         int         count = 10;
         uint32_t    start_time_ms = 0, end_time_ms = 0;
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
   strcpy (Ptr_Glob->variant.var_1.Str_Comp, 
           "DHRYSTONE PROGRAM, SOME STRING");       
   strcpy (Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING");
 
   Arr_2_Glob [8][7] = 10;
         /* Was missing in published program. Without this statement,   */
         /* Arr_2_Glob [8][7] would have an undefined value.            */
         /* Warning: With 16-Bit processors and Number_Of_Runs > 32000, */
         /* overflow may occur for this array element.                  */
 
 
    TRACE_IMM(0, "##########################################\n"); 

   TRACE_IMM(0, "\n");
   TRACE_IMM(0, "Dhrystone Benchmark, Version 2.1 (Language: C or C++)\n");
   TRACE_IMM(0, "\n");
   
   TRACE_IMM(0, "Optimisation    %s\n", options);
   #ifdef ROPT
       TRACE_IMM(0, "Register option selected\n\n");
   #else
       TRACE_IMM(0, "Register option not selected\n\n");
       strcpy(Reg_Define, "Register option  Not selected.");
   #endif

   Number_Of_Runs = 2000;

   do
   {

       Number_Of_Runs = Number_Of_Runs * 2;
       count = count - 1;
       Arr_2_Glob [8][7] = 10;
        
       /***************/
       /* Start timer */
       /***************/
  
       start_time_ms = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
       for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
       {
 
         Proc_5();
         Proc_4();
           /* Ch_1_Glob == 'A', Ch_2_Glob == 'B', Bool_Glob == true */
         Int_1_Loc = 2;
         Int_2_Loc = 3;
         strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING");
         Enum_Loc = Ident_2;
         Bool_Glob = ! Func_2 (Str_1_Loc, Str_2_Loc);
           /* Bool_Glob == 1 */
         while (Int_1_Loc < Int_2_Loc)  /* loop body executed once */
         {
           Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
             /* Int_3_Loc == 7 */
           Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
             /* Int_3_Loc == 7 */
           Int_1_Loc += 1;
         }   /* while */
            /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
         Proc_8 (Arr_1_Glob, Arr_2_Glob, Int_1_Loc, Int_3_Loc);
           /* Int_Glob == 5 */
         Proc_1 (Ptr_Glob);
         for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index)
                              /* loop body executed twice */
         {
           if (Enum_Loc == Func_1 (Ch_Index, 'C'))
               /* then, not executed */
             {
               Proc_6 (Ident_1, &Enum_Loc);
               strcpy (Str_2_Loc, "DHRYSTONE PROGRAM, 3'RD STRING");
               Int_2_Loc = Run_Index;
               Int_Glob = Run_Index;
             }
         }
           /* Int_1_Loc == 3, Int_2_Loc == 3, Int_3_Loc == 7 */
         Int_2_Loc = Int_2_Loc * Int_1_Loc;
         Int_1_Loc = Int_2_Loc / Int_3_Loc;
         Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc;
           /* Int_1_Loc == 1, Int_2_Loc == 13, Int_3_Loc == 7 */
         Proc_2 (&Int_1_Loc);
           /* Int_1_Loc == 5 */
 
       }   /* loop "for Run_Index" */
 
       /**************/
       /* Stop timer */
       /**************/
 
       end_time_ms = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
       User_Time = end_time_ms - start_time_ms;
       //TRACE(0, "%d runs %dus\n", Number_Of_Runs, User_Time);
       if (User_Time > 2000000)
       {
             count = 0;
             TRACE(0, "Test end %d runs %dus\n", Number_Of_Runs, User_Time);
       }
       else
       {
             if (User_Time < 50000)
             {
                  Number_Of_Runs = Number_Of_Runs * 5;
                  TRACE(0, "Number_Of_Runs = Number_Of_Runs * 5");
             }
       }
   }   /* calibrate/run do while */
   while (count >0);
 
   TRACE_IMM(0, "\n");
   TRACE_IMM(0, "Final values (* implementation-dependent):\n");
   TRACE_IMM(0, "\n");
   TRACE_IMM(0, "Int_Glob:      ");
   if (Int_Glob == 5)  TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  ", Int_Glob);
      
   TRACE_IMM(0, "Bool_Glob:     ");
   if (Bool_Glob == 1) TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d\n", Bool_Glob);
      
   TRACE_IMM(0, "Ch_1_Glob:     ");
   if (Ch_1_Glob == 'A')  TRACE_IMM(0, "O.K.  ");               
   else                   TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%c  ", Ch_1_Glob);
         
   TRACE_IMM(0, "Ch_2_Glob:     ");
   if (Ch_2_Glob == 'B')  TRACE_IMM(0, "O.K.  ");
   else                   TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%c\n",  Ch_2_Glob);
   
   TRACE_IMM(0, "Arr_1_Glob[8]: ");
   if (Arr_1_Glob[8] == 7)  TRACE_IMM(0, "O.K.  ");
   else                     TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  ", Arr_1_Glob[8]);
            
   TRACE_IMM(0, "Arr_2_Glob8/7: ");
   if (Arr_2_Glob[8][7] == Number_Of_Runs + 10)
                          TRACE_IMM(0, "O.K.  ");
   else                   TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%10d\n", Arr_2_Glob[8][7]);
   
   TRACE_IMM(0, "Ptr_Glob->            ");
   TRACE_IMM(0, "  Ptr_Comp:       *    %d\n", (int) Ptr_Glob->Ptr_Comp);
   
   TRACE_IMM(0, "  Discr:       ");
   if (Ptr_Glob->Discr == 0)  TRACE_IMM(0, "O.K.  ");
   else                       TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  ", Ptr_Glob->Discr);
            
   TRACE_IMM(0, "Enum_Comp:     ");
   if (Ptr_Glob->variant.var_1.Enum_Comp == 2)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d\n", Ptr_Glob->variant.var_1.Enum_Comp);
      
   TRACE_IMM(0, "  Int_Comp:    ");
   if (Ptr_Glob->variant.var_1.Int_Comp == 17)  TRACE_IMM(0, "O.K.  ");
   else                                         TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d ", Ptr_Glob->variant.var_1.Int_Comp);
      
   TRACE_IMM(0, "Str_Comp:      ");
   if (strcmp(Ptr_Glob->variant.var_1.Str_Comp,
                        "DHRYSTONE PROGRAM, SOME STRING") == 0)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");   
   TRACE_IMM(0, "%s\n", Ptr_Glob->variant.var_1.Str_Comp);
   
   TRACE_IMM(0, "Next_Ptr_Glob->       "); 
   TRACE_IMM(0, "  Ptr_Comp:       *    %d", (int) Next_Ptr_Glob->Ptr_Comp);
   TRACE_IMM(0, " same as above\n");
   
   TRACE_IMM(0, "  Discr:       ");
   if (Next_Ptr_Glob->Discr == 0)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  ", Next_Ptr_Glob->Discr);
   
   TRACE_IMM(0, "Enum_Comp:     ");
   if (Next_Ptr_Glob->variant.var_1.Enum_Comp == 1)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d\n", Next_Ptr_Glob->variant.var_1.Enum_Comp);
   
   TRACE_IMM(0, "  Int_Comp:    ");
   if (Next_Ptr_Glob->variant.var_1.Int_Comp == 18)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d ", Next_Ptr_Glob->variant.var_1.Int_Comp);
   
   TRACE_IMM(0, "Str_Comp:      ");
   if (strcmp(Next_Ptr_Glob->variant.var_1.Str_Comp,
                        "DHRYSTONE PROGRAM, SOME STRING") == 0)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");   
   TRACE_IMM(0, "%s\n", Next_Ptr_Glob->variant.var_1.Str_Comp);
   
   TRACE_IMM(0, "Int_1_Loc:     ");
   if (Int_1_Loc == 5)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  ", Int_1_Loc);
      
   TRACE_IMM(0, "Int_2_Loc:     ");
   if (Int_2_Loc == 13)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d\n", Int_2_Loc);
   
   TRACE_IMM(0, "Int_3_Loc:     ");
   if (Int_3_Loc == 7)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  ", Int_3_Loc);
   
   TRACE_IMM(0, "Enum_Loc:      ");
   if (Enum_Loc == 1)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");
   TRACE_IMM(0, "%d  \n", Enum_Loc);
   
   
   TRACE_IMM(0, "Str_1_Loc:                             ");
   if (strcmp(Str_1_Loc, "DHRYSTONE PROGRAM, 1'ST STRING") == 0)
                        TRACE_IMM(0, "O.K.  ");

   else                TRACE_IMM(0, "WRONG ");   
   TRACE_IMM(0, "%s\n", Str_1_Loc);

   TRACE_IMM(0, "Str_2_Loc:                             ");
   if (strcmp(Str_2_Loc, "DHRYSTONE PROGRAM, 2'ND STRING") == 0)
                        TRACE_IMM(0, "O.K.  ");
   else                TRACE_IMM(0, "WRONG ");   
   TRACE_IMM(0, "%s\n", Str_2_Loc);
         

   TRACE_IMM(0, "\n");
    
 
   if (User_Time < Too_Small_Time)
   {
     TRACE_IMM(0, "Measured time too small to obtain meaningful results\n");
     TRACE_IMM(0, "Please increase number of runs\n");
     TRACE_IMM(0, "\n");
   }
   else
   {
     uint32_t a,b;
     double dmips_hz;
     Microseconds = (double)User_Time / (double) Number_Of_Runs;
     Dhrystones_Per_Second = (double) Number_Of_Runs / ((double)User_Time / (double)(1000000.0));
     Vax_Mips = Dhrystones_Per_Second / (double)1757.0;
     cpu_freq = (double)hal_sys_timer_calc_cpu_freq(10, 1);
     TRACE_IMM(0, "Microseconds for one run through Dhrystone: ");
     TRACE_IMM(0, "%d \n", (uint32_t)Microseconds);
     TRACE_IMM(0, "Dhrystones per Second:                      ");
     TRACE_IMM(0, "%d \n", (uint32_t)Dhrystones_Per_Second);
     TRACE_IMM(0, "VAX  MIPS rating =                          ");
     a = (uint32_t)Vax_Mips;
     b = (uint32_t)((Vax_Mips * (double)1000) - ((double)a * (double)1000));
     TRACE_IMM(0, "%d.%03d \n",a,b);

     dmips_hz = Vax_Mips/(cpu_freq/(double)(1000000.0));
     a = (uint32_t)dmips_hz;
     b = (uint32_t)((dmips_hz * (double)1000) - ((double)a * (double)1000));
     TRACE_IMM(0, "VAX  DMIPS rating =                          ");
     TRACE_IMM(0, "%d.%03d \n",a,b);
     TRACE_IMM(0, "SYS FREQ =                          ");
     TRACE_IMM(0, "%d hz \n", (uint32_t)cpu_freq);
     TRACE_IMM(0, "\n");
   }
 }
 
 void DHRYT_FUNC_LOC Proc_1 (REG Rec_Pointer Ptr_Val_Par)
 /******************/
 
     /* executed once */
 {
   REG Rec_Pointer Next_Record = Ptr_Val_Par->Ptr_Comp;  
                                         /* == Ptr_Glob_Next */
   /* Local variable, initialized with Ptr_Val_Par->Ptr_Comp,    */
   /* corresponds to "rename" in Ada, "with" in Pascal           */
   
   structassign (*Ptr_Val_Par->Ptr_Comp, *Ptr_Glob);
   Ptr_Val_Par->variant.var_1.Int_Comp = 5;
   Next_Record->variant.var_1.Int_Comp 
         = Ptr_Val_Par->variant.var_1.Int_Comp;
   Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
   Proc_3 (&Next_Record->Ptr_Comp);
     /* Ptr_Val_Par->Ptr_Comp->Ptr_Comp 
                         == Ptr_Glob->Ptr_Comp */
   if (Next_Record->Discr == Ident_1)
     /* then, executed */
   {
     Next_Record->variant.var_1.Int_Comp = 6;
     Proc_6 (Ptr_Val_Par->variant.var_1.Enum_Comp, 
            &Next_Record->variant.var_1.Enum_Comp);
     Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
     Proc_7 (Next_Record->variant.var_1.Int_Comp, 10, 
            &Next_Record->variant.var_1.Int_Comp);
   }
   else /* not executed */
     structassign (*Ptr_Val_Par, *Ptr_Val_Par->Ptr_Comp);
 } /* Proc_1 */
 
 
 void DHRYT_FUNC_LOC Proc_2 (One_Fifty *Int_Par_Ref)
 /******************/
     /* executed once */
     /* *Int_Par_Ref == 1, becomes 4 */
 
 {
   One_Fifty  Int_Loc;
   Enumeration   Enum_Loc;
 
   Int_Loc = *Int_Par_Ref + 10;
   do /* executed once */
     if (Ch_1_Glob == 'A')
       /* then, executed */
     {
       Int_Loc -= 1;
       *Int_Par_Ref = Int_Loc - Int_Glob;
       Enum_Loc = Ident_1;
     } /* if */
   while (Enum_Loc != Ident_1); /* true */
 } /* Proc_2 */
 
 
 void DHRYT_FUNC_LOC Proc_3 (Rec_Pointer *Ptr_Ref_Par)
 /******************/
     /* executed once */
     /* Ptr_Ref_Par becomes Ptr_Glob */
 
 {
   if (Ptr_Glob != Null)
     /* then, executed */
     *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
   Proc_7 (10, Int_Glob, &Ptr_Glob->variant.var_1.Int_Comp);
 } /* Proc_3 */
 
 
void DHRYT_FUNC_LOC Proc_4 () /* without parameters */
 /*******/
     /* executed once */
 {
   Boolean Bool_Loc;
 
   Bool_Loc = Ch_1_Glob == 'A';
   Bool_Glob = Bool_Loc | Bool_Glob;
   Ch_2_Glob = 'B';
 } /* Proc_4 */
 
 
 void DHRYT_FUNC_LOC Proc_5 () /* without parameters */
 /*******/
     /* executed once */
 {
   Ch_1_Glob = 'A';
   Bool_Glob = false;
 } /* Proc_5 */
 
 
         /* Procedure for the assignment of structures,          */
         /* if the C compiler doesn't support this feature       */
 #ifdef  NOSTRUCTASSIGN
 DHRYT_FUNC_LOC memcpy (d, s, l)
 register char   *d;
 register char   *s;
 register int    l;
 {
         while (l--) *d++ = *s++;
 }
 #endif



