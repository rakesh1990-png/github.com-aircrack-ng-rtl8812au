/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================

#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8814A_SUPPORT == 1)
#ifdef PHYDM_PRIMARY_CCA
VOID
odm_Write_Dynamic_CCA_8814A(
	struct dm_struct    *dm,
	u8			CurrentMFstate
	)
{
	struct phydm_pri_cca_struct*		PrimaryCCA = &(dm->dm_pri_cca);

	if (PrimaryCCA->MF_state != CurrentMFstate){

		ODM_SetBBReg(dm, ODM_REG_L1SBD_PD_CH_11N, BIT(8)|BIT(7), CurrentMFstate);
	}

	PrimaryCCA->MF_state = CurrentMFstate;

}

VOID
odm_PrimaryCCA_Check_Init_8814A(
	struct dm_struct    *dm)
{
#if ((DM_ODM_SUPPORT_TYPE == ODM_WIN) || (DM_ODM_SUPPORT_TYPE == ODM_AP))
	void *		padapter = dm->adapter;
	struct phydm_pri_cca_struct*		PrimaryCCA = &(dm->dm_pri_cca);
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(padapter);

	hal_data->RTSEN = 0;
	PrimaryCCA->DupRTS_flag = 0;
	PrimaryCCA->intf_flag = 0;
	PrimaryCCA->intf_type = 0;
	PrimaryCCA->Monitor_flag = 0;
	PrimaryCCA->PriCCA_flag = 0;
	PrimaryCCA->CH_offset = 0;
	PrimaryCCA->MF_state = 0;
#endif /*((DM_ODM_SUPPORT_TYPE==ODM_WIN) ||(DM_ODM_SUPPORT_TYPE==ODM_AP)) */
}

VOID
odm_DynamicPrimaryCCA_Check_8814A(
	struct dm_struct    *dm
	)
{
	if(dm->SupportICType != ODM_RTL8814A)
		return;

	switch	(dm->SupportPlatform)
	{
		case	ODM_WIN:

#if(DM_ODM_SUPPORT_TYPE==ODM_WIN)
			odm_DynamicPrimaryCCAMP_8814A(dm);
#endif
			break;

		case	ODM_CE:
#if(DM_ODM_SUPPORT_TYPE==ODM_CE)

#endif
			break;

		case	ODM_AP:
#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
			odm_DynamicPrimaryCCAAP_8814A(dm);
#endif
			break;
		}

}


#if(DM_ODM_SUPPORT_TYPE==ODM_WIN)

