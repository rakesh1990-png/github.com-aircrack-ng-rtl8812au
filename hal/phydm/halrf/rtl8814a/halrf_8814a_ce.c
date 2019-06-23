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

#include "mp_precomp.h"
#include "../../phydm_precomp.h"



/*---------------------------Define Local Constant---------------------------*/
// 2010/04/25 MH Define the max tx power tracking tx agc power.
#define		ODM_TXPWRTRACK_MAX_IDX_8814A		6

/*---------------------------Define Local Constant---------------------------*/

//3============================================================
//3 Tx Power Tracking
//3============================================================


// Add CheckRFGainOffset By YuChen to make sure that RF gain offset will not over upperbound 4'b1010

u8
CheckRFGainOffset(
	struct dm_struct    *dm,
	enum pwrtrack_method 	Method,
	u8				RFPath
	)
{
	s1Byte	UpperBound = 10, LowerBound = -5; // 4'b1010 = 10
	s1Byte	Final_RF_Index = 0;
	boolean	bPositive = false;
	u32	bit_mask = 0;
	u8	Final_OFDM_Swing_Index = 0, TxScalingUpperBound = 28, TxScalingLowerBound = 4;// upper bound +2dB, lower bound -10dB
	struct dm_rf_calibration_struct  *  cali_info = &(dm->rf_calibrate_info);
	if(Method == MIX_MODE)	//normal Tx power tracking
	{
		PHYDM_DBG(dm,DBG_COMP_MCC, ("is 8814 MP chip\n"));
		bit_mask = BIT(19);
		cali_info->absolute_ofdm_swing_idx[RFPath] = cali_info->absolute_ofdm_swing_idx[RFPath] + cali_info->kfree_offset[RFPath];


		PHYDM_DBG(dm, DBG_COMP_MCC,  ("=========================== [Path-%d] TXBB Offset============================\n", RFPath));
		PHYDM_DBG(dm, DBG_COMP_MCC,  ("absolute_ofdm_swing_idx[RFPath](%d) = absolute_ofdm_swing_idx[RFPath](%d) + kfree_offset[RFPath](%d), RFPath=%d\n", cali_info->absolute_ofdm_swing_idx[RFPath], cali_info->absolute_ofdm_swing_idx[RFPath], cali_info->kfree_offset[RFPath], RFPath));

		if (cali_info->absolute_ofdm_swing_idx[RFPath] >= 0)				/* check if RF_Index is positive or not*/
			bPositive = true;
		else
			bPositive = false;

		odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, bit_mask, bPositive);

		bit_mask = BIT(18)|BIT(17)|BIT(16)|BIT(15);
		Final_RF_Index = cali_info->absolute_ofdm_swing_idx[RFPath]  / 2;	/*TxBB 1 step equal 1dB, BB swing 1step equal 0.5dB*/

	}

	if(Final_RF_Index > UpperBound)		//Upper bound = 10dB, if more htan upper bound, then move to bb swing max = +2dB
	{
		odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, bit_mask, UpperBound);	//set RF Reg0x55 per path

		Final_OFDM_Swing_Index = cali_info->default_ofdm_index + (cali_info->absolute_ofdm_swing_idx[RFPath] - (UpperBound << 1));

		PHYDM_DBG(dm, DBG_COMP_MCC,  ("Final_OFDM_Swing_Index(%d) = default_ofdm_index(%d) + (absolute_ofdm_swing_idx[RFPath](%d) - (UpperBound(%d) << 1)), RFPath=%d\n", Final_OFDM_Swing_Index, cali_info->default_ofdm_index, cali_info->absolute_ofdm_swing_idx[RFPath], UpperBound, RFPath));

		if (Final_OFDM_Swing_Index > TxScalingUpperBound) {	/* bb swing upper bound = +2dB */
			Final_OFDM_Swing_Index = TxScalingUpperBound;
			PHYDM_DBG(dm, DBG_COMP_MCC,  ("Final_OFDM_Swing_Index(%d) > TxScalingUpperBound(%d)   Final_OFDM_Swing_Index = TxScalingUpperBound\n", Final_OFDM_Swing_Index, TxScalingUpperBound));
			PHYDM_DBG(dm, DBG_COMP_MCC,  ("===========================================================================\n"));
		}

		return Final_OFDM_Swing_Index;
	}
	else if(Final_RF_Index < LowerBound)	// lower bound = -5dB
	{
		odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, bit_mask, (-1)*(LowerBound));	//set RF Reg0x55 per path

		Final_OFDM_Swing_Index = cali_info->default_ofdm_index - ((LowerBound<<1) - cali_info->absolute_ofdm_swing_idx[RFPath]);

		PHYDM_DBG(dm, DBG_COMP_MCC,  ("Final_OFDM_Swing_Index(%d) = default_ofdm_index(%d) - ((LowerBound(%d)<<1) - absolute_ofdm_swing_idx[RFPath](%d)), RFPath=%d\n", Final_OFDM_Swing_Index, cali_info->default_ofdm_index, LowerBound, cali_info->absolute_ofdm_swing_idx[RFPath], RFPath));

		if (Final_OFDM_Swing_Index < TxScalingLowerBound) {	/* BB swing lower bound = -10dB */
			Final_OFDM_Swing_Index = TxScalingLowerBound;
			PHYDM_DBG(dm, DBG_COMP_MCC,  ("Final_OFDM_Swing_Index(%d) > TxScalingLowerBound(%d)   Final_OFDM_Swing_Index = TxScalingLowerBound\n", Final_OFDM_Swing_Index, TxScalingLowerBound));
			PHYDM_DBG(dm, DBG_COMP_MCC,  ("===========================================================================\n"));
		}
		return Final_OFDM_Swing_Index;
	}
	else		// normal case
	{

		if(bPositive == true)
			odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, bit_mask, Final_RF_Index);	//set RF Reg0x55 per path
		else
			odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, bit_mask, (-1)*Final_RF_Index);	//set RF Reg0x55 per path

		Final_OFDM_Swing_Index = cali_info->default_ofdm_index + (cali_info->absolute_ofdm_swing_idx[RFPath])%2;

		PHYDM_DBG(dm, DBG_COMP_MCC,  ("Final_OFDM_Swing_Index(%d) = default_ofdm_index(%d) + (absolute_ofdm_swing_idx[RFPath])//2(%d), RFPath=%d\n", Final_OFDM_Swing_Index, cali_info->default_ofdm_index, (cali_info->absolute_ofdm_swing_idx[RFPath])%2, RFPath));
		PHYDM_DBG(dm, DBG_COMP_MCC,  ("===========================================================================\n"));

		return Final_OFDM_Swing_Index;
	}

	return false;
}


