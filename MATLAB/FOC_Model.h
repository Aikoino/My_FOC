/*
 * File: FOC_Model.h
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

#ifndef RTW_HEADER_FOC_Model_h_
#define RTW_HEADER_FOC_Model_h_
#include <stddef.h>
#include <float.h>
#include <math.h>
#include "tim.h"
#include "BSP_ADC.h"
#include "fcan_ctl.h"
#include "hall_sensor.h"
#ifndef FOC_Model_COMMON_INCLUDES_
#define FOC_Model_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* FOC_Model_COMMON_INCLUDES_ */

/* Model Code Variants */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T RestsSingal;                  /* '<S2>/Chart1' */
  real32_T RateTransition3;            /* '<S1>/Rate Transition3' */
  real32_T Merge;                      /* '<S2>/Merge' */
  real32_T Merge1;                     /* '<S2>/Merge1' */
  real32_T ZReset;                     /* '<S2>/Chart1' */
  real32_T Integrator_DSTATE;          /* '<S111>/Integrator' */
  real32_T Integrator_DSTATE_f;        /* '<S161>/Integrator' */
  real32_T UnitDelay2_DSTATE;          /* '<S16>/Unit Delay2' */
  real32_T UnitDelay4_DSTATE;          /* '<S16>/Unit Delay4' */
  real32_T DiscreteTimeIntegrator_DSTATE;/* '<S18>/Discrete-Time Integrator' */
  real32_T Integrator_DSTATE_m;        /* '<S53>/Integrator' */
  real32_T Delay_DSTATE;               /* '<S17>/Delay' */
  real32_T DiscreteTimeIntegrator1_DSTATE;/* '<S12>/Discrete-Time Integrator1' */
  real32_T UnitDelay_DSTATE_h;         /* '<S12>/Unit Delay' */
  real32_T DiscreteTimeIntegrator_DSTATE_a;/* '<S10>/Discrete-Time Integrator' */
  real32_T DiscreteTimeIntegrator1_DSTAT_k;/* '<S10>/Discrete-Time Integrator1' */
  real32_T Integrator_DSTATE_m1;       /* '<S212>/Integrator' */
  real32_T RateTransition3_Buffer0;    /* '<S1>/Rate Transition3' */
  real32_T Integrator_PREV_U;          /* '<S111>/Integrator' */
  real32_T Integrator_PREV_U_a;        /* '<S161>/Integrator' */
  real32_T DiscreteTimeIntegrator_PREV_U;/* '<S18>/Discrete-Time Integrator' */
  real32_T Integrator_PREV_U_g;        /* '<S53>/Integrator' */
  real32_T DiscreteTimeIntegrator1_PREV_U;/* '<S12>/Discrete-Time Integrator1' */
  real32_T DiscreteTimeIntegrator_PREV_U_b;/* '<S10>/Discrete-Time Integrator' */
  real32_T DiscreteTimeIntegrator1_PREV__i;/* '<S10>/Discrete-Time Integrator1' */
  real32_T cnt;                        /* '<S2>/Chart1' */
  real32_T Integrator_PREV_U_p;        /* '<S212>/Integrator' */
  uint32_T CurrLoop_PREV_T;            /* '<S1>/CurrLoop' */
  uint32_T IfActionSubsystem4_PREV_T;  /* '<S2>/If Action Subsystem4' */
  uint32_T IfActionSubsystem2_PREV_T;  /* '<S2>/If Action Subsystem2' */
  uint32_T SpeedLoop_PREV_T;           /* '<S1>/SpeedLoop' */
  uint16_T temporalCounter_i1;         /* '<S2>/Chart1' */
  int8_T SwitchCase1_ActiveSubsystem;  /* '<S2>/Switch Case1' */
  int8_T DiscreteTimeIntegrator_PrevRese;/* '<S10>/Discrete-Time Integrator' */
  int8_T DiscreteTimeIntegrator1_PrevRes;/* '<S10>/Discrete-Time Integrator1' */
  int8_T Integrator_PrevResetState;    /* '<S212>/Integrator' */
  uint8_T Integrator_SYSTEM_ENABLE;    /* '<S111>/Integrator' */
  uint8_T Integrator_SYSTEM_ENABLE_j;  /* '<S161>/Integrator' */
  uint8_T DiscreteTimeIntegrator_SYSTEM_E;/* '<S18>/Discrete-Time Integrator' */
  uint8_T Integrator_SYSTEM_ENABLE_g;  /* '<S53>/Integrator' */
  uint8_T DiscreteTimeIntegrator1_SYSTEM_;/* '<S12>/Discrete-Time Integrator1' */
  uint8_T DiscreteTimeIntegrator_SYSTEM_d;/* '<S10>/Discrete-Time Integrator' */
  uint8_T DiscreteTimeIntegrator1_SYSTE_b;/* '<S10>/Discrete-Time Integrator1' */
  uint8_T is_active_c7_FOC_Model;      /* '<S2>/Chart1' */
  uint8_T is_c7_FOC_Model;             /* '<S2>/Chart1' */
  uint8_T Integrator_SYSTEM_ENABLE_l;  /* '<S212>/Integrator' */
  boolean_T CurrLoop_RESET_ELAPS_T;    /* '<S1>/CurrLoop' */
  boolean_T IfActionSubsystem4_RESET_ELAPS_;/* '<S2>/If Action Subsystem4' */
  boolean_T IfActionSubsystem2_RESET_ELAPS_;/* '<S2>/If Action Subsystem2' */
  boolean_T SpeedLoop_RESET_ELAPS_T;   /* '<S1>/SpeedLoop' */
} DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real32_T ia;                         /* '<Root>/ia' */
  real32_T ib;                         /* '<Root>/ib' */
  real32_T ic;                         /* '<Root>/ic' */
  real32_T v_bus;                      /* '<Root>/v_bus' */
  real32_T Motor_OnOff;                /* '<Root>/Motor_OnOff' */
  real_T SpeedRef;                     /* '<Root>/SpeedRef' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real32_T Tcmp1;                      /* '<Root>/Tcmp1' */
  real32_T Tcmp2;                      /* '<Root>/Tcmp2' */
  real32_T Tcmp3;                      /* '<Root>/Tcmp3' */
} ExtY;

