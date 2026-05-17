#ifndef REGISTERS_H
#define REGISTERS_H

#define REG8(base, offset) ((volatile uint8_t*)(base + offset))
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
        // switch this whole ting out with the xc.h definitions
/*---------------------------------------------------------------------------------------*/
/*PIE and PIR ---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
#define PIE1_ADDRESS 0x474
#define PIE2_ADDRESS 0x475
#define PIE3_ADDRESS 0x476
#define PIE4_ADDRESS 0x477

#define PIE4_TMR2IE_SHIFT 0x0
#define PIE4_TMR2IE_MASK BIT(PIE4_TMR2IE_SHIFT)
#define PIE4_TMR4IE_SHIFT 0x1
#define PIE4_TMR4IE_MASK BIT(PIE4_TMR4IE_SHIFT)

#define PIE3_TMR0IE_SHIFT 0x5
#define PIE3_TMR0IE_MASK BIT(PIE3_TMR0IE_SHIFT)
//------------------------------------------------

#define PIR1_ADDRESS 0x46A
#define PIR2_ADDRESS 0x46B
#define PIR3_ADDRESS 0x46C
#define PIR4_ADDRESS 0x46D

#define PIR4_TMR2IF_SHIFT 0x0
#define PIR4_TMR4IF_MASK BIT(PIR4_TMR4IF_SHIFT)
#define PIR4_TMR4IF_SHIFT 0x1
#define PIR4_TMR2IF_MASK BIT(PIR4_TMR2IF_SHIFT)

#define PIR3_TMR0IF_SHIFT 0x5
#define PIR3_TMR0IF_MASK BIT(PIR3_TMR0IF_SHIFT)
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

/*---------------------------------------------------------------------------------------*/
/*CLCx---------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
#define CLC_BASE_ADDRESS          0x01A3
#define CLC_OFFSET_NEXT_MODULE    0x0C
#define CLC_NUM_MODULES           0x4

#define CLC_DATA_OFFSET           0x0
#define CLC_SELECT_OFFSET         0x1
#define CLC_CON_OFFSET            0x2
#define CLC_POL_OFFSET            0x3
#define CLC_SEL0_OFFSET           0x4
#define CLC_SEL1_OFFSET           0x5
#define CLC_SEL2_OFFSET           0x6
#define CLC_SEL3_OFFSET           0x7
#define CLC_GLS0_OFFSET           0x8
#define CLC_GLS1_OFFSET           0x9
#define CLC_GLS2_OFFSET           0xA
#define CLC_GLS3_OFFSET           0xB

// CLCxDATA
#define CLC_DATA_OUT0_SHIFT       0
#define CLC_DATA_OUT1_SHIFT       1
#define CLC_DATA_OUT2_SHIFT       2
#define CLC_DATA_OUT3_SHIFT       3

#define CLC_DATA_OUT0_MASK        BIT(CLC_DATA_OUT0_SHIFT)
#define CLC_DATA_OUT1_MASK        BIT(CLC_DATA_OUT1_SHIFT)
#define CLC_DATA_OUT2_MASK        BIT(CLC_DATA_OUT2_SHIFT)
#define CLC_DATA_OUT3_MASK        BIT(CLC_DATA_OUT3_SHIFT)

//CLCxSELECT
#define CLC_SELECT_SLCT_SHIFT     0
#define CLC_SELECT_SLCT_MASK      FIELD(3, CLC_SELECT_SLCT_SHIFT)

//CLCxCON
#define CLC_CON_MODE_SHIFT        0
#define CLC_CON_INTN_SHIFT        3
#define CLC_CON_INTP_SHIFT        4
#define CLC_CON_OUT_SHIFT         6
#define CLC_CON_EN_SHIFT          7

#define CLC_CON_MODE_MASK         FIELD(3, CLC_CON_MODE_SHIFT)
#define CLC_CON_INTN_MASK         BIT(CLC_CON_INTN_SHIFT)
#define CLC_CON_INTP_MASK         BIT(CLC_CON_INTP_SHIFT)
#define CLC_CON_OUT_MASK          BIT(CLC_CON_OUT_SHIFT)
#define CLC_CON_EN_MASK           BIT(CLC_CON_EN_SHIFT)

// CLCxPOL
#define CLC_POL_G1POL_SHIFT       0
#define CLC_POL_G2POL_SHIFT       1
#define CLC_POL_G3POL_SHIFT       2
#define CLC_POL_G4POL_SHIFT       3
#define CLC_POL_POL_SHIFT         7

#define CLC_POL_G1POL_MASK        BIT(CLC_POL_G1POL_SHIFT)
#define CLC_POL_G2POL_MASK        BIT(CLC_POL_G2POL_SHIFT)
#define CLC_POL_G3POL_MASK        BIT(CLC_POL_G3POL_SHIFT)
#define CLC_POL_G4POL_MASK        BIT(CLC_POL_G4POL_SHIFT)
#define CLC_POL_POL_MASK          BIT(CLC_POL_POL_SHIFT)

//CLCxSEL
#define CLC_SEL_D_SHIFT           0
#define CLC_SEL_D_MASK            FIELD(6, CLC_SEL_D_SHIFT)

//CLCxGLS
#define CLC_GLS_G1D1N_SHIFT       0
#define CLC_GLS_G1D1T_SHIFT       1
#define CLC_GLS_G1D2N_SHIFT       2
#define CLC_GLS_G1D2T_SHIFT       3
#define CLC_GLS_G1D3N_SHIFT       4
#define CLC_GLS_G1D3T_SHIFT       5
#define CLC_GLS_G1D4N_SHIFT       6
#define CLC_GLS_G1D4T_SHIFT       7

#define CLC_GLS_G1D1N_MASK        BIT(CLC_GLS_G1D1N_SHIFT)
#define CLC_GLS_G1D1T_MASK        BIT(CLC_GLS_G1D1T_SHIFT)
#define CLC_GLS_G1D2N_MASK        BIT(CLC_GLS_G1D2N_SHIFT)
#define CLC_GLS_G1D2T_MASK        BIT(CLC_GLS_G1D2T_SHIFT)
#define CLC_GLS_G1D3N_MASK        BIT(CLC_GLS_G1D3N_SHIFT)
#define CLC_GLS_G1D3T_MASK        BIT(CLC_GLS_G1D3T_SHIFT)
#define CLC_GLS_G1D4N_MASK        BIT(CLC_GLS_G1D4N_SHIFT)
#define CLC_GLS_G1D4T_MASK        BIT(CLC_GLS_G1D4T_SHIFT)

/*---------------------------------------------------------------------------------------*/
/*PWMx-----------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------*/
#define PWM_BASE_ADDR       0x0151
#define PWM_OFFSET_NEXT_MODULE  0x11
#define PWM_LOAD_OFFSET     0x0
#define PWM_EN_OFFSET       0x1
#define PWM_ERS_OFFSET      0x2
#define PWM_CLK_OFFSET      0x3
#define PWM_LDS_OFFSET      0x4
#define PWM_PRL_OFFSET      0x5
#define PWM_PRH_OFFSET      0x6
#define PWM_CPRE_OFFSET     0x7
#define PWM_PIPOS_OFFSET    0x8
#define PWM_CON_OFFSET      0xB
#define PWM_CFG_OFFSET      0xC
#define PWM_SAP1_OFFSET     0xD
#define PWM_SAP2_OFFSET     0xE