VOID
ODM_TxPwrTrackSetPwr8814A(
	IN	PVOID		pDM_VOID,
	enum pwrtrack_method 	Method,
	u8 				RFPath,
	u8 				ChannelMappedIndex
	)
{
	struct dm_struct	*		dm = (struct dm_struct	*)pDM_VOID;
		void *		adapter = dm->adapter;
		PHAL_DATA_TYPE	hal_data = GET_HAL_DATA(adapter);
		struct dm_rf_calibration_struct  *	cali_info = &(dm->rf_calibrate_info);
		u8			Final_OFDM_Swing_Index = 0;

		if (Method == MIX_MODE)
		{
			PHYDM_DBG(dm, DBG_COMP_MCC,  ("cali_info->default_ofdm_index=%d, cali_info->absolute_ofdm_swing_idx[RFPath]=%d, RF_Path = %d\n",
				cali_info->default_ofdm_index, cali_info->absolute_ofdm_swing_idx[RFPath], RFPath));

			Final_OFDM_Swing_Index = CheckRFGainOffset(dm, MIX_MODE, RFPath);
		}
		else if(Method == TSSI_MODE)
		{
			odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, BIT(18)|BIT(17)|BIT(16)|BIT(15), 0);
		}
		else if(Method == BBSWING)		// use for mp driver clean power tracking status
		{
			cali_info->absolute_ofdm_swing_idx[RFPath] = cali_info->absolute_ofdm_swing_idx[RFPath] + cali_info->kfree_offset[RFPath];

			Final_OFDM_Swing_Index = cali_info->default_ofdm_index + (cali_info->absolute_ofdm_swing_idx[RFPath]);

			odm_set_rf_reg(dm, RFPath, rRF_TxGainOffset, BIT(18)|BIT(17)|BIT(16)|BIT(15), 0);
		}

		if((Method == MIX_MODE) || (Method == BBSWING))
		{
			PHYDM_DBG(dm, DBG_COMP_MCC,  ("=========================== [Path-%d] BBSWING Offset============================\n", RFPath));

			switch(RFPath)
			{
				case RF_PATH_A:

					odm_set_bb_reg(dm, rA_TxScale_Jaguar, 0xFFE00000, tx_scaling_table_jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					PHYDM_DBG(dm,DBG_COMP_MCC,
						("******Path_A Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
					break;

				case RF_PATH_B:

					odm_set_bb_reg(dm, rB_TxScale_Jaguar, 0xFFE00000, tx_scaling_table_jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					PHYDM_DBG(dm,DBG_COMP_MCC,
						("******Path_B Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
					break;

				case RF_PATH_C:

					odm_set_bb_reg(dm, rC_TxScale_Jaguar2, 0xFFE00000, tx_scaling_table_jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					PHYDM_DBG(dm,DBG_COMP_MCC,
						("******Path_C Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
			            	break;

				case RF_PATH_D:

					odm_set_bb_reg(dm, rD_TxScale_Jaguar2, 0xFFE00000, tx_scaling_table_jaguar[Final_OFDM_Swing_Index]);	//set BBswing

					PHYDM_DBG(dm,DBG_COMP_MCC,
						("******Path_D Compensate with BBSwing , Final_OFDM_Swing_Index = %d \n", Final_OFDM_Swing_Index));
					break;

				default:
					PHYDM_DBG(dm,DBG_COMP_MCC,
						("Wrong Path name!!!! \n"));

				break;
			}

			PHYDM_DBG(dm, DBG_COMP_MCC,  ("===========================================================================\n"));
		}
		return;
}	// ODM_TxPwrTrackSetPwr8814A


VOID
GetDeltaSwingTable_8814A(
	IN	PVOID		pDM_VOID,
	u8* 			*TemperatureUP_A,
	u8* 			*TemperatureDOWN_A,
	u8* 			*TemperatureUP_B,
	u8* 			*TemperatureDOWN_B
	)
{
	struct dm_struct	*	dm = (struct dm_struct	*)pDM_VOID;
    struct _ADAPTER        *adapter   		 = dm->adapter;
	struct dm_rf_calibration_struct  *  	cali_info = &(dm->rf_calibrate_info);
	HAL_DATA_TYPE  	*hal_data  		 = GET_HAL_DATA(adapter);
	u8		TxRate			= 0xFF;
	u8         	channel   		 = hal_data->current_channel;


	if (*(dm->mp_mode) == true) {
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
		#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			#if (MP_DRIVER == 1)
					PMPT_CONTEXT pMptCtx = &(adapter->mpt_ctx);

					TxRate = mpt_to_mgnt_rate(pMptCtx->mpt_rate_index);
			#endif
		#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				PMPT_CONTEXT pMptCtx = &(adapter->mppriv.mpt_ctx);

				TxRate = mpt_to_mgnt_rate(pMptCtx->mpt_rate_index);
		#endif
	#endif
	} else {
		u2Byte	rate	 = *(dm->forced_data_rate);

		if (!rate) { /*auto rate*/
			if (dm->tx_rate != 0xFF) {
			#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
						TxRate = adapter->HalFunc.GetHwRateFromMRateHandler(dm->tx_rate);
			#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
						TxRate = hw_rate_to_m_rate(dm->tx_rate);
			#endif
			}
		} else { /*force rate*/
			TxRate = (u8)rate;
		}
	}

	PHYDM_DBG(dm, DBG_COMP_MCC,  ("Power Tracking TxRate=0x%X\n", TxRate));

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_A   = cali_info->delta_swing_table_idx_2g_cck_a_p;
			*TemperatureDOWN_A = cali_info->delta_swing_table_idx_2g_cck_a_n;
			*TemperatureUP_B   = cali_info->delta_swing_table_idx_2g_cck_b_p;
			*TemperatureDOWN_B = cali_info->delta_swing_table_idx_2g_cck_b_n;
		} else {
			*TemperatureUP_A   = cali_info->delta_swing_table_idx_2ga_p;
			*TemperatureDOWN_A = cali_info->delta_swing_table_idx_2ga_n;
			*TemperatureUP_B   = cali_info->delta_swing_table_idx_2gb_p;
			*TemperatureDOWN_B = cali_info->delta_swing_table_idx_2gb_n;
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_A   = cali_info->delta_swing_table_idx_5ga_p[0];
		*TemperatureDOWN_A = cali_info->delta_swing_table_idx_5ga_n[0];
		*TemperatureUP_B   = cali_info->delta_swing_table_idx_5gb_p[0];
		*TemperatureDOWN_B = cali_info->delta_swing_table_idx_5gb_n[0];
	} else if (100 <= channel && channel <= 144) {
		*TemperatureUP_A   = cali_info->delta_swing_table_idx_5ga_p[1];
		*TemperatureDOWN_A = cali_info->delta_swing_table_idx_5ga_n[1];
		*TemperatureUP_B   = cali_info->delta_swing_table_idx_5gb_p[1];
		*TemperatureDOWN_B = cali_info->delta_swing_table_idx_5gb_n[1];
	} else if (149 <= channel && channel <= 173) {
		*TemperatureUP_A   = cali_info->delta_swing_table_idx_5ga_p[2];
		*TemperatureDOWN_A = cali_info->delta_swing_table_idx_5ga_n[2];
		*TemperatureUP_B   = cali_info->delta_swing_table_idx_5gb_p[2];
		*TemperatureDOWN_B = cali_info->delta_swing_table_idx_5gb_n[2];
	} else {
		*TemperatureUP_A   = (u8*)delta_swing_table_idx_2ga_p_8188e;
		*TemperatureDOWN_A = (u8*)delta_swing_table_idx_2ga_n_8188e;
		*TemperatureUP_B   = (u8*)delta_swing_table_idx_2ga_p_8188e;
		*TemperatureDOWN_B = (u8*)delta_swing_table_idx_2ga_n_8188e;
	}



	return;
}


VOID
GetDeltaSwingTable_8814A_PathCD(
	IN	PVOID		pDM_VOID,
	u8* 			*TemperatureUP_C,
	u8* 			*TemperatureDOWN_C,
	u8* 			*TemperatureUP_D,
	u8* 			*TemperatureDOWN_D
	)
{
	struct dm_struct	*	dm = (struct dm_struct	*)pDM_VOID;
    struct _ADAPTER			*adapter   		 = dm->adapter;
	struct dm_rf_calibration_struct  *  	cali_info = &(dm->rf_calibrate_info);
	HAL_DATA_TYPE  	*hal_data  		 = GET_HAL_DATA(adapter);
	u8		TxRate			= 0xFF;
	u8         	channel   		 = hal_data->current_channel;


	if (*(dm->mp_mode) == true) {
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
		#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
			#if (MP_DRIVER == 1)
					PMPT_CONTEXT pMptCtx = &(adapter->mpt_ctx);

					TxRate = mpt_to_mgnt_rate(pMptCtx->mpt_rate_index);
			#endif
		#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				PMPT_CONTEXT pMptCtx = &(adapter->mppriv.mpt_ctx);

				TxRate = mpt_to_mgnt_rate(pMptCtx->mpt_rate_index);
		#endif
	#endif
	} else {
		u2Byte	rate	 = *(dm->forced_data_rate);

		if (!rate) { /*auto rate*/
			if (dm->tx_rate != 0xFF) {
			#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
						TxRate = adapter->HalFunc.GetHwRateFromMRateHandler(dm->tx_rate);
			#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
						TxRate = hw_rate_to_m_rate(dm->tx_rate);
			#endif
			}
		} else { /*force rate*/
			TxRate = (u8)rate;
		}
	}

	PHYDM_DBG(dm, DBG_COMP_MCC,  ("Power Tracking TxRate=0x%X\n", TxRate));

	if ( 1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_C  = cali_info->delta_swing_table_idx_2g_cck_c_p;
			*TemperatureDOWN_C = cali_info->delta_swing_table_idx_2g_cck_c_n;
			 *TemperatureUP_D   = cali_info->delta_swing_table_idx_2g_cck_d_p;
			*TemperatureDOWN_D = cali_info->delta_swing_table_idx_2g_cck_d_n;
		} else {
			*TemperatureUP_C   = cali_info->delta_swing_table_idx_2gc_p;
			*TemperatureDOWN_C = cali_info->delta_swing_table_idx_2gc_n;
			*TemperatureUP_D   = cali_info->delta_swing_table_idx_2gd_p;
			*TemperatureDOWN_D = cali_info->delta_swing_table_idx_2gd_n;
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_C   = cali_info->delta_swing_table_idx_5gc_p[0];
		*TemperatureDOWN_C = cali_info->delta_swing_table_idx_5gc_n[0];
		*TemperatureUP_D   = cali_info->delta_swing_table_idx_5gd_p[0];
		*TemperatureDOWN_D = cali_info->delta_swing_table_idx_5gd_n[0];
	} else if (100 <= channel && channel <= 144) {
		*TemperatureUP_C   = cali_info->delta_swing_table_idx_5gc_p[1];
		*TemperatureDOWN_C = cali_info->delta_swing_table_idx_5gc_n[1];
		*TemperatureUP_D   = cali_info->delta_swing_table_idx_5gd_p[1];
		*TemperatureDOWN_D = cali_info->delta_swing_table_idx_5gd_n[1];
	} else if (149 <= channel && channel <= 173) {
		*TemperatureUP_C   = cali_info->delta_swing_table_idx_5gc_p[2];
		*TemperatureDOWN_C = cali_info->delta_swing_table_idx_5gc_n[2];
		*TemperatureUP_D   = cali_info->delta_swing_table_idx_5gd_p[2];
		*TemperatureDOWN_D = cali_info->delta_swing_table_idx_5gd_n[2];
	} else {
		*TemperatureUP_C   = (u8*)delta_swing_table_idx_2ga_p_8188e;
		*TemperatureDOWN_C = (u8*)delta_swing_table_idx_2ga_n_8188e;
		*TemperatureUP_D   = (u8*)delta_swing_table_idx_2ga_p_8188e;
		*TemperatureDOWN_D = (u8*)delta_swing_table_idx_2ga_n_8188e;
	}

	return;
}

void configure_txpower_track_8814a(
	struct txpwrtrack_cfg	*pConfig
	)
{
	pConfig->swing_table_size_cck = CCK_TABLE_SIZE;
	pConfig->swing_table_size_ofdm = OFDM_TABLE_SIZE;
	pConfig->threshold_iqk = 8;
	pConfig->average_thermal_num = AVG_THERMAL_NUM_8814A;
	pConfig->rf_path_count = MAX_PATH_NUM_8814A;
	pConfig->thermal_reg_addr = RF_T_METER_88E;

	pConfig->odm_tx_pwr_track_set_pwr = ODM_TxPwrTrackSetPwr8814A;
	pConfig->do_iqk = DoIQK_8814A;
	pConfig->phy_lc_calibrate = phy_lc_calibrate_8814a;
	pConfig->get_delta_swing_table = GetDeltaSwingTable_8814A;
	pConfig->get_delta_swing_table8814only = GetDeltaSwingTable_8814A_PathCD;
}

VOID
_phy_lc_calibrate_8814a(
	IN struct dm_struct	*		dm,
	IN	boolean		is2T
	)
{
	u32	/*RF_Amode=0, RF_Bmode=0,*/ LC_Cal = 0, tmp = 0, cnt;

	//Check continuous TX and Packet TX
	u32	reg0x914 = odm_read_4byte(dm, rSingleTone_ContTx_Jaguar);;

	// Backup RF reg18.

	if((reg0x914 & 0x70000) == 0)
		odm_write_1byte(dm, REG_TXPAUSE, 0xFF);

	//3 3. Read RF reg18
	LC_Cal = odm_get_rf_reg(dm, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	//3 4. Set LC calibration begin bit15
	odm_set_rf_reg(dm, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x1b126);

	ODM_delay_ms(100);

	for (cnt = 0; cnt < 100; cnt++) {
		if (odm_get_rf_reg(dm, RF_PATH_A, RF_CHNLBW, 0x8000) != 0x1)
			break;
		ODM_delay_ms(10);
	}
	PHYDM_DBG(dm, DBG_CMN,  ("retry cnt = %d\n", cnt));

	odm_set_rf_reg( dm, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x13126);
	odm_set_rf_reg( dm, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x13124);
	//3 Restore original situation
	if((reg0x914 & 70000) == 0)
		odm_write_1byte(dm, REG_TXPAUSE, 0x00);

	// Recover channel number
	odm_set_rf_reg(dm, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);

	//DbgPrint("Call %s\n", __FUNCTION__);
}


VOID
phy_APCalibrate_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN struct dm_struct	*		dm,
#else
	IN	void *	padapter,
#endif
	IN	s1Byte 		delta,
	IN	boolean		is2T
	)
{
}


VOID
phy_lc_calibrate_8814a(
	IN	PVOID		pDM_VOID
	)
{
	boolean 		bStartContTx = false, bSingleTone = false, bCarrierSuppression = false;
	struct dm_struct	*		dm = (struct dm_struct	*)pDM_VOID;
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	struct _ADAPTER		 		*padapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(padapter);

#if (MP_DRIVER == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PMPT_CONTEXT	pMptCtx = &(padapter->mpt_ctx);
	bStartContTx = pMptCtx->bStartContTx;
	bSingleTone = pMptCtx->bSingleTone;
	bCarrierSuppression = pMptCtx->bCarrierSuppression;
#else
	PMPT_CONTEXT	pMptCtx = &(padapter->mppriv.mpt_ctx);
#endif
#endif
#endif

	PHYDM_DBG(dm, DBG_CMN,  ("===> PHY_LCCalibrate_8814A\n"));

//#if (MP_DRIVER == 1)
	_phy_lc_calibrate_8814a(dm, true);
//#endif

	PHYDM_DBG(dm, DBG_CMN,  ("<=== PHY_LCCalibrate_8814A\n"));

}

VOID
PHY_APCalibrate_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN struct dm_struct	*		dm,
#else
	IN	void *	padapter,
#endif
	IN	s1Byte 		delta
	)
{

}


VOID
PHY_DPCalibrate_8814A(
	IN 	struct dm_struct	*	dm
	)
{
}


boolean
phy_QueryRFPathSwitch_8814A(
	IN	void *	padapter
	)
{
	return true;
}


boolean PHY_QueryRFPathSwitch_8814A(
	IN	void *	padapter
	)
{
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(padapter);

#if DISABLE_BB_RF
	return true;
#endif

	return phy_QueryRFPathSwitch_8814A(padapter);
}


VOID _phy_SetRFPathSwitch_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN struct dm_struct	*		dm,
#else
	IN	void *	padapter,
#endif
	IN	boolean		bMain,
	IN	boolean		is2T
	)
{
}
VOID phy_set_rf_path_switch_8814a(
#if ((DM_ODM_SUPPORT_TYPE & ODM_AP) || (DM_ODM_SUPPORT_TYPE == ODM_CE))
	IN struct dm_struct	*		dm,
#else
	IN	void *	padapter,
#endif
	IN	boolean		bMain
	)
{
}