/* Type definition for custom storage class: Struct */
typedef struct motor_tag {
  real32_T L;                          /* Referenced by:
                                        * '<S16>/Gain1'
                                        * '<S16>/Gain3'
                                        * '<S16>/Gain8'
                                        * '<S16>/Gain9'
                                        */
  real32_T Pn;                         /* Referenced by:
                                        * '<S7>/Gain'
                                        * '<S10>/Gain'
                                        * '<S12>/Gain'
                                        */
  real32_T Rs;                         /* Referenced by:
                                        * '<S16>/Gain'
                                        * '<S16>/Gain2'
                                        */
  real32_T flux;                       /* Referenced by:
                                        * '<S16>/Constant'
                                        * '<S16>/Gain6'
                                        * '<S16>/Gain7'
                                        */
} motor_type;

typedef struct rt_Simulink_Struct_tag {
  real32_T CurrKi;                     /* Referenced by:
                                        * '<S15>/Constant2'
                                        * '<S15>/Constant4'
                                        */
  real32_T CurrKp;                     /* Referenced by:
                                        * '<S15>/Constant1'
                                        * '<S15>/Constant3'
                                        */
  real32_T Gamma;                      /* Referenced by:
                                        * '<S16>/Constant1'
                                        * '<S16>/Constant2'
                                        */
  real32_T LPFFilter;                  /* Referenced by: '<S17>/Constant' */
  real32_T PLLKi;                     /* Referenced by: '<S50>/Integral Gain' */
  real32_T PLLKp;                 /* Referenced by: '<S58>/Proportional Gain' */
  real32_T spd_ki;                   /* Referenced by: '<S209>/Integral Gain' */
  real32_T spd_kp;               /* Referenced by: '<S217>/Proportional Gain' */
} rt_Simulink_Struct_type;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    uint32_T clockTick0;
    uint32_T clockTick1;
    struct {
      uint8_T TID[2];
    } TaskCounters;
  } Timing;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/*
 * Exported Global Signals
 *
 * Note: Exported global signals are block signals with an exported global
 * storage class designation.  Code generation will declare the memory for
 * these signals and export their symbols.
 *
 * Exported block signals */

 /* External variables */
 extern real32_T ialpha;                /* '<S6>/Add1' */
 extern real32_T ibeta;                 /* '<S6>/Gain2' */
 extern real32_T ualpha;                /* '<S4>/Add' */
 extern real32_T ubeta;                 /* '<S4>/Add1' */
 extern real32_T FluxTheta;             /* '<S18>/Mod' */
 extern real32_T FluxWe;                /* '<S7>/Gain1' */
 extern real32_T state;                 /* '<S2>/Chart1' */

 /* HALL_Handle_t is defined in tim.h (Core/Inc/tim.h) */
 extern HALL_Handle_t HALL_Handle;
 extern uint8_t Hal_State;

 /* Model entry point functions */
 extern void FOC_Model_initialize(void);
 extern void FOC_Model_step(void);

 /* Exported data declaration */

