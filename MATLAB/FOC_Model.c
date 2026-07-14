/*
 * File: FOC_Model.c
 *
 * Code generated for Simulink model 'FOC_Model'.
 *
 * Model version                  : 1.269
 * Simulink Coder version         : 9.6 (R2021b) 14-May-2021
 * C/C++ source code generated on : Tue Jul 22 15:37:44 2025
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "FOC_Model.h"
#include "tim.h"

/* Named constants for Chart: '<S2>/Chart1' */
#define IN_AlignStage                  ((uint8_T)1U)
#define IN_IDLE                        ((uint8_T)2U)
#define IN_OpenStage                   ((uint8_T)3U)
#define IN_RunStage                    ((uint8_T)4U)
#define IN_ThetaAlign                  ((uint8_T)5U)
#define NumBitsPerChar                 8U
#ifndef UCHAR_MAX
#include <limits.h>
#endif

#if ( UCHAR_MAX != (0xFFU) ) || ( SCHAR_MAX != (0x7F) )
#error Code was generated for compiler with different sized uchar/char. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( USHRT_MAX != (0xFFFFU) ) || ( SHRT_MAX != (0x7FFF) )
#error Code was generated for compiler with different sized ushort/short. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( UINT_MAX != (0xFFFFFFFFU) ) || ( INT_MAX != (0x7FFFFFFF) )
#error Code was generated for compiler with different sized uint/int. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

#if ( ULONG_MAX != (0xFFFFFFFFU) ) || ( LONG_MAX != (0x7FFFFFFF) )
#error Code was generated for compiler with different sized ulong/long. \
Consider adjusting Test hardware word size settings on the \
Hardware Implementation pane to match your compiler word sizes as \
defined in limits.h of the compiler. Alternatively, you can \
select the Test hardware is the same as production hardware option and \
select the Enable portable word sizes option on the Code Generation > \
Verification pane for ERT based targets, which will disable the \
preprocessor word size checks.
#endif

/* Skipping ulong_long/long_long check: insufficient preprocessor integer range. */

/* Exported block signals */
real32_T ialpha;                       /* '<S6>/Add1' */
real32_T ibeta;                        /* '<S6>/Gain2' */
real32_T ualpha;                       /* '<S4>/Add' */
real32_T ubeta;                        /* '<S4>/Add1' */
real32_T FluxTheta;                    /* '<S18>/Mod' */
real32_T FluxWe;                       /* '<S7>/Gain1' */
real32_T state;                        /* '<S2>/Chart1' */

/* Global sensor/sensorless selector: 0=observer(sensorless), 1=HALL */
uint8_t Hal_State = 0;

/* Exported data definition */

/* Definition for custom storage class: Struct */
motor_type motor = {
  /* L */
  0.00535F,

  /* Pn */
  2.0F,

  /* Rs */
  6.97F,

  /* flux */
  0.016884F
};

rt_Simulink_Struct_type rt_Simulink_Struct = {
  /* CurrKi */
  35.0F,

  /* CurrKp */
  0.017F,

  /* Gamma */
  100000.0F,

  /* LPFFilter */
  0.3F,

  /* PLLKi */
  640000.0F,

  /* PLLKp */
  1131.2F,

  /* spd_ki */
  0.000144F,

  /* spd_kp */
  0.003389F
};

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
extern real32_T rt_modf_snf(real32_T u0, real32_T u1);
static void SVPWM(real32_T rtu_Valpha, real32_T rtu_Vbeta, real32_T rtu_v_bus,
                  real32_T rty_tABC[3]);
static void rate_scheduler(void);
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);
extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
static void rt_InitInfAndNaN(size_t realSize);
static boolean_T rtIsInf(real_T value);
static boolean_T rtIsInfF(real32_T value);
static boolean_T rtIsNaN(real_T value);
static boolean_T rtIsNaNF(real32_T value);
typedef struct {
  struct {
    uint32_T wordH;
    uint32_T wordL;
  } words;
} BigEndianIEEEDouble;

typedef struct {
  struct {
    uint32_T wordL;
    uint32_T wordH;
  } words;
} LittleEndianIEEEDouble;

typedef struct {
  union {
    real32_T wordLreal;
    uint32_T wordLuint;
  } wordL;
} IEEESingle;

real_T rtInf;
real_T rtMinusInf;
real_T rtNaN;
real32_T rtInfF;
real32_T rtMinusInfF;
real32_T rtNaNF;
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);

/*
 * Initialize rtNaN needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetNaN(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T nan = 0.0;
  if (bitsPerReal == 32U) {
    nan = rtGetNaNF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF80000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    nan = tmpVal.fltVal;
  }

  return nan;
}

/*
 * Initialize rtNaNF needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetNaNF(void)
{
  IEEESingle nanF = { { 0.0F } };

  nanF.wordL.wordLuint = 0xFFC00000U;
  return nanF.wordL.wordLreal;
}

/*
 * Initialize the rtInf, rtMinusInf, and rtNaN needed by the
 * generated code. NaN is initialized as non-signaling. Assumes IEEE.
 */
static void rt_InitInfAndNaN(size_t realSize)
{
  (void) (realSize);
  rtNaN = rtGetNaN();
  rtNaNF = rtGetNaNF();
  rtInf = rtGetInf();
  rtInfF = rtGetInfF();
  rtMinusInf = rtGetMinusInf();
  rtMinusInfF = rtGetMinusInfF();
}

/* Test if value is infinite */
static boolean_T rtIsInf(real_T value)
{
  return (boolean_T)((value==rtInf || value==rtMinusInf) ? 1U : 0U);
}

/* Test if single-precision value is infinite */
static boolean_T rtIsInfF(real32_T value)
{
  return (boolean_T)(((value)==rtInfF || (value)==rtMinusInfF) ? 1U : 0U);
}

/* Test if value is not a number */
static boolean_T rtIsNaN(real_T value)
{
  boolean_T result = (boolean_T) 0;
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  if (bitsPerReal == 32U) {
    result = rtIsNaNF((real32_T)value);
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.fltVal = value;
    result = (boolean_T)((tmpVal.bitVal.words.wordH & 0x7FF00000) == 0x7FF00000 &&
                         ( (tmpVal.bitVal.words.wordH & 0x000FFFFF) != 0 ||
                          (tmpVal.bitVal.words.wordL != 0) ));
  }

  return result;
}

/* Test if single-precision value is not a number */
static boolean_T rtIsNaNF(real32_T value)
{
  IEEESingle tmp;
  tmp.wordL.wordLreal = value;
  return (boolean_T)( (tmp.wordL.wordLuint & 0x7F800000) == 0x7F800000 &&
                     (tmp.wordL.wordLuint & 0x007FFFFF) != 0 );
}

/*
 * Initialize rtInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T inf = 0.0;
  if (bitsPerReal == 32U) {
    inf = rtGetInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0x7FF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    inf = tmpVal.fltVal;
  }

  return inf;
}

/*
 * Initialize rtInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetInfF(void)
{
  IEEESingle infF;
  infF.wordL.wordLuint = 0x7F800000U;
  return infF.wordL.wordLreal;
}

/*
 * Initialize rtMinusInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetMinusInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T minf = 0.0;
  if (bitsPerReal == 32U) {
    minf = rtGetMinusInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    minf = tmpVal.fltVal;
  }

  return minf;
}

/*
 * Initialize rtMinusInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetMinusInfF(void)
{
  IEEESingle minfF;
  minfF.wordL.wordLuint = 0xFF800000U;
  return minfF.wordL.wordLreal;
}

/*
 *         This function updates active task flag for each subrate.
 *         The function is called at model base rate, hence the
 *         generated code self-manages all its subrates.
 */
static void rate_scheduler(void)
{
  /* Compute which subrates run during the next base time step.  Subrates
   * are an integer multiple of the base rate counter.  Therefore, the subtask
   * counter is reset when it reaches its limit (zero means run).
   */
  (rtM->Timing.TaskCounters.TID[1])++;
  if ((rtM->Timing.TaskCounters.TID[1]) > 9) {/* Sample time: [0.001s, 0.0s] */
    rtM->Timing.TaskCounters.TID[1] = 0;
  }
}