VOID
odm_DynamicPrimaryCCAMP_8814A(
	struct dm_struct    *dm
	)
{
	void *		padapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(padapter);
	Pfalse_ALARM_STATISTICS		FalseAlmCnt = (Pfalse_ALARM_STATISTICS)PhyDM_Get_Structure( dm, PHYDM_FALSEALMCNT);
	struct phydm_pri_cca_struct*		PrimaryCCA = &(dm->dm_pri_cca);
	boolean			Is40MHz = false;
	u8Byte			OFDM_CCA, OFDM_FA, BW_USC_Cnt, BW_LSC_Cnt;
	u8			SecCHOffset;
	u8			CurMFstate;
	static u8		CountDown = Monitor_TIME;

	OFDM_CCA = FalseAlmCnt->Cnt_OFDM_CCA;
	OFDM_FA = FalseAlmCnt->Cnt_Ofdm_fail;
	BW_USC_Cnt = FalseAlmCnt->Cnt_BW_USC;
	BW_LSC_Cnt = FalseAlmCnt->Cnt_BW_LSC;
	PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: OFDM CCA=%d\n", OFDM_CCA));
	PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: OFDM FA=%d\n", OFDM_FA));
	PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: BW_USC=%d\n", BW_USC_Cnt));
	PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: BW_LSC=%d\n", BW_LSC_Cnt));
       Is40MHz = *(dm->pBandWidth);
	SecCHOffset = *(dm->pSecChOffset);		// NIC: 2: sec is below,  1: sec is above
	//DbgPrint("8814A: SecCHOffset = %d\n", SecCHOffset);
	if(!dm->bLinked){
		return;
	}
	else{

		if(Is40MHz){
			PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: Cont Down= %d\n", CountDown));
			PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: Primary_CCA_flag=%d\n", PrimaryCCA->PriCCA_flag));
			PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: Intf_Type=%d\n", PrimaryCCA->intf_type));
			PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: Intf_flag=%d\n", PrimaryCCA->intf_flag ));
			PHYDM_DBG(dm,ODM_COMP_DYNAMIC_PRICCA,  ("8814A: Duplicate RTS Flag=%d\n", PrimaryCCA->DupRTS_flag));
			//DbgPrint("8814A RTS_EN=%d\n", hal_data->RTSEN);

			if(PrimaryCCA->PriCCA_flag == 0){

				if(SecCHOffset == 2){    // Primary channel is above   NOTE: duplicate CTS can remove this condition

					if((OFDM_CCA > OFDMCCA_TH) && (BW_LSC_Cnt>(BW_USC_Cnt + BW_Ind_Bias))
						&& (OFDM_FA>(OFDM_CCA>>1))){

						PrimaryCCA->intf_type = 1;
						PrimaryCCA->intf_flag = 1;
						CurMFstate = MF_USC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
						PrimaryCCA->PriCCA_flag = 1;
					}
					else if((OFDM_CCA > OFDMCCA_TH) && (BW_LSC_Cnt>(BW_USC_Cnt + BW_Ind_Bias))
						&& (OFDM_FA < (OFDM_CCA>>1))){

 						PrimaryCCA->intf_type = 2;
						PrimaryCCA->intf_flag = 1;
						CurMFstate = MF_USC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
						PrimaryCCA->PriCCA_flag = 1;
						PrimaryCCA->DupRTS_flag = 1;
						hal_data->RTSEN = 1;
					}
					else{

						PrimaryCCA->intf_type = 0;
						PrimaryCCA->intf_flag = 0;
						CurMFstate = MF_USC_LSC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
						hal_data->RTSEN = 0;
						PrimaryCCA->DupRTS_flag = 0;
					}

				}
				else if (SecCHOffset == 1){

					if((OFDM_CCA > OFDMCCA_TH) && (BW_USC_Cnt > (BW_LSC_Cnt + BW_Ind_Bias))
						&& (OFDM_FA > (OFDM_CCA>>1))){

						PrimaryCCA->intf_type = 1;
						PrimaryCCA->intf_flag = 1;
						CurMFstate = MF_LSC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
						PrimaryCCA->PriCCA_flag = 1;
					}
					else if((OFDM_CCA > OFDMCCA_TH) && (BW_USC_Cnt>(BW_LSC_Cnt + BW_Ind_Bias))
						&& (OFDM_FA < (OFDM_CCA>>1))){

 						PrimaryCCA->intf_type = 2;
						PrimaryCCA->intf_flag = 1;
						CurMFstate = MF_LSC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
						PrimaryCCA->PriCCA_flag = 1;
						PrimaryCCA->DupRTS_flag = 1;
						hal_data->RTSEN = 1;
					}
					else{

						PrimaryCCA->intf_type = 0;
						PrimaryCCA->intf_flag = 0;
						CurMFstate = MF_USC_LSC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
						hal_data->RTSEN = 0;
						PrimaryCCA->DupRTS_flag = 0;
					}

				}

			}
			else{	// PrimaryCCA->PriCCA_flag==1

				CountDown--;
				if(CountDown == 0){
					CountDown = Monitor_TIME;
					PrimaryCCA->PriCCA_flag = 0;
					CurMFstate = MF_USC_LSC;
					odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);   /* default*/
					hal_data->RTSEN = 0;
					PrimaryCCA->DupRTS_flag = 0;
					PrimaryCCA->intf_type = 0;
					PrimaryCCA->intf_flag = 0;
				}

			}

		}
		else{

			return;
		}
	}

}

#elif(DM_ODM_SUPPORT_TYPE == ODM_AP)