/* Declaration for custom storage class: Struct */
extern motor_type motor;
extern rt_Simulink_Struct_type rt_Simulink_Struct;

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S7>/Scope1' : Unused code path elimination
 * Block '<S10>/Scope' : Unused code path elimination
 * Block '<S10>/Scope1' : Unused code path elimination
 * Block '<S12>/Add1' : Unused code path elimination
 * Block '<S12>/Add2' : Unused code path elimination
 * Block '<S12>/Constant3' : Unused code path elimination
 * Block '<S12>/Mod1' : Unused code path elimination
 * Block '<S12>/Product' : Unused code path elimination
 * Block '<S12>/Scope' : Unused code path elimination
 * Block '<S12>/Scope1' : Unused code path elimination
 * Block '<S12>/Scope2' : Unused code path elimination
 * Block '<S12>/Scope3' : Unused code path elimination
 * Block '<S12>/Scope4' : Unused code path elimination
 * Block '<S2>/Scope' : Unused code path elimination
 * Block '<S2>/Scope1' : Unused code path elimination
 * Block '<S2>/Scope3' : Unused code path elimination
 * Block '<S15>/Scope' : Unused code path elimination
 * Block '<S1>/Scope' : Unused code path elimination
 * Block '<S2>/Data Type Conversion3' : Eliminate redundant data type conversion
 * Block '<S2>/Data Type Conversion4' : Eliminate redundant data type conversion
 * Block '<S1>/Rate Transition1' : Eliminated since input and output rates are identical
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('matlab_model/FOC_Model')    - opens subsystem matlab_model/FOC_Model
 * hilite_system('matlab_model/FOC_Model/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'matlab_model'
 * '<S1>'   : 'matlab_model/FOC_Model'
 * '<S2>'   : 'matlab_model/FOC_Model/CurrLoop'
 * '<S3>'   : 'matlab_model/FOC_Model/SpeedLoop'
 * '<S4>'   : 'matlab_model/FOC_Model/CurrLoop/AntiPark'
 * '<S5>'   : 'matlab_model/FOC_Model/CurrLoop/Chart1'
 * '<S6>'   : 'matlab_model/FOC_Model/CurrLoop/Clark'
 * '<S7>'   : 'matlab_model/FOC_Model/CurrLoop/Flux'
 * '<S8>'   : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem'
 * '<S9>'   : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem1'
 * '<S10>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem2'
 * '<S11>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem3'
 * '<S12>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem4'
 * '<S13>'  : 'matlab_model/FOC_Model/CurrLoop/Park'
 * '<S14>'  : 'matlab_model/FOC_Model/CurrLoop/SVPWM'
 * '<S15>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller'
 * '<S16>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/FluxObserver'
 * '<S17>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/LFP'
 * '<S18>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL'
 * '<S19>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller'
 * '<S20>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Anti-windup'
 * '<S21>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/D Gain'
 * '<S22>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Filter'
 * '<S23>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Filter ICs'
 * '<S24>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/I Gain'
 * '<S25>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Ideal P Gain'
 * '<S26>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Ideal P Gain Fdbk'
 * '<S27>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Integrator'
 * '<S28>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Integrator ICs'
 * '<S29>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/N Copy'
 * '<S30>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/N Gain'
 * '<S31>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/P Copy'
 * '<S32>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Parallel P Gain'
 * '<S33>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Reset Signal'
 * '<S34>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Saturation'
 * '<S35>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Saturation Fdbk'
 * '<S36>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Sum'
 * '<S37>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Sum Fdbk'
 * '<S38>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tracking Mode'
 * '<S39>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tracking Mode Sum'
 * '<S40>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tsamp - Integral'
 * '<S41>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tsamp - Ngain'
 * '<S42>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/postSat Signal'
 * '<S43>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/preSat Signal'
 * '<S44>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Anti-windup/Disc. Clamping Parallel'
 * '<S45>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S46>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S47>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/D Gain/Disabled'
 * '<S48>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Filter/Disabled'
 * '<S49>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Filter ICs/Disabled'
 * '<S50>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/I Gain/Internal Parameters'
 * '<S51>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Ideal P Gain/Passthrough'
 * '<S52>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S53>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Integrator/Discrete'
 * '<S54>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Integrator ICs/Internal IC'
 * '<S55>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/N Copy/Disabled wSignal Specification'
 * '<S56>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/N Gain/Disabled'
 * '<S57>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/P Copy/Disabled'
 * '<S58>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S59>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Reset Signal/Disabled'
 * '<S60>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Saturation/Enabled'
 * '<S61>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Saturation Fdbk/Disabled'
 * '<S62>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Sum/Sum_PI'
 * '<S63>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Sum Fdbk/Disabled'
 * '<S64>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tracking Mode/Disabled'
 * '<S65>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S66>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tsamp - Integral/Passthrough'
 * '<S67>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S68>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/postSat Signal/Forward_Path'
 * '<S69>'  : 'matlab_model/FOC_Model/CurrLoop/Flux/PLL/PID Controller/preSat Signal/Forward_Path'
 * '<S70>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem4/If Action Subsystem'
 * '<S71>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem4/If Action Subsystem1'
 * '<S72>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem4/If Action Subsystem2'
 * '<S73>'  : 'matlab_model/FOC_Model/CurrLoop/If Action Subsystem4/If Action Subsystem3'
 * '<S74>'  : 'matlab_model/FOC_Model/CurrLoop/SVPWM/AntiClark'
 * '<S75>'  : 'matlab_model/FOC_Model/CurrLoop/SVPWM/ei_t'
 * '<S76>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1'
 * '<S77>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2'
 * '<S78>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Anti-windup'
 * '<S79>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/D Gain'
 * '<S80>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Filter'
 * '<S81>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Filter ICs'
 * '<S82>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/I Gain'
 * '<S83>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Ideal P Gain'
 * '<S84>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Ideal P Gain Fdbk'
 * '<S85>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Integrator'
 * '<S86>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Integrator ICs'
 * '<S87>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/N Copy'
 * '<S88>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/N Gain'
 * '<S89>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/P Copy'
 * '<S90>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Parallel P Gain'
 * '<S91>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Reset Signal'
 * '<S92>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Saturation'
 * '<S93>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Saturation Fdbk'
 * '<S94>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Sum'
 * '<S95>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Sum Fdbk'
 * '<S96>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tracking Mode'
 * '<S97>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tracking Mode Sum'
 * '<S98>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tsamp - Integral'
 * '<S99>'  : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tsamp - Ngain'
 * '<S100>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/postSat Signal'
 * '<S101>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/preSat Signal'
 * '<S102>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Anti-windup/Disc. Clamping Parallel'
 * '<S103>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S104>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S105>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/D Gain/Disabled'
 * '<S106>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Filter/Disabled'
 * '<S107>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Filter ICs/Disabled'
 * '<S108>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/I Gain/External Parameters'
 * '<S109>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Ideal P Gain/Passthrough'
 * '<S110>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S111>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Integrator/Discrete'
 * '<S112>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Integrator ICs/Internal IC'
 * '<S113>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/N Copy/Disabled wSignal Specification'
 * '<S114>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/N Gain/Disabled'
 * '<S115>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/P Copy/Disabled'
 * '<S116>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Parallel P Gain/External Parameters'
 * '<S117>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Reset Signal/Disabled'
 * '<S118>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Saturation/Enabled'
 * '<S119>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Saturation Fdbk/Disabled'
 * '<S120>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Sum/Sum_PI'
 * '<S121>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Sum Fdbk/Disabled'
 * '<S122>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tracking Mode/Disabled'
 * '<S123>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S124>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tsamp - Integral/Passthrough'
 * '<S125>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S126>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/postSat Signal/Forward_Path'
 * '<S127>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller1/preSat Signal/Forward_Path'
 * '<S128>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Anti-windup'
 * '<S129>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/D Gain'
 * '<S130>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Filter'
 * '<S131>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Filter ICs'
 * '<S132>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/I Gain'
 * '<S133>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Ideal P Gain'
 * '<S134>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Ideal P Gain Fdbk'
 * '<S135>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Integrator'
 * '<S136>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Integrator ICs'
 * '<S137>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/N Copy'
 * '<S138>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/N Gain'
 * '<S139>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/P Copy'
 * '<S140>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Parallel P Gain'
 * '<S141>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Reset Signal'
 * '<S142>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Saturation'
 * '<S143>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Saturation Fdbk'
 * '<S144>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Sum'
 * '<S145>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Sum Fdbk'
 * '<S146>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tracking Mode'
 * '<S147>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tracking Mode Sum'
 * '<S148>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tsamp - Integral'
 * '<S149>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tsamp - Ngain'
 * '<S150>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/postSat Signal'
 * '<S151>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/preSat Signal'
 * '<S152>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Anti-windup/Disc. Clamping Parallel'
 * '<S153>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S154>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S155>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/D Gain/Disabled'
 * '<S156>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Filter/Disabled'
 * '<S157>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Filter ICs/Disabled'
 * '<S158>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/I Gain/External Parameters'
 * '<S159>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Ideal P Gain/Passthrough'
 * '<S160>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Ideal P Gain Fdbk/Disabled'
 * '<S161>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Integrator/Discrete'
 * '<S162>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Integrator ICs/Internal IC'
 * '<S163>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/N Copy/Disabled wSignal Specification'
 * '<S164>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/N Gain/Disabled'
 * '<S165>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/P Copy/Disabled'
 * '<S166>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Parallel P Gain/External Parameters'
 * '<S167>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Reset Signal/Disabled'
 * '<S168>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Saturation/Enabled'
 * '<S169>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Saturation Fdbk/Disabled'
 * '<S170>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Sum/Sum_PI'
 * '<S171>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Sum Fdbk/Disabled'
 * '<S172>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tracking Mode/Disabled'
 * '<S173>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tracking Mode Sum/Passthrough'
 * '<S174>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tsamp - Integral/Passthrough'
 * '<S175>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/Tsamp - Ngain/Passthrough'
 * '<S176>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/postSat Signal/Forward_Path'
 * '<S177>' : 'matlab_model/FOC_Model/CurrLoop/idq_Controller/PID Controller2/preSat Signal/Forward_Path'
 * '<S178>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller'
 * '<S179>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Anti-windup'
 * '<S180>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/D Gain'
 * '<S181>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Filter'
 * '<S182>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Filter ICs'
 * '<S183>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/I Gain'
 * '<S184>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Ideal P Gain'
 * '<S185>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Ideal P Gain Fdbk'
 * '<S186>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Integrator'
 * '<S187>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Integrator ICs'
 * '<S188>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/N Copy'
 * '<S189>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/N Gain'
 * '<S190>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/P Copy'
 * '<S191>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Parallel P Gain'
 * '<S192>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Reset Signal'
 * '<S193>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Saturation'
 * '<S194>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Saturation Fdbk'
 * '<S195>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Sum'
 * '<S196>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Sum Fdbk'
 * '<S197>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tracking Mode'
 * '<S198>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tracking Mode Sum'
 * '<S199>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tsamp - Integral'
 * '<S200>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tsamp - Ngain'
 * '<S201>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/postSat Signal'
 * '<S202>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/preSat Signal'
 * '<S203>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Anti-windup/Disc. Clamping Parallel'
 * '<S204>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S205>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S206>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/D Gain/Disabled'
 * '<S207>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Filter/Disabled'
 * '<S208>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Filter ICs/Disabled'
 * '<S209>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/I Gain/Internal Parameters'
 * '<S210>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Ideal P Gain/Passthrough'
 * '<S211>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S212>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Integrator/Discrete'
 * '<S213>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Integrator ICs/Internal IC'
 * '<S214>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/N Copy/Disabled wSignal Specification'
 * '<S215>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/N Gain/Disabled'
 * '<S216>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/P Copy/Disabled'
 * '<S217>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S218>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Reset Signal/External Reset'
 * '<S219>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Saturation/Enabled'
 * '<S220>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Saturation Fdbk/Disabled'
 * '<S221>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Sum/Sum_PI'
 * '<S222>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Sum Fdbk/Disabled'
 * '<S223>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tracking Mode/Disabled'
 * '<S224>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S225>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tsamp - Integral/Passthrough'
 * '<S226>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S227>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/postSat Signal/Forward_Path'
 * '<S228>' : 'matlab_model/FOC_Model/SpeedLoop/PID Controller/preSat Signal/Forward_Path'
 */
#endif                                 /* RTW_HEADER_FOC_Model_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