#define PWM_CON_EN_SHIFT        0x7
#define PWM_CON_LD_SHIFT        0x2
#define PWM_CON_ERSPOL_SHIFT    0x1
#define PWM_CON_ERSNOW_SHIFT    0x0

#define PWM_CON_EN_MASK      BIT(PWM_CON_EN_SHIFT)
#define PWM_CON_LD_MASK      BIT(PWM_CON_LD_SHIFT)
#define PWM_CON_ERSPOL_MASK  BIT(PWM_CON_ERSPOL_SHIFT)
#define PWM_CON_ERSNOW_MASK  BIT(PWM_CON_ERSNOW_SHIFT)

#define PWM_CFG_POL1_SHIFT      0x7
#define PWM_CFG_POL2_SHIFT      0x6
#define PWM_CFG_PPEN_SHIFT      0x3
#define PWM_CFG_MODE_SHIFT      0x0

#define PWM_CFG_POL1_MASK      BIT(PWM_CFG_POL1_SHIFT)
#define PWM_CFG_POL2_MASK      BIT(PWM_CFG_POL2_SHIFT)
#define PWM_CFG_PPEN_MASK      BIT(PWM_CFG_PPEN_SHIFT)
#define PWM_CFG_MODE_MASK      FIELD(3,PWM_CFG_MODE_SHIFT)