VOID
odm_DynamicPrimaryCCAAP_8814A(
	struct dm_struct    *dm
	)
{
	void *	adapter = dm->adapter;
	prtl8192cd_priv	priv = dm->priv;
	Pfalse_ALARM_STATISTICS		FalseAlmCnt = (Pfalse_ALARM_STATISTICS)PhyDM_Get_Structure( dm, PHYDM_FALSEALMCNT);
	struct phydm_pri_cca_struct*		PrimaryCCA = &(dm->dm_pri_cca);

	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(adapter);
	u8 		i;
	static u4Byte	Count_Down = Monitor_TIME;
	boolean		STA_BW = false, STA_BW_pre = false, STA_BW_TMP = false;
	boolean		bConnected = false;
	boolean		Is40MHz = false;
	u8		SecCHOffset;
	u8		CurMFstate;
	PSTA_INFO_T		pstat;

	Is40MHz = *(dm->pBandWidth);
	SecCHOffset = *(dm->pSecChOffset);		// AP: 1: sec is below,  2: sec is above


	for(i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++){
		pstat = dm->pODM_StaInfo[i];
		if(IS_STA_VALID(pstat)){

			STA_BW_TMP = pstat->tx_bw;
			if(STA_BW_TMP > STA_BW){
				STA_BW = STA_BW_TMP;
			}
			bConnected = true;
		}
	}

	if(Is40MHz){

		if(PrimaryCCA->PriCCA_flag == 0){

			if(bConnected){

				if(STA_BW == 0){   //2 STA BW=20M

					PrimaryCCA->PriCCA_flag = 1;
					if(SecCHOffset==1){
						CurMFstate = MF_USC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
					}
					else if(SecCHOffset==2){
						CurMFstate = MF_USC;
						odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
					}
				}
				else{     		 	//2  STA BW=40M
					if(PrimaryCCA->intf_flag == 0){

						odm_Intf_Detection(dm);
					}
					else{	// intf_flag = 1

						if(PrimaryCCA->intf_type == 1){

							if(PrimaryCCA->CH_offset == 1){

								CurMFstate = MF_USC;
								if(SecCHOffset == 1){  // AP,  1: primary is above  2: primary is below
									odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
								}
							}
							else if(PrimaryCCA->CH_offset == 2){

								CurMFstate = MF_LSC;
								if(SecCHOffset == 2){
									odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
								}
							}
						}
						else if(PrimaryCCA->intf_type==2){

							if(PrimaryCCA->CH_offset==1){

								//ODM_SetBBReg(dm, ODM_REG_L1SBD_PD_CH_11N, BIT(8)|BIT(7), MF_USC);
								hal_data->RTSEN = 1;
							}
							else if(PrimaryCCA->CH_offset==2){

								//ODM_SetBBReg(dm, ODM_REG_L1SBD_PD_CH_11N, BIT(8)|BIT(7), MF_LSC);
								hal_data->RTSEN = 1;
							}

						}
					}
				}

			}
			else{		// disconnected  interference detection

				odm_Intf_Detection(dm);
			}// end of disconnected


		}
		else{	// PrimaryCCA->PriCCA_flag == 1

			if(STA_BW==0){

				STA_BW_pre = STA_BW;
				return;
			}

			Count_Down--;
			if((Count_Down == 0) || ((STA_BW & STA_BW_pre) != 1)){

				Count_Down = Monitor_TIME;
				PrimaryCCA->PriCCA_flag = 0;
				PrimaryCCA->intf_type = 0;
				PrimaryCCA->intf_flag = 0;
				CurMFstate = MF_USC_LSC;
				odm_Write_Dynamic_CCA_8814A(dm, CurMFstate); /* default*/
				hal_data->RTSEN = 0;

			}

		}

		STA_BW_pre = STA_BW;

	}
	else{
		//2 Reset
		odm_PrimaryCCA_Check_Init(dm);
		CurMFstate = MF_USC_LSC;
		odm_Write_Dynamic_CCA_8814A(dm, CurMFstate);
		Count_Down = Monitor_TIME;
	}

}


VOID
odm_Intf_Detection_8814A(
	struct dm_struct    *dm
	)
{
	Pfalse_ALARM_STATISTICS		FalseAlmCnt = (Pfalse_ALARM_STATISTICS)PhyDM_Get_Structure( dm, PHYDM_FALSEALMCNT);
	struct phydm_pri_cca_struct*					PrimaryCCA = &(dm->dm_pri_cca);

	if((FalseAlmCnt->Cnt_OFDM_CCA>OFDMCCA_TH)
		&&(FalseAlmCnt->Cnt_BW_LSC>(FalseAlmCnt->Cnt_BW_USC+BW_Ind_Bias))){

		PrimaryCCA->intf_flag = 1;
		PrimaryCCA->CH_offset = 1;  //  1:LSC, 2:USC
		if(FalseAlmCnt->Cnt_Ofdm_fail>(FalseAlmCnt->Cnt_OFDM_CCA>>1)){
			PrimaryCCA->intf_type = 1;
		}
		else{
			PrimaryCCA->intf_type = 2;
		}
	}
	else if((FalseAlmCnt->Cnt_OFDM_CCA>OFDMCCA_TH)
		&&(FalseAlmCnt->Cnt_BW_USC>(FalseAlmCnt->Cnt_BW_LSC+BW_Ind_Bias))){

		PrimaryCCA->intf_flag = 1;
		PrimaryCCA->CH_offset = 2;  //  1:LSC, 2:USC
		if(FalseAlmCnt->Cnt_Ofdm_fail>(FalseAlmCnt->Cnt_OFDM_CCA>>1)){
			PrimaryCCA->intf_type = 1;
		}
		else{
			PrimaryCCA->intf_type = 2;
		}
	}
	else{
		PrimaryCCA->intf_flag = 0;
		PrimaryCCA->intf_type = 0;
		PrimaryCCA->CH_offset = 0;
	}

}

