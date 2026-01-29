#ifndef REGISTERS_H
#define REGISTERS_H

#define SET_BIT(reg, shift)    ((*reg) |= (1U << (shift)))
#define CLEAR_BIT(reg, shift)  ((*reg) &= ~(1U << (shift)))
#define GET_BIT(reg, shift)    (((*(reg) >> (shift)) & 1U))

#define BIT(n) (1U << n)
#define FIELD(width, shift) (((1 << width) - 1) << shift)

#define SET_REG_FIELD(reg, shift, mask, value) \
    (*(reg) |= (*(reg) & ~(mask)) | ((value << shift) & mask))

#define CLEAR_REG_FIELD(reg, mask) \
    (*(reg) &= ~(mask))

#define GET_REG_FIELD(reg, shift, mask) \
    (((*(reg)) & (mask)) >> (shift))

    //TODO: update the old definitons to the new form. looks like shit tbh
/*---------------------------------------------------------------------------------------*/
/*TIMER2 and TIMER4----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
#define TMR_BASE_ADDRESS 0x0119
#define TMR_OFFSET_NEXT_MODULE 0x6
#define TMR_NUM_MODULES 0x2

#define TMR_TMR_OFFSET        0x0
#define TMR_PR_OFFSET         0x1
#define TMR_CON_OFFSET        0x2
#define TMR_HLT_OFFSET        0x3
#define TMR_CLKCON_OFFSET     0x4
#define TMR_RST_OFFSET        0x5

// PRx register field definitions
#define TMR_PR_SHIFT          0
#define TMR_PR_MASK           FIELD(8, T2PR_SHIFT)

// TMRx register field definitions
#define TMR_TMR_SHIFT         0
#define TMR_TMR_MASK          FIELD(8, T2TMR_SHIFT)

// TxCON register field definitions
#define TMR_CON_ON_SHIFT      7
#define TMR_CON_CKPS_SHIFT    4
#define TMR_CON_OUTPS_SHIFT   0

#define TMR_CON_ON_MASK       BIT(TMR_CON_ON_SHIFT)
#define TMR_CON_CKPS_MASK     FIELD(3, TMR_CON_CKPS_SHIFT)
#define TMR_CON_OUTPS_MASK    FIELD(4, TMR_CON_OUTPS_SHIFT)

// TxHLT register field definitions
#define TMR_HLT_PSYNC_SHIFT   7
#define TMR_HLT_CPOL_SHIFT    6
#define TMR_HLT_CSYNC_SHIFT   5
#define TMR_HLT_MODE_SHIFT    0

#define TMR_HLT_PSYNC_MASK    BIT(TMR_HLT_PSYNC_SHIFT)
#define TMR_HLT_CPOL_MASK     BIT(TMR_HLT_CPOL_SHIFT)
#define TMR_HLT_CSYNC_MASK    BIT(TMR_HLT_CSYNC_SHIFT)
#define TMR_HLT_MODE_MASK     FIELD(5, TMR_HLT_MODE_SHIFT)

// TxCLKCON register field definitions
#define TMR_CLKCON_CS_SHIFT   0
#define TMR_CLKCON_CS_MASK    FIELD(4, TMR_CLKCON_CS_SHIFT)

// TxRST register field definitions
#define TMR_RST_RSEL_SHIFT    0
#define TMR_RST_RSEL_MASK     FIELD(5, TMR_RST_RSEL_SHIFT)
/*---------------------------------------------------------------------------------------*/
/*CCP1 and CCP2--------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
// TODO clean up, and make consistent
#define CCP_BASE_ADDRESS 0x149
#define CCP_OFFSET_NEXT_CCP 0x4
#define CCP_NUM_MODULES 0x2
// register offsets
#define CCPCCPRL_OFFSET 0x00 
#define CCPCCPRH_OFFSET 0x01 
#define CCPCON_OFFSET 0x02 
#define CCPCAP_OFFSET 0x03 
// CCPRx register field definitions
    // -- for cap/com same as offset, so dont care
// CCPxCON register field definitions
#define CCP_EN_SHIFT 0x7
#define CCP_EN_MASK  (1U << CCP_EN_SHIFT)
#define CCP_FMT_SHIFT 0x4
#define CCP_FMT_MASK  (1U << CCP_FMT_SHIFT)
#define CCP_MODE_SHIFT 0x0 
#define CCP_MODE_MASK  (((1U << 0x4) - 1U) << CCP_MODE_SHIFT)
// CCPxCAP register field definitions
#define CCP_CTS_SHIFT 0x0 
#define CCP_CTS_MASK  (((1U << 0x3) - 1U) << CCP_CTS_SHIFT)

/*---------------------------------------------------------------------------------------*/
/*UTMRA and UTMRB------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

#define TU16_BASE_ADDRESS 0x012C
#define TU16_TUCHAIN_ADDRESS 0x012B
#define TU16_OFFSET_NEXT_MODULE 0xE

#define TU16_CON0_OFFSET 0x0
#define TU16_CON1_OFFSET 0x1
#define TU16_HLT_OFFSET 0x2
#define TU16_PS_OFFSET 0x3
#define TU16_TMR_OFFSET 0x4
#define TU16_CR_OFFSET 0x5
#define TU16_PR_OFFSET 0x8
#define TU16_CLK_OFFSET 0xC
#define TU16_ERS_OFFSET 0xD
// TUxyCON0 Timer Control Register 0
#define TU16_CON0_ON_SHIFT        0x7
#define TU16_CON0_CPOL_SHIFT      0x6
#define TU16_CON0_OM_SHIFT        0x5
#define TU16_CON0_OPOL_SHIFT      0x4
#define TU16_CON0_RDSEL_SHIFT     0x3
#define TU16_CON0_PRIE_SHIFT      0x2
#define TU16_CON0_ZIE_SHIFT       0x1
#define TU16_CON0_CIE_SHIFT       0x0

#define TU16_CON0_ON_MASK        BIT(TU16_CON0_ON_SHIFT)
#define TU16_CON0_CPOL_MASK      BIT(TU16_CON0_CPOL_SHIFT)
#define TU16_CON0_OM_MASK        BIT(TU16_CON0_OM_SHIFT)
#define TU16_CON0_OPOL_MASK      BIT(TU16_CON0_OPOL_SHIFT)
#define TU16_CON0_RDSEL_MASK     BIT(TU16_CON0_RDSEL_SHIFT)
#define TU16_CON0_PRIE_MASK      BIT(TU16_CON0_PRIE_SHIFT)
#define TU16_CON0_ZIE_MASK       BIT(TU16_CON0_ZIE_SHIFT)
#define TU16_CON0_CIE_MASK       BIT(TU16_CON0_CIE_SHIFT)
// TUxyCON1 Timer Control Register 1
#define TU16_CON1_RUN_SHIFT       7
#define TU16_CON1_OSEN_SHIFT      6
#define TU16_CON1_CLR_SHIFT       5
#define TU16_CON1_LIMIT_SHIFT     4
#define TU16_CON1_CAPT_SHIFT      3
#define TU16_CON1_PRIF_SHIFT      2
#define TU16_CON1_ZIF_SHIFT       1
#define TU16_CON1_CIF_SHIFT       0

#define TU16_CON1_RUN_MASK       BIT(TU16_CON1_RUN_SHIFT)
#define TU16_CON1_OSEN_MASK      BIT(TU16_CON1_OSEN_SHIFT)
#define TU16_CON1_CLR_MASK       BIT(TU16_CON1_CLR_SHIFT)
#define TU16_CON1_LIMIT_MASK     BIT(TU16_CON1_LIMIT_SHIFT)
#define TU16_CON1_CAPT_MASK      BIT(TU16_CON1_CAPT_SHIFT)
#define TU16_CON1_PRIF_MASK      BIT(TU16_CON1_PRIF_SHIFT)
#define TU16_CON1_ZIF_MASK       BIT(TU16_CON1_ZIF_SHIFT)
#define TU16_CON1_CIF_MASK       BIT(TU16_CON1_CIF_SHIFT)
// TUxyHLT Hardware Limit Timer Control Register
#define TU16_HLT_EPOL_SHIFT       7
#define TU16_HLT_CSYNC_SHIFT      6
#define TU16_HLT_START_SHIFT      4
#define TU16_HLT_RESET_SHIFT      2
#define TU16_HLT_STOP_SHIFT       0

#define TU16_HLT_EPOL_MASK       BIT(TU16_HLT_EPOL_SHIFT)
#define TU16_HLT_CSYNC_MASK      BIT(TU16_HLT_CSYNC_SHIFT)
#define TU16_HLT_START_MASK      FIELD(2, TU16_HLT_START_SHIFT)
#define TU16_HLT_RESET_MASK      FIELD(2, TU16_HLT_RESET_SHIFT)
#define TU16_HLT_STOP_MASK       FIELD(2, TU16_HLT_STOP_SHIFT)
// TUxyPS Clock Prescaler Register
#define TU16_PS_SHIFT             0
#define TU16_PS_MASK             FIELD(8, TU16_PS_SHIFT)
// TUxyTMR Timer Counter Register
#define TU16_TMR_L_SHIFT          0
#define TU16_TMR_H_SHIFT          8

#define TU16_TMR_L_MASK          FIELD(8, TU16_TMR_L_SHIFT)
#define TU16_TMR_H_MASK          FIELD(8, TU16_TMR_H_SHIFT)
// TUxyCR Timer Capture Register
#define TU16_CR_L_SHIFT           0
#define TU16_CR_H_SHIFT           8

#define TU16_CR_L_MASK           FIELD(8, TU16_CR_L_SHIFT)
#define TU16_CR_H_MASK           FIELD(8, TU16_CR_H_SHIFT)
// TUxyPR Timer Period Register
#define TU16_PR_L_SHIFT           0
#define TU16_PR_H_SHIFT           8

#define TU16_PR_L_MASK           FIELD(8, TU16_PR_L_SHIFT)
#define TU16_PR_H_MASK           FIELD(8, TU16_PR_H_SHIFT)
// TUxyCLK Clock Input Selector
#define TU16_CLK_SHIFT            0
#define TU16_CLK_MASK            FIELD(5, TU16_CLK_SHIFT)
// TUxyERS External Reset Selector
#define TU16_ERS_SHIFT            0
#define TU16_ERS_MASK            FIELD(6, TU16_ERS_SHIFT)
#endif // REGISTERS_H

/*---------------------------------------------------------------------------------------*/
/*TMR0-----------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

#define TMR0_BASE_ADDRESS 0x103
#define TMR0_TMRL_OFFSET 0x0
#define TMR0_TMRH_OFFSET 0x1
#define TMR0_CON0_OFFSET 0x2
#define TMR0_CON1_OFFSET 0x3

// TMR0CON0
#define TMR0_CON0_EN_SHIFT      7
#define TMR0_CON0_OUT_SHIFT     5
#define TMR0_CON0_MD16_SHIFT    4
#define TMR0_CON0_OUTPS_SHIFT   0

#define TMR0_CON0_EN_MASK       BIT(TMR0_CON0_EN_SHIFT)
#define TMR0_CON0_OUT_MASK      BIT(TMR0_CON0_OUT_SHIFT)
#define TMR0_CON0_MD16_MASK     BIT(TMR0_CON0_MD16_SHIFT)
#define TMR0_CON0_OUTPS_MASK    FIELD(4,TMR0_CON0_OUTPS_SHIFT)

// TMRCON1
#define TMR0_CON1_CS_SHIFT      5
#define TMR0_CON1_ASYNC_SHIFT   4
#define TMR0_CON1_CKPS_SHIFT    0

#define TMR0_CON1_CS_MASK       FIELD(3,TMR0_CON1_CS_SHIFT)
#define TMR0_CON1_ASYNC_MASK    BIT(TMR0_CON1_ASYNC_SHIFT)
#define TMR0_CON1_CKSPS_MASK    FIELD(4,TMR0_CON1_CKPS_SHIFT)

/*---------------------------------------------------------------------------------------*/
/*TIMER1---------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/

#define TMR1_BASE_ADDRESS      0x0107
#define TMR1_TMRL_OFFSET       0x0
#define TMR1_TMRH_OFFSET       0x1
#define TMR1_CON_OFFSET        0x2
#define TMR1_GCON_OFFSET       0x3
#define TMR1_GATE_OFFSET       0x4
#define TMR1_CLK_OFFSET        0x5

// TMR1CON
#define TMR1_CON_CKPS_SHIFT    5
#define TMR1_CON_SYNC_SHIFT    4
#define TMR1_CON_RD16_SHIFT    2
#define TMR1_CON_ON_SHIFT      0

#define TMR1_CON_CKPS_MASK     FIELD(2, TMR1_CON_CKPS_SHIFT)
#define TMR1_CON_SYNC_MASK     BIT(TMR1_CON_SYNC_SHIFT)
#define TMR1_CON_RD16_MASK     BIT(TMR1_CON_RD16_SHIFT)
#define TMR1_CON_ON_MASK       BIT(TMR1_CON_ON_SHIFT)

// TMR1GCON

#define TMR1_GCON_GE_SHIFT        7
#define TMR1_GCON_GPOL_SHIFT      6
#define TMR1_GCON_GTM_SHIFT       5
#define TMR1_GCON_GSPM_SHIFT      4
#define TMR1_GCON_GGO_SHIFT       3
#define TMR1_GCON_GVAL_SHIFT      2

#define TMR1_GCON_GE_MASK         BIT(TMR1_GCON_GE_SHIFT)
#define TMR1_GCON_GPOL_MASK       BIT(TMR1_GCON_GPOL_SHIFT)
#define TMR1_GCON_GTM_MASK        BIT(TMR1_GCON_GTM_SHIFT)
#define TMR1_GCON_GSPM_MASK       BIT(TMR1_GCON_GSPM_SHIFT)
#define TMR1_GCON_GGO_MASK        BIT(TMR1_GCON_GGO_SHIFT)
#define TMR1_GCON_GVAL_MASK       BIT(TMR1_GCON_GVAL_SHIFT)

// TMRGSS
#define TMR1_GATE_GSS_SHIFT     0
#define TMR1_GATE_GSS_MASK      FIELD(4, TMR1_GATE_GSS_SHIFT)

//  TMR1CLK
#define TMR1_CLK_CS_SHIFT       0
#define TMR1_CLK_CS_MASK        FIELD(4, TMR1_CLK_CS_SHIFT)