/* ============================================================
 * I2C REGISTER DEFINITIONS
 * ============================================================ */

#define I2C1_BASE_ADDR             0x01E4u

/* ============================================================
 * Register Offsets
 * ============================================================ */

#define I2C_STAT0_OFFSET           0x00u
#define I2C_STAT1_OFFSET           0x01u
#define I2C_CON0_OFFSET            0x02u
#define I2C_CON1_OFFSET            0x03u
#define I2C_CON2_OFFSET            0x04u
#define I2C_CON3_OFFSET            0x05u
#define I2C_PIR_OFFSET             0x06u
#define I2C_PIE_OFFSET             0x07u
#define I2C_ERR_OFFSET             0x08u
#define I2C_CNTL_OFFSET            0x09u
#define I2C_CNTH_OFFSET            0x0Au
#define I2C_RXB_OFFSET             0x0Bu
#define I2C_TXB_OFFSET             0x0Cu
#define I2C_ADB0_OFFSET            0x0Du
#define I2C_ADB1_OFFSET            0x0Eu
#define I2C_ADR0_OFFSET            0x0Fu
#define I2C_ADR1_OFFSET            0x10u
#define I2C_ADR2_OFFSET            0x11u
#define I2C_ADR3_OFFSET            0x12u
#define I2C_BTO_OFFSET             0x13u
#define I2C_BAUD_OFFSET            0x14u
#define I2C_CLK_OFFSET             0x15u
#define I2C_BTOC_OFFSET            0x16u

/* ============================================================
 * I2C1STAT0
 * ============================================================ */

#define I2C_STAT0_D_SHIFT          3u
#define I2C_STAT0_R_SHIFT          4u
#define I2C_STAT0_MMA_SHIFT        5u
#define I2C_STAT0_SMA_SHIFT        6u
#define I2C_STAT0_BFRE_SHIFT       7u

#define I2C_STAT0_D_MASK           BIT(I2C_STAT0_D_SHIFT)
#define I2C_STAT0_R_MASK           BIT(I2C_STAT0_R_SHIFT)
#define I2C_STAT0_MMA_MASK         BIT(I2C_STAT0_MMA_SHIFT)
#define I2C_STAT0_SMA_MASK         BIT(I2C_STAT0_SMA_SHIFT)
#define I2C_STAT0_BFRE_MASK        BIT(I2C_STAT0_BFRE_SHIFT)

/* ============================================================
 * I2C1STAT1
 * ============================================================ */

#define I2C_STAT1_CLRBF_SHIFT      3u
#define I2C_STAT1_RXBF_SHIFT       4u
#define I2C_STAT1_RXRE_SHIFT       5u
#define I2C_STAT1_TXBE_SHIFT       6u
#define I2C_STAT1_TXWE_SHIFT       7u

#define I2C_STAT1_CLRBF_MASK       BIT(I2C_STAT1_CLRBF_SHIFT)
#define I2C_STAT1_RXBF_MASK        BIT(I2C_STAT1_RXBF_SHIFT)
#define I2C_STAT1_RXRE_MASK        BIT(I2C_STAT1_RXRE_SHIFT)
#define I2C_STAT1_TXBE_MASK        BIT(I2C_STAT1_TXBE_SHIFT)
#define I2C_STAT1_TXWE_MASK        BIT(I2C_STAT1_TXWE_SHIFT)

/* ============================================================
 * I2C1CON0
 * ============================================================ */

#define I2C_CON0_MODE_SHIFT        0u
#define I2C_CON0_MDR_SHIFT         3u
#define I2C_CON0_CSTR_SHIFT        4u
#define I2C_CON0_S_SHIFT           5u
#define I2C_CON0_RSEN_SHIFT        6u
#define I2C_CON0_EN_SHIFT          7u