#endif
#endif /* #ifdef PHYDM_PRIMARY_CCA */

u8
phydm_spur_nbi_setting_8814a(
	struct dm_struct    *dm
	)
{
	u8	set_result = 0;

	/*dm->pChannel means central frequency, so we can use 20M as input*/
	if (dm->rfe_type == 0 || dm->rfe_type == 1 || dm->rfe_type == 6) {
		/*channel asked by RF Jeff*/
		if (*dm->channel == 14)
			set_result = phydm_nbi_setting(dm,	FUNC_ENABLE, *dm->channel, 40, 2480, PHYDM_DONT_CARE);
		else if (*dm->channel >= 4 || *dm->channel <= 8)
			set_result = phydm_nbi_setting(dm,	FUNC_ENABLE, *dm->channel, 40, 2440, PHYDM_DONT_CARE);
		else
			set_result = phydm_nbi_setting(dm,	FUNC_ENABLE, *dm->channel, 40, 2440, PHYDM_DONT_CARE);
	}
	PHYDM_DBG(dm, ODM_COMP_COMMON,  ("%s, set_result = 0x%d, pChannel = %d\n", __func__, set_result, *dm->channel));
	//printk("%s, set_result = 0x%d, pChannel = %d\n", __func__, set_result, *dm->channel);
	dm->nbi_set_result = set_result;
	return set_result;

}

void odm_hw_setting_8814a(
	struct dm_struct	*dm
	)
{
#ifdef PHYDM_PRIMARY_CCA
	odm_PrimaryCCA_Check_Init_8814A(dm);
	odm_DynamicPrimaryCCA_Check_8814A(dm);
	odm_Intf_Detection_8814A(dm);
#endif
}


#ifdef DYN_ANT_WEIGHTING_SUPPORT
void phydm_dynamic_ant_weighting_8814a(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u8 rssi_l2h = 43, rssi_h2l = 37;
	u8 reg_8;

    PHYDM_DBG(dm, ODM_COMP_API, "PEC DBD: non-existing function start.%s\n",__PRETTY_FUNCTION__);

	if (dm->is_disable_dym_ant_weighting)
		return;

	if (*dm->channel <= 14) {
		if (dm->rssi_min >= rssi_l2h) {
			odm_set_bb_reg(dm, R_0x854, BIT(3), 0x1);
			odm_set_bb_reg(dm, R_0x850, BIT(21) | BIT(22), 0x0);

			/*equal weighting*/
			reg_8 = (u8)odm_get_bb_reg(dm, R_0xf94, BIT(0) | BIT(1) | BIT(2));
			PHYDM_DBG(dm, ODM_COMP_API,
				  "Equal weighting ,rssi_min = %d\n, 0xf94[2:0] = 0x%x\n",
				  dm->rssi_min, reg_8);
		} else if (dm->rssi_min <= rssi_h2l) {
			odm_set_bb_reg(dm, R_0x854, BIT(3), 0x1);
			odm_set_bb_reg(dm, R_0x850, BIT(21) | BIT(22), 0x1);

			/*fix sec_min_wgt = 1/2*/
			reg_8 = (u8)odm_get_bb_reg(dm, R_0xf94, BIT(0) | BIT(1) | BIT(2));
			PHYDM_DBG(dm, ODM_COMP_API,
				  "AGC weighting ,rssi_min = %d\n, 0xf94[2:0] = 0x%x\n",
				  dm->rssi_min, reg_8);
		}
	} else {
		odm_set_bb_reg(dm, R_0x854, BIT(3), 0x1);
		odm_set_bb_reg(dm, R_0x850, BIT(21) | BIT(22), 0x1);

		reg_8 = (u8)odm_get_bb_reg(dm, R_0xf94, BIT(0) | BIT(1) | BIT(2));
		PHYDM_DBG(dm, ODM_COMP_API,
			  "AGC weighting ,rssi_min = %d\n, 0xf94[2:0] = 0x%x\n",
			  dm->rssi_min, reg_8);
		/*fix sec_min_wgt = 1/2*/
	}
    PHYDM_DBG(dm, ODM_COMP_API, "PEC DBD: non-existing function end.%s\n",__PRETTY_FUNCTION__);
}
#endif

void phydm_hwsetting_8814a(struct dm_struct *dm)
{
    PHYDM_DBG(dm, ODM_COMP_API, "PEC DBD: non-existing function start.%s\n",__PRETTY_FUNCTION__);
	phydm_dynamic_ant_weighting(dm);
    PHYDM_DBG(dm, ODM_COMP_API, "PEC DBD: non-existing function end.%s\n",__PRETTY_FUNCTION__);
}

#endif		// RTL8814A_SUPPORT == 1