/* Output and update for atomic system: '<S2>/SVPWM' */
static void SVPWM(real32_T rtu_Valpha, real32_T rtu_Vbeta, real32_T rtu_v_bus,
                  real32_T rty_tABC[3])
{
  real32_T rtb_Min;
  real32_T rtb_Sum1_l;
  real32_T rtb_Sum_p;

  /* Gain: '<S74>/Gain' */
  rtb_Min = -0.5F * rtu_Valpha;

  /* Gain: '<S74>/Gain1' */
  rtb_Sum1_l = 0.866025388F * rtu_Vbeta;

  /* Sum: '<S74>/Sum' */
  rtb_Sum_p = rtb_Min + rtb_Sum1_l;

  /* Sum: '<S74>/Sum1' */
  rtb_Sum1_l = rtb_Min - rtb_Sum1_l;

  /* Gain: '<S75>/Gain' incorporates:
   *  MinMax: '<S75>/Min'
   *  MinMax: '<S75>/Min1'
   *  Sum: '<S75>/Sum'
   */
  rtb_Min = (fminf(fminf(rtu_Valpha, rtb_Sum_p), rtb_Sum1_l) + fmaxf(fmaxf
              (rtu_Valpha, rtb_Sum_p), rtb_Sum1_l)) * -0.5F;

  /* Gain: '<S14>/PWM_HalfPeriod' incorporates:
   *  Constant: '<S14>/Constant'
   *  Gain: '<S14>/Gain'
   *  Product: '<S14>/Divide'
   *  Sum: '<S14>/Sum'
   *  Sum: '<S14>/Sum1'
   */
  rty_tABC[0] = (-(rtb_Min + rtu_Valpha) / rtu_v_bus + 0.5F) * 8500.0F;
  rty_tABC[1] = (-(rtb_Min + rtb_Sum_p) / rtu_v_bus + 0.5F) * 8500.0F;
  rty_tABC[2] = (-(rtb_Min + rtb_Sum1_l) / rtu_v_bus + 0.5F) * 8500.0F;
}

real32_T rt_modf_snf(real32_T u0, real32_T u1)
{
  real32_T y;
  y = u0;
  if (u1 == 0.0F) {
    if (u0 == 0.0F) {
      y = u1;
    }
  } else if (rtIsNaNF(u0) || rtIsNaNF(u1) || rtIsInfF(u0)) {
    y = (rtNaNF);
  } else if (u0 == 0.0F) {
    y = 0.0F / u1;
  } else if (rtIsInfF(u1)) {
    if ((u1 < 0.0F) != (u0 < 0.0F)) {
      y = u1;
    }
  } else {
    boolean_T yEq;
    y = fmodf(u0, u1);
    yEq = (y == 0.0F);
    if ((!yEq) && (u1 > floorf(u1))) {
      real32_T q;
      q = fabsf(u0 / u1);
      yEq = !(fabsf(q - floorf(q + 0.5F)) > FLT_EPSILON * q);
    }

    if (yEq) {
      y = u1 * 0.0F;
    } else if ((u0 < 0.0F) != (u1 < 0.0F)) {
      y += u1;
    }
  }

  return y;
}