#define I2C_CON0_MODE_MASK         FIELD(3u, I2C_CON0_MODE_SHIFT)
#define I2C_CON0_MDR_MASK          BIT(I2C_CON0_MDR_SHIFT)
#define I2C_CON0_CSTR_MASK         BIT(I2C_CON0_CSTR_SHIFT)
#define I2C_CON0_S_MASK            BIT(I2C_CON0_S_SHIFT)
#define I2C_CON0_RSEN_MASK         BIT(I2C_CON0_RSEN_SHIFT)
#define I2C_CON0_EN_MASK           BIT(I2C_CON0_EN_SHIFT)

/* ============================================================
 * I2C1CON1
 * ============================================================ */

#define I2C_CON1_CSD_SHIFT         0u
#define I2C_CON1_TXU_SHIFT         1u
#define I2C_CON1_RXO_SHIFT         2u
#define I2C_CON1_P_SHIFT           3u
#define I2C_CON1_ACKT_SHIFT        4u
#define I2C_CON1_ACKSTAT_SHIFT     5u
#define I2C_CON1_ACKDT_SHIFT       6u
#define I2C_CON1_ACKCNT_SHIFT      7u

#define I2C_CON1_CSD_MASK          BIT(I2C_CON1_CSD_SHIFT)
#define I2C_CON1_TXU_MASK          BIT(I2C_CON1_TXU_SHIFT)
#define I2C_CON1_RXO_MASK          BIT(I2C_CON1_RXO_SHIFT)
#define I2C_CON1_P_MASK            BIT(I2C_CON1_P_SHIFT)
#define I2C_CON1_ACKT_MASK         BIT(I2C_CON1_ACKT_SHIFT)
#define I2C_CON1_ACKSTAT_MASK      BIT(I2C_CON1_ACKSTAT_SHIFT)
#define I2C_CON1_ACKDT_MASK        BIT(I2C_CON1_ACKDT_SHIFT)
#define I2C_CON1_ACKCNT_MASK       BIT(I2C_CON1_ACKCNT_SHIFT)

/* ============================================================
 * I2C1CON2
 * ============================================================ */

#define I2C_CON2_BFRET_SHIFT       0u
#define I2C_CON2_SDAHT_SHIFT       2u
#define I2C_CON2_ABD_SHIFT         4u
#define I2C_CON2_GCEN_SHIFT        5u
#define I2C_CON2_ACNT_SHIFT        7u

#define I2C_CON2_BFRET_MASK        FIELD(2u, I2C_CON2_BFRET_SHIFT)
#define I2C_CON2_SDAHT_MASK        FIELD(2u, I2C_CON2_SDAHT_SHIFT)
#define I2C_CON2_ABD_MASK          BIT(I2C_CON2_ABD_SHIFT)
#define I2C_CON2_GCEN_MASK         BIT(I2C_CON2_GCEN_SHIFT)
#define I2C_CON2_ACNT_MASK         BIT(I2C_CON2_ACNT_SHIFT)

/* ============================================================
 * I2C1CON3
 * ============================================================ */

#define I2C_CON3_ACNTMD_SHIFT      0u
#define I2C_CON3_FME_SHIFT         2u
#define I2C_CON3_BFREDR_SHIFT      7u

#define I2C_CON3_ACNTMD_MASK       FIELD(2u, I2C_CON3_ACNTMD_SHIFT)
#define I2C_CON3_FME_MASK          FIELD(2u, I2C_CON3_FME_SHIFT)
#define I2C_CON3_BFREDR_MASK       BIT(I2C_CON3_BFREDR_SHIFT)

/* ============================================================
 * I2C1PIR
 * ============================================================ */

#define I2C_PIR_SCIF_SHIFT         0u
#define I2C_PIR_RSCIF_SHIFT        1u
#define I2C_PIR_PCIF_SHIFT         2u
#define I2C_PIR_ADRIF_SHIFT        3u
#define I2C_PIR_WRIF_SHIFT         4u
#define I2C_PIR_ACKTIF_SHIFT       5u
#define I2C_PIR_CNTIF_SHIFT        6u

#define I2C_PIR_SCIF_MASK          BIT(I2C_PIR_SCIF_SHIFT)
#define I2C_PIR_RSCIF_MASK         BIT(I2C_PIR_RSCIF_SHIFT)
#define I2C_PIR_PCIF_MASK          BIT(I2C_PIR_PCIF_SHIFT)
#define I2C_PIR_ADRIF_MASK         BIT(I2C_PIR_ADRIF_SHIFT)
#define I2C_PIR_WRIF_MASK          BIT(I2C_PIR_WRIF_SHIFT)
#define I2C_PIR_ACKTIF_MASK        BIT(I2C_PIR_ACKTIF_SHIFT)
#define I2C_PIR_CNTIF_MASK         BIT(I2C_PIR_CNTIF_SHIFT)

/* ============================================================
 * I2C1PIE
 * ============================================================ */

#define I2C_PIE_SCIE_SHIFT         0u
#define I2C_PIE_RSCIE_SHIFT        1u
#define I2C_PIE_PCIE_SHIFT         2u
#define I2C_PIE_ADRIE_SHIFT        3u
#define I2C_PIE_WRIE_SHIFT         4u
#define I2C_PIE_ACKTIE_SHIFT       5u
#define I2C_PIE_CNTIE_SHIFT        6u

#define I2C_PIE_SCIE_MASK          BIT(I2C_PIE_SCIE_SHIFT)
#define I2C_PIE_RSCIE_MASK         BIT(I2C_PIE_RSCIE_SHIFT)
#define I2C_PIE_PCIE_MASK          BIT(I2C_PIE_PCIE_SHIFT)
#define I2C_PIE_ADRIE_MASK         BIT(I2C_PIE_ADRIE_SHIFT)
#define I2C_PIE_WRIE_MASK          BIT(I2C_PIE_WRIE_SHIFT)
#define I2C_PIE_ACKTIE_MASK        BIT(I2C_PIE_ACKTIE_SHIFT)
#define I2C_PIE_CNTIE_MASK         BIT(I2C_PIE_CNTIE_SHIFT)

/* ============================================================
 * I2C1ERR
 * ============================================================ */

#define I2C_ERR_NACKIE_SHIFT       0u
#define I2C_ERR_BCLIE_SHIFT        1u
#define I2C_ERR_BTOIE_SHIFT        2u
#define I2C_ERR_NACKIF_SHIFT       4u
#define I2C_ERR_BCLIF_SHIFT        5u
#define I2C_ERR_BTOIF_SHIFT        6u

#define I2C_ERR_NACKIE_MASK        BIT(I2C_ERR_NACKIE_SHIFT)
#define I2C_ERR_BCLIE_MASK         BIT(I2C_ERR_BCLIE_SHIFT)
#define I2C_ERR_BTOIE_MASK         BIT(I2C_ERR_BTOIE_SHIFT)
#define I2C_ERR_NACKIF_MASK        BIT(I2C_ERR_NACKIF_SHIFT)
#define I2C_ERR_BCLIF_MASK         BIT(I2C_ERR_BCLIF_SHIFT)
#define I2C_ERR_BTOIF_MASK         BIT(I2C_ERR_BTOIF_SHIFT)

/* ============================================================
 * I2C1BTO
 * ============================================================ */

#define I2C_BTO_TOTIME_SHIFT       0u
#define I2C_BTO_TOBY32_SHIFT       6u
#define I2C_BTO_TOREC_SHIFT        7u

#define I2C_BTO_TOTIME_MASK        FIELD(6u, I2C_BTO_TOTIME_SHIFT)
#define I2C_BTO_TOBY32_MASK        BIT(I2C_BTO_TOBY32_SHIFT)
#define I2C_BTO_TOREC_MASK         BIT(I2C_BTO_TOREC_SHIFT)

/* ============================================================
 * I2C1BAUD
 * ============================================================ */

#define I2C_BAUD_BAUD_SHIFT        0u
#define I2C_BAUD_BAUD_MASK         FIELD(8u, I2C_BAUD_BAUD_SHIFT)

/* ============================================================
 * I2C1CLK
 * ============================================================ */

#define I2C_CLK_CLK_SHIFT          0u
#define I2C_CLK_CLK_MASK           FIELD(5u, I2C_CLK_CLK_SHIFT)

#endif // REGISTERS_H