/* Model step function */
void FOC_Model_step(void)
{
  real32_T PWM_HalfPeriod[3];
  real32_T DiscreteTimeIntegrator;
  real32_T DiscreteTimeIntegrator1_a;
  real32_T DiscreteTimeIntegrator_m;
  real32_T Integrator_c;
  real32_T Integrator_l;
  real32_T rtb_Add2_f;
  real32_T rtb_Add_m;
  real32_T rtb_IProdOut_p;
  real32_T rtb_Saturation;
  real32_T rtb_SignPreIntegrator_a;
  real32_T rtb_SignPreIntegrator_b;
  real32_T rtb_SignPreSat;
  real32_T rtb_SignPreSat_1;
  real32_T rtb_SignPreSat_o;
  real32_T rtb_SinCos1;
  real32_T rtb_ZeroGain;
  real32_T rtb_ZeroGain_h;
  real32_T rtb_ZeroGain_m;
  uint32_T CurrLoop_ELAPS_T;
  uint32_T CurrLoop_PREV_T_tmp;
  int8_T rtAction;
  int8_T rtPrevAction;

  /* Outputs for Atomic SubSystem: '<Root>/FOC_Model' */
  /* RateTransition: '<S1>/Rate Transition3' */
  if (rtM->Timing.TaskCounters.TID[1] == 0) {
    /* RateTransition: '<S1>/Rate Transition3' */
    rtDW.RateTransition3 = rtDW.RateTransition3_Buffer0;
  }

  /* End of RateTransition: '<S1>/Rate Transition3' */

  /* S-Function (fcgen): '<S1>/Function-Call Generator1' incorporates:
   *  SubSystem: '<S1>/CurrLoop'
   */
  if (rtDW.CurrLoop_RESET_ELAPS_T) {
    CurrLoop_ELAPS_T = 0U;
  } else {
    CurrLoop_ELAPS_T = rtM->Timing.clockTick0 - rtDW.CurrLoop_PREV_T;
  }

  /* Update for IfAction SubSystem: '<S2>/If Action Subsystem4' incorporates:
   *  ActionPort: '<S12>/Action Port'
   */
  /* Update for IfAction SubSystem: '<S2>/If Action Subsystem2' incorporates:
   *  ActionPort: '<S10>/Action Port'
   */
  /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem4' incorporates:
   *  ActionPort: '<S12>/Action Port'
   */
  /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem2' incorporates:
   *  ActionPort: '<S10>/Action Port'
   */
  /* SwitchCase: '<S2>/Switch Case1' */
  CurrLoop_PREV_T_tmp = rtM->Timing.clockTick0;

  /* End of Outputs for SubSystem: '<S2>/If Action Subsystem2' */
  /* End of Outputs for SubSystem: '<S2>/If Action Subsystem4' */
  /* End of Update for SubSystem: '<S2>/If Action Subsystem2' */
  /* End of Update for SubSystem: '<S2>/If Action Subsystem4' */
  rtDW.CurrLoop_PREV_T = CurrLoop_PREV_T_tmp;
  rtDW.CurrLoop_RESET_ELAPS_T = false;

  /* Sum: '<S6>/Add1' incorporates:
   *  Gain: '<S6>/Gain'
   *  Gain: '<S6>/Gain1'
   *  Inport: '<Root>/ia'
   *  Inport: '<Root>/ib'
   *  Inport: '<Root>/ic'
   *  Sum: '<S6>/Add'
   */
  ialpha = 0.666666687F * rtU.ia - (rtU.ib + rtU.ic) * 0.333333343F;

  /* Chart: '<S2>/Chart1' incorporates:
   *  Inport: '<Root>/Motor_OnOff'
   */
  if (rtDW.temporalCounter_i1 < 32767U) {
    rtDW.temporalCounter_i1++;
  }

  if (rtDW.is_active_c7_FOC_Model == 0U) {
    rtDW.is_active_c7_FOC_Model = 1U;
    rtDW.is_c7_FOC_Model = IN_IDLE;
  } else {
    switch (rtDW.is_c7_FOC_Model) {
     case IN_AlignStage:
      if (rtDW.temporalCounter_i1 >= 1000) {
        rtDW.is_c7_FOC_Model = IN_OpenStage;
        rtDW.temporalCounter_i1 = 0U;
        rtDW.ZReset = 0.0F;
        rtDW.cnt = 0.0F;
      } else if (rtU.Motor_OnOff == 0.0F) {
        rtDW.is_c7_FOC_Model = IN_IDLE;
      } else {
        state = 2.0F;
        rtDW.RestsSingal = 0.0;
      }
      break;

     case IN_IDLE:
      if (rtU.Motor_OnOff == 1.0F) {
        rtDW.is_c7_FOC_Model = IN_AlignStage;
        rtDW.temporalCounter_i1 = 0U;
      } else {
        state = 1.0F;
        rtDW.RestsSingal = 0.0;
      }
      break;

     case IN_OpenStage:
      if (rtU.Motor_OnOff == 0.0F) {
        rtDW.is_c7_FOC_Model = IN_IDLE;
      } else if (rtDW.temporalCounter_i1 >= 30000) {
        rtDW.is_c7_FOC_Model = IN_ThetaAlign;
        rtDW.temporalCounter_i1 = 0U;
      } else {
        if (rtDW.cnt == 1.0F) {
          rtDW.ZReset = 1.0F;
        }

        rtDW.cnt = 1.0F;
        state = 3.0F;
        rtDW.RestsSingal = 0.0;
      }
      break;

     case IN_RunStage:
      if (rtU.Motor_OnOff == 0.0F) {
        rtDW.is_c7_FOC_Model = IN_IDLE;
      } else {
        state = 5.0F;
        rtDW.RestsSingal = 1.0;
      }
      break;

     default:
      /* case IN_ThetaAlign: */
      if (rtDW.temporalCounter_i1 >= 5000) {
        rtDW.is_c7_FOC_Model = IN_RunStage;
      } else {
        state = 4.0F;
        rtDW.RestsSingal = 0.0;
      }
      break;
    }
  }

  /* End of Chart: '<S2>/Chart1' */

  /* SwitchCase: '<S2>/Switch Case1' */
  rtPrevAction = rtDW.SwitchCase1_ActiveSubsystem;
  rtAction = -1;
  switch ((int32_T)state) {
   case 1:
    rtAction = 0;
    break;

   case 2:
    rtAction = 1;
    break;

   case 3:
    rtAction = 2;
    break;

   case 4:
    rtAction = 3;
    break;

   case 5:
    rtAction = 4;
    break;
  }

  rtDW.SwitchCase1_ActiveSubsystem = rtAction;
  switch (rtAction) {
   case 0:
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem' incorporates:
     *  ActionPort: '<S8>/Action Port'
     */
    /* Merge: '<S2>/Merge' incorporates:
     *  Constant: '<S8>/Constant'
     *  SignalConversion generated from: '<S8>/theta_fd'
     */
    rtDW.Merge = 0.0F;

    /* Merge: '<S2>/Merge1' incorporates:
     *  Constant: '<S8>/Constant1'
     *  SignalConversion generated from: '<S8>/iq_ref'
     */
    rtDW.Merge1 = 0.0F;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem' */
    break;

   case 1:
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem1' incorporates:
     *  ActionPort: '<S9>/Action Port'
     */
    /* Merge: '<S2>/Merge' incorporates:
     *  Constant: '<S9>/Constant'
     *  SignalConversion generated from: '<S9>/theta_fd'
     */
    rtDW.Merge = 0.0F;

    /* Merge: '<S2>/Merge1' incorporates:
     *  Constant: '<S9>/Constant1'
     *  SignalConversion generated from: '<S9>/iq_ref'
     */
    rtDW.Merge1 = 1.0F;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem1' */
    break;

   case 2:
    {
      uint32_T IfActionSubsystem2_ELAPS_T;
      if (rtAction != rtPrevAction) {
        rtDW.IfActionSubsystem2_RESET_ELAPS_ = true;

        /* Enable for IfAction SubSystem: '<S2>/If Action Subsystem2' incorporates:
         *  ActionPort: '<S10>/Action Port'
         */
        /* Enable for SwitchCase: '<S2>/Switch Case1' incorporates:
         *  DiscreteIntegrator: '<S10>/Discrete-Time Integrator'
         *  DiscreteIntegrator: '<S10>/Discrete-Time Integrator1'
         */
        rtDW.DiscreteTimeIntegrator_SYSTEM_d = 1U;
        rtDW.DiscreteTimeIntegrator1_SYSTE_b = 1U;

        /* End of Enable for SubSystem: '<S2>/If Action Subsystem2' */
      }

      /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem2' incorporates:
       *  ActionPort: '<S10>/Action Port'
       */
      if (rtDW.IfActionSubsystem2_RESET_ELAPS_) {
        IfActionSubsystem2_ELAPS_T = 0U;
      } else {
        IfActionSubsystem2_ELAPS_T = CurrLoop_PREV_T_tmp -
          rtDW.IfActionSubsystem2_PREV_T;
      }

      rtDW.IfActionSubsystem2_PREV_T = CurrLoop_PREV_T_tmp;
      rtDW.IfActionSubsystem2_RESET_ELAPS_ = false;

      /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */
      if (rtDW.DiscreteTimeIntegrator_SYSTEM_d != 0) {
        /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */
        DiscreteTimeIntegrator_m = rtDW.DiscreteTimeIntegrator_DSTATE_a;
      } else if ((rtDW.ZReset > 0.0F) && (rtDW.DiscreteTimeIntegrator_PrevRese <=
                  0)) {
        rtDW.DiscreteTimeIntegrator_DSTATE_a = 0.0F;

        /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */
        DiscreteTimeIntegrator_m = rtDW.DiscreteTimeIntegrator_DSTATE_a;
      } else {
        /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */
        DiscreteTimeIntegrator_m = 0.0001F * (real32_T)
          IfActionSubsystem2_ELAPS_T * rtDW.DiscreteTimeIntegrator_PREV_U_b +
          rtDW.DiscreteTimeIntegrator_DSTATE_a;
      }

      /* End of DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */

      /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
      if (rtDW.DiscreteTimeIntegrator1_SYSTE_b != 0) {
        /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
        DiscreteTimeIntegrator1_a = rtDW.DiscreteTimeIntegrator1_DSTAT_k;
      } else if ((rtDW.ZReset > 0.0F) && (rtDW.DiscreteTimeIntegrator1_PrevRes <=
                  0)) {
        rtDW.DiscreteTimeIntegrator1_DSTAT_k = 0.0F;

        /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
        DiscreteTimeIntegrator1_a = rtDW.DiscreteTimeIntegrator1_DSTAT_k;
      } else {
        /* DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
        DiscreteTimeIntegrator1_a = 0.0001F * (real32_T)
          IfActionSubsystem2_ELAPS_T * rtDW.DiscreteTimeIntegrator1_PREV__i +
          rtDW.DiscreteTimeIntegrator1_DSTAT_k;
      }

      /* End of DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */

      /* Merge: '<S2>/Merge' incorporates:
       *  Constant: '<S10>/Constant'
       *  Math: '<S10>/Mod'
       */
      rtDW.Merge = rt_modf_snf(DiscreteTimeIntegrator1_a, 6.28318548F);

      /* Merge: '<S2>/Merge1' incorporates:
       *  Constant: '<S10>/Constant2'
       *  SignalConversion generated from: '<S10>/iq_ref'
       */
      rtDW.Merge1 = 1.0F;

      /* Update for DiscreteIntegrator: '<S10>/Discrete-Time Integrator' incorporates:
       *  Gain: '<S10>/Gain'
       *  Product: '<S10>/Product'
       */
      rtDW.DiscreteTimeIntegrator_SYSTEM_d = 0U;
      rtDW.DiscreteTimeIntegrator_DSTATE_a = DiscreteTimeIntegrator_m;
      if (rtDW.ZReset > 0.0F) {
        rtDW.DiscreteTimeIntegrator_PrevRese = 1;
      } else if (rtDW.ZReset < 0.0F) {
        rtDW.DiscreteTimeIntegrator_PrevRese = -1;
      } else if (rtDW.ZReset == 0.0F) {
        rtDW.DiscreteTimeIntegrator_PrevRese = 0;
      } else {
        rtDW.DiscreteTimeIntegrator_PrevRese = 2;
      }

      rtDW.DiscreteTimeIntegrator_PREV_U_b = motor.Pn * 62.8318558F *
        0.333333343F;

      /* End of Update for DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */

      /* Update for DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
      rtDW.DiscreteTimeIntegrator1_SYSTE_b = 0U;
      rtDW.DiscreteTimeIntegrator1_DSTAT_k = DiscreteTimeIntegrator1_a;
      if (rtDW.ZReset > 0.0F) {
        rtDW.DiscreteTimeIntegrator1_PrevRes = 1;
      } else if (rtDW.ZReset < 0.0F) {
        rtDW.DiscreteTimeIntegrator1_PrevRes = -1;
      } else if (rtDW.ZReset == 0.0F) {
        rtDW.DiscreteTimeIntegrator1_PrevRes = 0;
      } else {
        rtDW.DiscreteTimeIntegrator1_PrevRes = 2;
      }

      rtDW.DiscreteTimeIntegrator1_PREV__i = DiscreteTimeIntegrator_m;

      /* End of Update for DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
      /* End of Outputs for SubSystem: '<S2>/If Action Subsystem2' */
    }
    break;

   case 3:
    {
      uint32_T IfActionSubsystem2_ELAPS_T;
      if (rtAction != rtPrevAction) {
        rtDW.IfActionSubsystem4_RESET_ELAPS_ = true;

        /* Enable for IfAction SubSystem: '<S2>/If Action Subsystem4' incorporates:
         *  ActionPort: '<S12>/Action Port'
         */
        /* Enable for SwitchCase: '<S2>/Switch Case1' incorporates:
         *  DiscreteIntegrator: '<S12>/Discrete-Time Integrator1'
         */
        rtDW.DiscreteTimeIntegrator1_SYSTEM_ = 1U;

        /* End of Enable for SubSystem: '<S2>/If Action Subsystem4' */
      }

      /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem4' incorporates:
       *  ActionPort: '<S12>/Action Port'
       */
      if (rtDW.IfActionSubsystem4_RESET_ELAPS_) {
        IfActionSubsystem2_ELAPS_T = 0U;
      } else {
        IfActionSubsystem2_ELAPS_T = CurrLoop_PREV_T_tmp -
          rtDW.IfActionSubsystem4_PREV_T;
      }

      rtDW.IfActionSubsystem4_PREV_T = CurrLoop_PREV_T_tmp;
      rtDW.IfActionSubsystem4_RESET_ELAPS_ = false;

      /* DiscreteIntegrator: '<S12>/Discrete-Time Integrator1' */
      if (rtDW.DiscreteTimeIntegrator1_SYSTEM_ != 0) {
        /* DiscreteIntegrator: '<S12>/Discrete-Time Integrator1' */
        DiscreteTimeIntegrator_m = rtDW.DiscreteTimeIntegrator1_DSTATE;
      } else {
        /* DiscreteIntegrator: '<S12>/Discrete-Time Integrator1' */
        DiscreteTimeIntegrator_m = 0.0001F * (real32_T)
          IfActionSubsystem2_ELAPS_T * rtDW.DiscreteTimeIntegrator1_PREV_U +
          rtDW.DiscreteTimeIntegrator1_DSTATE;
      }

      /* End of DiscreteIntegrator: '<S12>/Discrete-Time Integrator1' */

      /* Merge: '<S2>/Merge' incorporates:
       *  Constant: '<S12>/Constant'
       *  Math: '<S12>/Mod'
       *  SignalConversion generated from: '<S12>/Theta_fd'
       */
      rtDW.Merge = rt_modf_snf(DiscreteTimeIntegrator_m, 6.28318548F);

      /* If: '<S12>/If' incorporates:
       *  Constant: '<S12>/Constant1'
       *  Constant: '<S70>/Constant'
       *  SignalConversion generated from: '<S70>/Out1'
       *  Sum: '<S12>/Add'
       *  UnitDelay: '<S12>/Unit Delay'
       */
      if (rtDW.UnitDelay_DSTATE_h + 0.01F >= 1.0F) {
        /* Outputs for IfAction SubSystem: '<S12>/If Action Subsystem' incorporates:
         *  ActionPort: '<S70>/Action Port'
         */
        rtb_SignPreIntegrator_b = 1.0F;

        /* End of Outputs for SubSystem: '<S12>/If Action Subsystem' */
      } else {
        rtb_SignPreIntegrator_b = rtDW.UnitDelay_DSTATE_h + 0.01F;
      }

      /* End of If: '<S12>/If' */

      /* Merge: '<S2>/Merge1' incorporates:
       *  Constant: '<S12>/Constant2'
       *  Constant: '<S12>/Constant4'
       *  Product: '<S12>/Product1'
       *  Sum: '<S12>/Add4'
       */
      rtDW.Merge1 = 1.0F - rtb_SignPreIntegrator_b * 0.2F;

      /* Update for DiscreteIntegrator: '<S12>/Discrete-Time Integrator1' incorporates:
       *  Gain: '<S12>/Gain'
       */
      rtDW.DiscreteTimeIntegrator1_SYSTEM_ = 0U;
      rtDW.DiscreteTimeIntegrator1_DSTATE = DiscreteTimeIntegrator_m;
      rtDW.DiscreteTimeIntegrator1_PREV_U = motor.Pn * 62.8318558F;

      /* Update for UnitDelay: '<S12>/Unit Delay' incorporates:
       *  Constant: '<S12>/Constant1'
       *  Sum: '<S12>/Add'
       */
      rtDW.UnitDelay_DSTATE_h += 0.01F;

      /* End of Outputs for SubSystem: '<S2>/If Action Subsystem4' */
    }
    break;

   case 4:
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem3' incorporates:
     *  ActionPort: '<S11>/Action Port'
     */
    /* Merge: '<S2>/Merge' incorporates:
     *  Inport: '<S11>/theta_Close'
     *  UnitDelay: '<S2>/Unit Delay1'
     *  HALL selector: Hal_State==0 -> observer(FluxTheta), Hal_State==1 -> HALL
     */
    if (Hal_State == 0) {
      /* Sensorless: use flux observer theta */
      rtDW.Merge = FluxTheta;
    } else {
      /* HALL sensor: use MeasuredElAngle from HALL handle */
      rtDW.Merge = HALL_Handle.MeasuredElAngle;
    }

    /* Merge: '<S2>/Merge1' incorporates:
     *  Inport: '<S11>/iq_CloseRef'
     */
    rtDW.Merge1 = rtDW.RateTransition3;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem3' */
    break;
  }

  /* Trigonometry: '<S2>/SinCos1' */
  rtb_SinCos1 = cosf(rtDW.Merge);

  /* Gain: '<S6>/Gain2' incorporates:
   *  Inport: '<Root>/ib'
   *  Inport: '<Root>/ic'
   *  Sum: '<S6>/Add2'
   */
  ibeta = (rtU.ib - rtU.ic) * 0.577350259F;

  /* Trigonometry: '<S2>/SinCos' */
  rtb_Add2_f = sinf(rtDW.Merge);

  /* Sum: '<S15>/Sum1' incorporates:
   *  Constant: '<S15>/Constant'
   *  Product: '<S13>/Product'
   *  Product: '<S13>/Product1'
   *  Sum: '<S13>/Add'
   */
  DiscreteTimeIntegrator_m = 0.0F - (ialpha * rtb_SinCos1 + ibeta * rtb_Add2_f);

  /* DiscreteIntegrator: '<S111>/Integrator' */
  if (rtDW.Integrator_SYSTEM_ENABLE != 0) {
    /* DiscreteIntegrator: '<S111>/Integrator' */
    DiscreteTimeIntegrator1_a = rtDW.Integrator_DSTATE;
  } else {
    /* DiscreteIntegrator: '<S111>/Integrator' */
    DiscreteTimeIntegrator1_a = 0.0001F * (real32_T)CurrLoop_ELAPS_T
      * rtDW.Integrator_PREV_U + rtDW.Integrator_DSTATE;
  }

  /* End of DiscreteIntegrator: '<S111>/Integrator' */

  /* Sum: '<S120>/Sum' incorporates:
   *  Constant: '<S15>/Constant3'
   *  Product: '<S116>/PProd Out'
   */
  rtb_SignPreSat = DiscreteTimeIntegrator_m * rt_Simulink_Struct.CurrKp +
    DiscreteTimeIntegrator1_a;

  /* Saturate: '<S118>/Saturation' */
  if (rtb_SignPreSat > 12.4707661F) {
    rtb_SignPreIntegrator_b = 12.4707661F;
  } else if (rtb_SignPreSat < -12.4707661F) {
    rtb_SignPreIntegrator_b = -12.4707661F;
  } else {
    rtb_SignPreIntegrator_b = rtb_SignPreSat;
  }

  /* End of Saturate: '<S118>/Saturation' */

  /* Sum: '<S15>/Sum7' incorporates:
   *  Product: '<S13>/Product2'
   *  Product: '<S13>/Product3'
   *  Sum: '<S13>/Add1'
   */
  rtb_IProdOut_p = rtDW.Merge1 - (ibeta * rtb_SinCos1 - ialpha * rtb_Add2_f);

  /* DiscreteIntegrator: '<S161>/Integrator' */
  if (rtDW.Integrator_SYSTEM_ENABLE_j != 0) {
    /* DiscreteIntegrator: '<S161>/Integrator' */
    Integrator_l = rtDW.Integrator_DSTATE_f;
  } else {
    /* DiscreteIntegrator: '<S161>/Integrator' */
    Integrator_l = 0.0001F * (real32_T)CurrLoop_ELAPS_T
      * rtDW.Integrator_PREV_U_a + rtDW.Integrator_DSTATE_f;
  }

  /* End of DiscreteIntegrator: '<S161>/Integrator' */

  /* Sum: '<S170>/Sum' incorporates:
   *  Constant: '<S15>/Constant1'
   *  Product: '<S166>/PProd Out'
   */
  rtb_SignPreSat_o = rtb_IProdOut_p * rt_Simulink_Struct.CurrKp + Integrator_l;

  /* Saturate: '<S168>/Saturation' */
  if (rtb_SignPreSat_o > 12.4707661F) {
    rtb_SignPreIntegrator_a = 12.4707661F;
  } else if (rtb_SignPreSat_o < -12.4707661F) {
    rtb_SignPreIntegrator_a = -12.4707661F;
  } else {
    rtb_SignPreIntegrator_a = rtb_SignPreSat_o;
  }

  /* End of Saturate: '<S168>/Saturation' */

  /* Sum: '<S4>/Add' incorporates:
   *  Product: '<S4>/Product'
   *  Product: '<S4>/Product1'
   */
  ualpha = rtb_SignPreIntegrator_b * rtb_SinCos1 - rtb_SignPreIntegrator_a *
    rtb_Add2_f;

  /* Sum: '<S4>/Add1' incorporates:
   *  Product: '<S4>/Product2'
   *  Product: '<S4>/Product3'
   */
  ubeta = rtb_SignPreIntegrator_b * rtb_Add2_f + rtb_SignPreIntegrator_a *
    rtb_SinCos1;

  /* Gain: '<S16>/Gain1' incorporates:
   *  Gain: '<S16>/Gain8'
   */
  rtb_Add_m = motor.L * ialpha;

  /* Sum: '<S16>/Sum' incorporates:
   *  Gain: '<S16>/Gain1'
   *  UnitDelay: '<S16>/Unit Delay2'
   */
  rtb_SignPreIntegrator_a = rtDW.UnitDelay2_DSTATE - rtb_Add_m;

  /* Gain: '<S16>/Gain3' incorporates:
   *  Gain: '<S16>/Gain9'
   */
  rtb_ZeroGain = motor.L * ibeta;

  /* Sum: '<S16>/Sum1' incorporates:
   *  Gain: '<S16>/Gain3'
   *  UnitDelay: '<S16>/Unit Delay4'
   */
  rtb_SignPreIntegrator_b = rtDW.UnitDelay4_DSTATE - rtb_ZeroGain;

  /* Sum: '<S16>/Add2' incorporates:
   *  Constant: '<S16>/Constant'
   *  Math: '<S16>/Magnitude Squared'
   *  Math: '<S16>/Magnitude Squared1'
   *  Math: '<S16>/Magnitude Squared2'
   *  Sum: '<S16>/Add1'
   *
   * About '<S16>/Magnitude Squared':
   *  Operator: magnitude^2
   *
   * About '<S16>/Magnitude Squared1':
   *  Operator: magnitude^2
   *
   * About '<S16>/Magnitude Squared2':
   *  Operator: magnitude^2
   */
  rtb_Add2_f = motor.flux * motor.flux - (rtb_SignPreIntegrator_a *
    rtb_SignPreIntegrator_a + rtb_SignPreIntegrator_b * rtb_SignPreIntegrator_b);

  /* Sum: '<S16>/Add6' incorporates:
   *  Constant: '<S16>/Constant1'
   *  Gain: '<S16>/Gain'
   *  Gain: '<S16>/Gain4'
   *  Product: '<S16>/Product'
   *  Sum: '<S16>/Add'
   *  Sum: '<S16>/Add4'
   *  UnitDelay: '<S16>/Unit Delay2'
   */
  rtb_SinCos1 = ((ualpha - motor.Rs * ialpha) + rt_Simulink_Struct.Gamma *
                 rtb_SignPreIntegrator_a * rtb_Add2_f) * 0.0001F +
    rtDW.UnitDelay2_DSTATE;

  /* Sum: '<S16>/Add8' incorporates:
   *  Constant: '<S16>/Constant2'
   *  Gain: '<S16>/Gain2'
   *  Gain: '<S16>/Gain5'
   *  Product: '<S16>/Product1'
   *  Sum: '<S16>/Add3'
   *  Sum: '<S16>/Add5'
   *  UnitDelay: '<S16>/Unit Delay4'
   */
  rtb_Add2_f = (rtb_Add2_f * rtb_SignPreIntegrator_b * rt_Simulink_Struct.Gamma
                + (ubeta - motor.Rs * ibeta)) * 0.0001F + rtDW.UnitDelay4_DSTATE;

  /* DiscreteIntegrator: '<S18>/Discrete-Time Integrator' */
  if (rtDW.DiscreteTimeIntegrator_SYSTEM_E != 0) {
    /* DiscreteIntegrator: '<S18>/Discrete-Time Integrator' */
    DiscreteTimeIntegrator = rtDW.DiscreteTimeIntegrator_DSTATE;
  } else {
    /* DiscreteIntegrator: '<S18>/Discrete-Time Integrator' */
    DiscreteTimeIntegrator = 0.0001F * (real32_T)CurrLoop_ELAPS_T
      * rtDW.DiscreteTimeIntegrator_PREV_U + rtDW.DiscreteTimeIntegrator_DSTATE;
  }

  /* End of DiscreteIntegrator: '<S18>/Discrete-Time Integrator' */

  /* Math: '<S18>/Mod' incorporates:
   *  Constant: '<S18>/Constant'
   */
  FluxTheta = rt_modf_snf(DiscreteTimeIntegrator, 6.28318548F);

  /* Sum: '<S18>/Add' incorporates:
   *  Gain: '<S16>/Gain6'
   *  Gain: '<S16>/Gain7'
   *  Product: '<S18>/Product'
   *  Product: '<S18>/Product1'
   *  Sum: '<S16>/Add7'
   *  Sum: '<S16>/Add9'
   *  Trigonometry: '<S18>/Cos'
   *  Trigonometry: '<S18>/Sin'
   */
  rtb_SignPreIntegrator_a = (rtb_Add2_f - rtb_ZeroGain) * (1.0F / motor.flux) *
    cosf(FluxTheta) - (rtb_SinCos1 - rtb_Add_m) * (1.0F / motor.flux) * sinf
    (FluxTheta);

  /* DiscreteIntegrator: '<S53>/Integrator' */
  if (rtDW.Integrator_SYSTEM_ENABLE_g != 0) {
    /* DiscreteIntegrator: '<S53>/Integrator' */
    Integrator_c = rtDW.Integrator_DSTATE_m;
  } else {
    /* DiscreteIntegrator: '<S53>/Integrator' */
    Integrator_c = 0.0001F * (real32_T)CurrLoop_ELAPS_T
      * rtDW.Integrator_PREV_U_g + rtDW.Integrator_DSTATE_m;
  }

  /* End of DiscreteIntegrator: '<S53>/Integrator' */

  /* Sum: '<S62>/Sum' incorporates:
   *  Gain: '<S58>/Proportional Gain'
   */
  rtb_SignPreIntegrator_b = rt_Simulink_Struct.PLLKp * rtb_SignPreIntegrator_a +
    Integrator_c;

  /* Saturate: '<S60>/Saturation' */
  if (rtb_SignPreIntegrator_b > 6283.18555F) {
    rtb_Saturation = 6283.18555F;
  } else if (rtb_SignPreIntegrator_b < -6283.18555F) {
    rtb_Saturation = -6283.18555F;
  } else {
    rtb_Saturation = rtb_SignPreIntegrator_b;
  }

  /* End of Saturate: '<S60>/Saturation' */

  /* Sum: '<S17>/Add' incorporates:
   *  Constant: '<S17>/Constant'
   *  Delay: '<S17>/Delay'
   *  Product: '<S17>/Product'
   *  Sum: '<S17>/Add1'
   */
  rtb_Add_m = (rtb_Saturation - rtDW.Delay_DSTATE) *
    rt_Simulink_Struct.LPFFilter + rtDW.Delay_DSTATE;

  /* Gain: '<S7>/Gain1' incorporates:
   *  Gain: '<S7>/Gain'
   */
  FluxWe = 1.0F / motor.Pn * rtb_Add_m * 9.54929638F;

  /* Gain: '<S44>/ZeroGain' */
  rtb_ZeroGain = 0.0F * rtb_SignPreIntegrator_b;

  /* DeadZone: '<S46>/DeadZone' */
  if (rtb_SignPreIntegrator_b > 6283.18555F) {
    rtb_SignPreIntegrator_b -= 6283.18555F;
  } else if (rtb_SignPreIntegrator_b >= -6283.18555F) {
    rtb_SignPreIntegrator_b = 0.0F;
  } else {
    rtb_SignPreIntegrator_b -= -6283.18555F;
  }

  /* End of DeadZone: '<S46>/DeadZone' */

  /* Gain: '<S50>/Integral Gain' */
  rtb_SignPreIntegrator_a *= rt_Simulink_Struct.PLLKi;

  /* Outputs for Atomic SubSystem: '<S2>/SVPWM' */
  /* Inport: '<Root>/v_bus' */
  SVPWM(ualpha, ubeta, rtU.v_bus, PWM_HalfPeriod);

  /* End of Outputs for SubSystem: '<S2>/SVPWM' */

  /* Outport: '<Root>/Tcmp2' incorporates:
   *  SignalConversion generated from: '<S2>/Tcmp2'
   */
  rtY.Tcmp2 = PWM_HalfPeriod[1];

  /* Outport: '<Root>/Tcmp3' incorporates:
   *  SignalConversion generated from: '<S2>/Tcmp3'
   */
  rtY.Tcmp3 = PWM_HalfPeriod[2];

  /* Gain: '<S102>/ZeroGain' */
  rtb_ZeroGain_h = 0.0F * rtb_SignPreSat;

  /* DeadZone: '<S104>/DeadZone' */
  if (rtb_SignPreSat > 12.4707661F) {
    rtb_SignPreSat -= 12.4707661F;
  } else if (rtb_SignPreSat >= -12.4707661F) {
    rtb_SignPreSat = 0.0F;
  } else {
    rtb_SignPreSat -= -12.4707661F;
  }

  /* End of DeadZone: '<S104>/DeadZone' */

  /* Product: '<S108>/IProd Out' incorporates:
   *  Constant: '<S15>/Constant4'
   */
  DiscreteTimeIntegrator_m *= rt_Simulink_Struct.CurrKi;

  /* Gain: '<S152>/ZeroGain' */
  rtb_ZeroGain_m = 0.0F * rtb_SignPreSat_o;

  /* DeadZone: '<S154>/DeadZone' */
  if (rtb_SignPreSat_o > 12.4707661F) {
    rtb_SignPreSat_o -= 12.4707661F;
  } else if (rtb_SignPreSat_o >= -12.4707661F) {
    rtb_SignPreSat_o = 0.0F;
  } else {
    rtb_SignPreSat_o -= -12.4707661F;
  }

  /* End of DeadZone: '<S154>/DeadZone' */

  /* Product: '<S158>/IProd Out' incorporates:
   *  Constant: '<S15>/Constant2'
   */
  rtb_IProdOut_p *= rt_Simulink_Struct.CurrKi;

  /* Update for SwitchCase: '<S2>/Switch Case1' */
  switch (rtDW.SwitchCase1_ActiveSubsystem) {
   case 0:
   case 1:
   case 4:
    break;

   case 2:
    /* Update for IfAction SubSystem: '<S2>/If Action Subsystem2' incorporates:
     *  ActionPort: '<S10>/Action Port'
     */
    rtDW.IfActionSubsystem2_PREV_T = CurrLoop_PREV_T_tmp;
    rtDW.IfActionSubsystem2_RESET_ELAPS_ = false;

    /* End of Update for SubSystem: '<S2>/If Action Subsystem2' */
    break;

   case 3:
    /* Update for IfAction SubSystem: '<S2>/If Action Subsystem4' incorporates:
     *  ActionPort: '<S12>/Action Port'
     */
    rtDW.IfActionSubsystem4_PREV_T = CurrLoop_PREV_T_tmp;
    rtDW.IfActionSubsystem4_RESET_ELAPS_ = false;

    /* End of Update for SubSystem: '<S2>/If Action Subsystem4' */
    break;
  }

  /* Update for DiscreteIntegrator: '<S111>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE = 0U;
  rtDW.Integrator_DSTATE = DiscreteTimeIntegrator1_a;

  /* Signum: '<S102>/SignPreSat' */
  if (rtb_SignPreSat < 0.0F) {
    rtb_SignPreSat_1 = -1.0F;
  } else if (rtb_SignPreSat > 0.0F) {
    rtb_SignPreSat_1 = 1.0F;
  } else if (rtb_SignPreSat == 0.0F) {
    rtb_SignPreSat_1 = 0.0F;
  } else {
    rtb_SignPreSat_1 = (rtNaNF);
  }

  /* End of Signum: '<S102>/SignPreSat' */

  /* Signum: '<S102>/SignPreIntegrator' */
  if (DiscreteTimeIntegrator_m < 0.0F) {
    DiscreteTimeIntegrator1_a = -1.0F;
  } else if (DiscreteTimeIntegrator_m > 0.0F) {
    DiscreteTimeIntegrator1_a = 1.0F;
  } else if (DiscreteTimeIntegrator_m == 0.0F) {
    DiscreteTimeIntegrator1_a = 0.0F;
  } else {
    DiscreteTimeIntegrator1_a = (rtNaNF);
  }

  /* End of Signum: '<S102>/SignPreIntegrator' */

  /* Switch: '<S102>/Switch' incorporates:
   *  DataTypeConversion: '<S102>/DataTypeConv1'
   *  DataTypeConversion: '<S102>/DataTypeConv2'
   *  Logic: '<S102>/AND3'
   *  RelationalOperator: '<S102>/Equal1'
   *  RelationalOperator: '<S102>/NotEqual'
   */
  if ((rtb_ZeroGain_h != rtb_SignPreSat) && ((int8_T)rtb_SignPreSat_1 == (int8_T)
       DiscreteTimeIntegrator1_a)) {
    /* Update for DiscreteIntegrator: '<S111>/Integrator' incorporates:
     *  Constant: '<S102>/Constant1'
     */
    rtDW.Integrator_PREV_U = 0.0F;
  } else {
    /* Update for DiscreteIntegrator: '<S111>/Integrator' */
    rtDW.Integrator_PREV_U = DiscreteTimeIntegrator_m;
  }

  /* End of Switch: '<S102>/Switch' */

  /* Update for DiscreteIntegrator: '<S161>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE_j = 0U;
  rtDW.Integrator_DSTATE_f = Integrator_l;

  /* Signum: '<S152>/SignPreSat' */
  if (rtb_SignPreSat_o < 0.0F) {
    DiscreteTimeIntegrator_m = -1.0F;
  } else if (rtb_SignPreSat_o > 0.0F) {
    DiscreteTimeIntegrator_m = 1.0F;
  } else if (rtb_SignPreSat_o == 0.0F) {
    DiscreteTimeIntegrator_m = 0.0F;
  } else {
    DiscreteTimeIntegrator_m = (rtNaNF);
  }

  /* End of Signum: '<S152>/SignPreSat' */

  /* Signum: '<S152>/SignPreIntegrator' */
  if (rtb_IProdOut_p < 0.0F) {
    rtb_SignPreSat = -1.0F;
  } else if (rtb_IProdOut_p > 0.0F) {
    rtb_SignPreSat = 1.0F;
  } else if (rtb_IProdOut_p == 0.0F) {
    rtb_SignPreSat = 0.0F;
  } else {
    rtb_SignPreSat = (rtNaNF);
  }

  /* End of Signum: '<S152>/SignPreIntegrator' */

  /* Switch: '<S152>/Switch' incorporates:
   *  DataTypeConversion: '<S152>/DataTypeConv1'
   *  DataTypeConversion: '<S152>/DataTypeConv2'
   *  Logic: '<S152>/AND3'
   *  RelationalOperator: '<S152>/Equal1'
   *  RelationalOperator: '<S152>/NotEqual'
   */
  if ((rtb_ZeroGain_m != rtb_SignPreSat_o) && ((int8_T)DiscreteTimeIntegrator_m ==
       (int8_T)rtb_SignPreSat)) {
    /* Update for DiscreteIntegrator: '<S161>/Integrator' incorporates:
     *  Constant: '<S152>/Constant1'
     */
    rtDW.Integrator_PREV_U_a = 0.0F;
  } else {
    /* Update for DiscreteIntegrator: '<S161>/Integrator' */
    rtDW.Integrator_PREV_U_a = rtb_IProdOut_p;
  }

  /* End of Switch: '<S152>/Switch' */

  /* Update for UnitDelay: '<S16>/Unit Delay2' */
  rtDW.UnitDelay2_DSTATE = rtb_SinCos1;

  /* Update for UnitDelay: '<S16>/Unit Delay4' */
  rtDW.UnitDelay4_DSTATE = rtb_Add2_f;

  /* Update for DiscreteIntegrator: '<S18>/Discrete-Time Integrator' */
  rtDW.DiscreteTimeIntegrator_SYSTEM_E = 0U;
  rtDW.DiscreteTimeIntegrator_DSTATE = DiscreteTimeIntegrator;
  rtDW.DiscreteTimeIntegrator_PREV_U = rtb_Saturation;

  /* Update for DiscreteIntegrator: '<S53>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE_g = 0U;
  rtDW.Integrator_DSTATE_m = Integrator_c;

  /* Signum: '<S44>/SignPreSat' */
  if (rtb_SignPreIntegrator_b < 0.0F) {
    rtb_SinCos1 = -1.0F;
  } else if (rtb_SignPreIntegrator_b > 0.0F) {
    rtb_SinCos1 = 1.0F;
  } else if (rtb_SignPreIntegrator_b == 0.0F) {
    rtb_SinCos1 = 0.0F;
  } else {
    rtb_SinCos1 = (rtNaNF);
  }

  /* End of Signum: '<S44>/SignPreSat' */

  /* Signum: '<S44>/SignPreIntegrator' */
  if (rtb_SignPreIntegrator_a < 0.0F) {
    DiscreteTimeIntegrator_m = -1.0F;
  } else if (rtb_SignPreIntegrator_a > 0.0F) {
    DiscreteTimeIntegrator_m = 1.0F;
  } else if (rtb_SignPreIntegrator_a == 0.0F) {
    DiscreteTimeIntegrator_m = 0.0F;
  } else {
    DiscreteTimeIntegrator_m = (rtNaNF);
  }

  /* End of Signum: '<S44>/SignPreIntegrator' */

  /* Switch: '<S44>/Switch' incorporates:
   *  DataTypeConversion: '<S44>/DataTypeConv1'
   *  DataTypeConversion: '<S44>/DataTypeConv2'
   *  Logic: '<S44>/AND3'
   *  RelationalOperator: '<S44>/Equal1'
   *  RelationalOperator: '<S44>/NotEqual'
   */
  if ((rtb_ZeroGain != rtb_SignPreIntegrator_b) && ((int8_T)rtb_SinCos1 ==
       (int8_T)DiscreteTimeIntegrator_m)) {
    /* Update for DiscreteIntegrator: '<S53>/Integrator' incorporates:
     *  Constant: '<S44>/Constant1'
     */
    rtDW.Integrator_PREV_U_g = 0.0F;
  } else {
    /* Update for DiscreteIntegrator: '<S53>/Integrator' */
    rtDW.Integrator_PREV_U_g = rtb_SignPreIntegrator_a;
  }

  /* End of Switch: '<S44>/Switch' */

  /* Update for Delay: '<S17>/Delay' */
  rtDW.Delay_DSTATE = rtb_Add_m;

  /* End of Outputs for S-Function (fcgen): '<S1>/Function-Call Generator1' */

  /* RateTransition: '<S1>/Rate Transition' */
  if (rtM->Timing.TaskCounters.TID[1] == 0) {
    /* S-Function (fcgen): '<S1>/Function-Call Generator' incorporates:
     *  SubSystem: '<S1>/SpeedLoop'
     */
    if (rtDW.SpeedLoop_RESET_ELAPS_T) {
      CurrLoop_ELAPS_T = 0U;
    } else {
      CurrLoop_ELAPS_T = rtM->Timing.clockTick1 - rtDW.SpeedLoop_PREV_T;
    }

    rtDW.SpeedLoop_PREV_T = rtM->Timing.clockTick1;
    rtDW.SpeedLoop_RESET_ELAPS_T = false;

    /* Sum: '<S3>/Sum' incorporates:
     *  Inport: '<Root>/SpeedRef'
     */
    DiscreteTimeIntegrator_m = (real32_T)rtU.SpeedRef - FluxWe;

    /* DiscreteIntegrator: '<S212>/Integrator' incorporates:
     *  RateTransition: '<S1>/Rate Transition2'
     */
    if (rtDW.Integrator_SYSTEM_ENABLE_l != 0) {
      /* DiscreteIntegrator: '<S212>/Integrator' */
      DiscreteTimeIntegrator1_a = rtDW.Integrator_DSTATE_m1;
    } else if ((rtDW.RestsSingal > 0.0) && (rtDW.Integrator_PrevResetState <= 0))
    {
      rtDW.Integrator_DSTATE_m1 = 0.0F;

      /* DiscreteIntegrator: '<S212>/Integrator' */
      DiscreteTimeIntegrator1_a = rtDW.Integrator_DSTATE_m1;
    } else {
      /* DiscreteIntegrator: '<S212>/Integrator' */
      DiscreteTimeIntegrator1_a = 0.001F * (real32_T)CurrLoop_ELAPS_T
        * rtDW.Integrator_PREV_U_p + rtDW.Integrator_DSTATE_m1;
    }

    /* End of DiscreteIntegrator: '<S212>/Integrator' */

    /* Sum: '<S221>/Sum' incorporates:
     *  Gain: '<S217>/Proportional Gain'
     */
    rtb_SignPreSat = rt_Simulink_Struct.spd_kp * DiscreteTimeIntegrator_m +
      DiscreteTimeIntegrator1_a;

    /* DeadZone: '<S205>/DeadZone' */
    if (rtb_SignPreSat > 3.0F) {
      rtb_SignPreIntegrator_b = rtb_SignPreSat - 3.0F;
    } else if (rtb_SignPreSat >= -3.0F) {
      rtb_SignPreIntegrator_b = 0.0F;
    } else {
      rtb_SignPreIntegrator_b = rtb_SignPreSat - -3.0F;
    }

    /* End of DeadZone: '<S205>/DeadZone' */

    /* Gain: '<S209>/Integral Gain' */
    DiscreteTimeIntegrator_m *= rt_Simulink_Struct.spd_ki;

    /* Update for DiscreteIntegrator: '<S212>/Integrator' incorporates:
     *  RateTransition: '<S1>/Rate Transition2'
     */
    rtDW.Integrator_SYSTEM_ENABLE_l = 0U;
    rtDW.Integrator_DSTATE_m1 = DiscreteTimeIntegrator1_a;
    if (rtDW.RestsSingal > 0.0) {
      rtDW.Integrator_PrevResetState = 1;
    } else if (rtDW.RestsSingal < 0.0) {
      rtDW.Integrator_PrevResetState = -1;
    } else if (rtDW.RestsSingal == 0.0) {
      rtDW.Integrator_PrevResetState = 0;
    } else {
      rtDW.Integrator_PrevResetState = 2;
    }

    /* Signum: '<S203>/SignPreSat' */
    if (rtb_SignPreIntegrator_b < 0.0F) {
      rtb_SinCos1 = -1.0F;
    } else if (rtb_SignPreIntegrator_b > 0.0F) {
      rtb_SinCos1 = 1.0F;
    } else if (rtb_SignPreIntegrator_b == 0.0F) {
      rtb_SinCos1 = 0.0F;
    } else {
      rtb_SinCos1 = (rtNaNF);
    }

    /* End of Signum: '<S203>/SignPreSat' */

    /* Signum: '<S203>/SignPreIntegrator' */
    if (DiscreteTimeIntegrator_m < 0.0F) {
      DiscreteTimeIntegrator1_a = -1.0F;
    } else if (DiscreteTimeIntegrator_m > 0.0F) {
      DiscreteTimeIntegrator1_a = 1.0F;
    } else if (DiscreteTimeIntegrator_m == 0.0F) {
      DiscreteTimeIntegrator1_a = 0.0F;
    } else {
      DiscreteTimeIntegrator1_a = (rtNaNF);
    }

    /* End of Signum: '<S203>/SignPreIntegrator' */

    /* Switch: '<S203>/Switch' incorporates:
     *  DataTypeConversion: '<S203>/DataTypeConv1'
     *  DataTypeConversion: '<S203>/DataTypeConv2'
     *  Gain: '<S203>/ZeroGain'
     *  Logic: '<S203>/AND3'
     *  RelationalOperator: '<S203>/Equal1'
     *  RelationalOperator: '<S203>/NotEqual'
     */
    if ((0.0F * rtb_SignPreSat != rtb_SignPreIntegrator_b) && ((int8_T)
         rtb_SinCos1 == (int8_T)DiscreteTimeIntegrator1_a)) {
      /* Update for DiscreteIntegrator: '<S212>/Integrator' incorporates:
       *  Constant: '<S203>/Constant1'
       */
      rtDW.Integrator_PREV_U_p = 0.0F;
    } else {
      /* Update for DiscreteIntegrator: '<S212>/Integrator' */
      rtDW.Integrator_PREV_U_p = DiscreteTimeIntegrator_m;
    }

    /* End of Switch: '<S203>/Switch' */

    /* Saturate: '<S219>/Saturation' */
    if (rtb_SignPreSat > 3.0F) {
      /* Update for RateTransition: '<S1>/Rate Transition3' */
      rtDW.RateTransition3_Buffer0 = 3.0F;
    } else if (rtb_SignPreSat < -3.0F) {
      /* Update for RateTransition: '<S1>/Rate Transition3' */
      rtDW.RateTransition3_Buffer0 = -3.0F;
    } else {
      /* Update for RateTransition: '<S1>/Rate Transition3' */
      rtDW.RateTransition3_Buffer0 = rtb_SignPreSat;
    }

    /* End of Saturate: '<S219>/Saturation' */
    /* End of Outputs for S-Function (fcgen): '<S1>/Function-Call Generator' */
  }

  /* End of RateTransition: '<S1>/Rate Transition' */
  /* End of Outputs for SubSystem: '<Root>/FOC_Model' */

  /* Outport: '<Root>/Tcmp1' */
  rtY.Tcmp1 = PWM_HalfPeriod[0];

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The resolution of this integer timer is 0.0001, which is the step size
   * of the task. Size of "clockTick0" ensures timer will not overflow during the
   * application lifespan selected.
   */
  rtM->Timing.clockTick0++;
  if (rtM->Timing.TaskCounters.TID[1] == 0) {
    /* Update absolute timer for sample time: [0.001s, 0.0s] */
    /* The "clockTick1" counts the number of times the code of this task has
     * been executed. The resolution of this integer timer is 0.001, which is the step size
     * of the task. Size of "clockTick1" ensures timer will not overflow during the
     * application lifespan selected.
     */
    rtM->Timing.clockTick1++;
  }

  rate_scheduler();
}

/* Model initialize function */
void FOC_Model_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* SystemInitialize for Atomic SubSystem: '<Root>/FOC_Model' */
  /* SystemInitialize for S-Function (fcgen): '<S1>/Function-Call Generator1' incorporates:
   *  SubSystem: '<S1>/CurrLoop'
   */
  /* Start for SwitchCase: '<S2>/Switch Case1' */
  rtDW.SwitchCase1_ActiveSubsystem = -1;

  /* SystemInitialize for IfAction SubSystem: '<S2>/If Action Subsystem2' */
  /* InitializeConditions for DiscreteIntegrator: '<S10>/Discrete-Time Integrator' */
  rtDW.DiscreteTimeIntegrator_PrevRese = 2;

  /* InitializeConditions for DiscreteIntegrator: '<S10>/Discrete-Time Integrator1' */
  rtDW.DiscreteTimeIntegrator1_PrevRes = 2;

  /* End of SystemInitialize for SubSystem: '<S2>/If Action Subsystem2' */
  /* End of SystemInitialize for S-Function (fcgen): '<S1>/Function-Call Generator1' */

  /* SystemInitialize for S-Function (fcgen): '<S1>/Function-Call Generator' incorporates:
   *  SubSystem: '<S1>/SpeedLoop'
   */
  /* InitializeConditions for DiscreteIntegrator: '<S212>/Integrator' */
  rtDW.Integrator_PrevResetState = 2;

  /* End of SystemInitialize for S-Function (fcgen): '<S1>/Function-Call Generator' */
  /* End of SystemInitialize for SubSystem: '<Root>/FOC_Model' */

  /* Enable for Atomic SubSystem: '<Root>/FOC_Model' */
  /* Enable for S-Function (fcgen): '<S1>/Function-Call Generator1' incorporates:
   *  SubSystem: '<S1>/CurrLoop'
   */
  rtDW.CurrLoop_RESET_ELAPS_T = true;

  /* Enable for DiscreteIntegrator: '<S111>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE = 1U;

  /* Enable for DiscreteIntegrator: '<S161>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE_j = 1U;

  /* Enable for DiscreteIntegrator: '<S18>/Discrete-Time Integrator' */
  rtDW.DiscreteTimeIntegrator_SYSTEM_E = 1U;

  /* Enable for DiscreteIntegrator: '<S53>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE_g = 1U;

  /* End of Enable for S-Function (fcgen): '<S1>/Function-Call Generator1' */

  /* Enable for S-Function (fcgen): '<S1>/Function-Call Generator' incorporates:
   *  SubSystem: '<S1>/SpeedLoop'
   */
  rtDW.SpeedLoop_RESET_ELAPS_T = true;

  /* Enable for DiscreteIntegrator: '<S212>/Integrator' */
  rtDW.Integrator_SYSTEM_ENABLE_l = 1U;

  /* End of Enable for S-Function (fcgen): '<S1>/Function-Call Generator' */
  /* End of Enable for SubSystem: '<Root>/FOC_Model' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
