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


/*---------------------------Define Local Constant---------------------------*/


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
void DoIQK_8814A(
	void*		pDM_VOID,
	u8 		DeltaThermalIndex,
	u8		ThermalValue,	
	u8 		Threshold
	)
{
	struct dm_struct    *	dm = (struct dm_struct    *)pDM_VOID;

	odm_reset_iqk_result(dm);		

	dm->rf_calibrate_info.thermal_value_iqk= ThermalValue;
    
	phy_iq_calibrate_8814a(dm, false);
	
}
#else
/*Originally pConfig->DoIQK is hooked PHY_IQCalibrate_8814A, but DoIQK_8814A and PHY_IQCalibrate_8814A have different arguments*/
void DoIQK_8814A(
	void*		pDM_VOID,
	u8	DeltaThermalIndex,
	u8	ThermalValue,	
	u8	Threshold
	)
{
	struct dm_struct    *	dm = (struct dm_struct    *)pDM_VOID;
	boolean		bReCovery = (boolean) DeltaThermalIndex;

	phy_iq_calibrate_8814a(dm, bReCovery);
}
#endif
//1 7.	IQK

VOID 
_IQK_BackupMacBB_8814A(
	IN struct dm_struct    *	dm,
	u32*		MAC_backup,
	u32*		BB_backup,
	u32*		Backup_MAC_REG,
	u32*		Backup_BB_REG
	)
{
	u32 i;
	 //save MACBB default value
	for (i = 0; i < MAC_REG_NUM_8814; i++){
		MAC_backup[i] = odm_read_4byte(dm, Backup_MAC_REG[i]);
	}
	for (i = 0; i < BB_REG_NUM_8814; i++){
		BB_backup[i] = odm_read_4byte(dm, Backup_BB_REG[i]);
	}
	
	PHYDM_DBG(dm, DBG_CMN,  ("BackupMacBB Success!!!!\n"));
}


VOID
_IQK_BackupRF_8814A(
	IN struct dm_struct    *	dm,
	u32		RF_backup[][4],
	u32*		Backup_RF_REG
	)	
{
	u32 i;
	//Save RF Parameters
    	for (i = 0; i < RF_REG_NUM_8814; i++){
        	RF_backup[i][RF_PATH_A] = odm_get_rf_reg(dm, RF_PATH_A, Backup_RF_REG[i], bRFRegOffsetMask);
		RF_backup[i][RF_PATH_B] = odm_get_rf_reg(dm, RF_PATH_B, Backup_RF_REG[i], bRFRegOffsetMask);
		RF_backup[i][RF_PATH_C] = odm_get_rf_reg(dm, RF_PATH_C, Backup_RF_REG[i], bRFRegOffsetMask);
		RF_backup[i][RF_PATH_D] = odm_get_rf_reg(dm, RF_PATH_D, Backup_RF_REG[i], bRFRegOffsetMask);
    	}

	PHYDM_DBG(dm, DBG_CMN,  ("BackupRF Success!!!!\n"));
}


VOID
_IQK_AFESetting_8814A(
	IN struct dm_struct    *	dm,
	IN boolean		Do_IQK
	)
{
	if(Do_IQK)
	{
		// IQK AFE Setting RX_WAIT_CCA mode 
		odm_write_4byte(dm, 0xc60, 0x0e808003); 
		odm_write_4byte(dm, 0xe60, 0x0e808003);
		odm_write_4byte(dm, 0x1860, 0x0e808003);
		odm_write_4byte(dm, 0x1a60, 0x0e808003);
		odm_set_bb_reg(dm, 0x90c, BIT(13), 0x1);
		 
		odm_set_bb_reg(dm, 0x764, BIT(10)|BIT(9), 0x3);
		odm_set_bb_reg(dm, 0x764, BIT(10)|BIT(9), 0x0);

		odm_set_bb_reg(dm, 0x804, BIT(2), 0x1);
		odm_set_bb_reg(dm, 0x804, BIT(2), 0x0);
		
		PHYDM_DBG(dm, DBG_CMN,  ("AFE IQK mode Success!!!!\n"));
	}
	else
	{
		odm_write_4byte(dm, 0xc60, 0x07808003);
		odm_write_4byte(dm, 0xe60, 0x07808003);
		odm_write_4byte(dm, 0x1860, 0x07808003);
		odm_write_4byte(dm, 0x1a60, 0x07808003);
		odm_set_bb_reg(dm, 0x90c, BIT(13), 0x1);
	
		odm_set_bb_reg(dm, 0x764, BIT(10)|BIT(9), 0x3);
		odm_set_bb_reg(dm, 0x764, BIT(10)|BIT(9), 0x0);

		odm_set_bb_reg(dm, 0x804, BIT(2), 0x1);
		odm_set_bb_reg(dm, 0x804, BIT(2), 0x0);
		
		PHYDM_DBG(dm, DBG_CMN,  ("AFE Normal mode Success!!!!\n"));
	}
	
}


VOID
_IQK_RestoreMacBB_8814A(
	IN struct dm_struct    *		dm,
	u32*		MAC_backup,
	u32*		BB_backup,
	u32*		Backup_MAC_REG, 
	u32*		Backup_BB_REG
	)	
{
	u32 i;
	//Reload MacBB Parameters 
    	for (i = 0; i < MAC_REG_NUM_8814; i++){
        	odm_write_4byte(dm, Backup_MAC_REG[i], MAC_backup[i]);
    	}
	for (i = 0; i < BB_REG_NUM_8814; i++){
        	odm_write_4byte(dm, Backup_BB_REG[i], BB_backup[i]);
    	}
    	PHYDM_DBG(dm, DBG_CMN,  ("RestoreMacBB Success!!!!\n"));
}

VOID
_IQK_RestoreRF_8814A(
	IN struct dm_struct    *			dm,
	u32*			Backup_RF_REG,
	u32 			RF_backup[][4]
	)
{	
	u32 i;

	odm_set_rf_reg(dm, RF_PATH_A, 0xef, bRFRegOffsetMask, 0x0);
	odm_set_rf_reg(dm, RF_PATH_B, 0xef, bRFRegOffsetMask, 0x0);
	odm_set_rf_reg(dm, RF_PATH_C, 0xef, bRFRegOffsetMask, 0x0);
	odm_set_rf_reg(dm, RF_PATH_D, 0xef, bRFRegOffsetMask, 0x0);

    	for (i = 0; i < RF_REG_NUM_8814; i++){
        	odm_set_rf_reg(dm, RF_PATH_A, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][RF_PATH_A]);
		odm_set_rf_reg(dm, RF_PATH_B, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][RF_PATH_B]);
		odm_set_rf_reg(dm, RF_PATH_C, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][RF_PATH_C]);
		odm_set_rf_reg(dm, RF_PATH_D, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][RF_PATH_D]);
    	}

	PHYDM_DBG(dm, DBG_CMN,  ("RestoreRF Success!!!!\n"));
	
}

VOID 
PHY_ResetIQKResult_8814A(
	IN	struct dm_struct    *	dm
)
{
	odm_write_4byte(dm, 0x1b00, 0xf8000000);
	odm_write_4byte(dm, 0x1b38, 0x20000000);
	odm_write_4byte(dm, 0x1b00, 0xf8000002);
	odm_write_4byte(dm, 0x1b38, 0x20000000);
	odm_write_4byte(dm, 0x1b00, 0xf8000004);
	odm_write_4byte(dm, 0x1b38, 0x20000000);
	odm_write_4byte(dm, 0x1b00, 0xf8000006);
	odm_write_4byte(dm, 0x1b38, 0x20000000);
	odm_write_4byte(dm, 0xc10, 0x100);
	odm_write_4byte(dm, 0xe10, 0x100);
	odm_write_4byte(dm, 0x1810, 0x100);
	odm_write_4byte(dm, 0x1a10, 0x100);
}

VOID 
_IQK_ResetNCTL_8814A(
	IN struct dm_struct    *	dm
)
{ 
	odm_write_4byte(dm, 0x1b00, 0xf8000000);
	odm_write_4byte(dm, 0x1b80, 0x00000006);
	odm_write_4byte(dm, 0x1b00, 0xf8000000);
	odm_write_4byte(dm, 0x1b80, 0x00000002);
	PHYDM_DBG(dm, DBG_CMN,  ("ResetNCTL Success!!!!\n"));
}

VOID 
_IQK_ConfigureMAC_8814A(
	IN struct dm_struct    *		dm
	)
{
	// ========MAC register setting========
	odm_write_1byte(dm, 0x522, 0x3f);
	odm_set_bb_reg(dm, 0x550, BIT(11)|BIT(3), 0x0);
	odm_write_1byte(dm, 0x808, 0x00);				//		RX ante off
	odm_set_bb_reg(dm, 0x838, 0xf, 0xe);				//		CCA off
	odm_set_bb_reg(dm, 0xa14, BIT(9)|BIT(8), 0x3);	//  		CCK RX Path off
	odm_write_4byte(dm, 0xcb0, 0x77777777);
	odm_write_4byte(dm, 0xeb0, 0x77777777);
	odm_write_4byte(dm, 0x18b4, 0x77777777);
	odm_write_4byte(dm, 0x1ab4, 0x77777777);
	odm_set_bb_reg(dm, 0x1abc, 0x0ff00000, 0x77);
	/*by YN*/
	odm_set_bb_reg(dm, 0xcbc, 0xf, 0x0);
}

VOID
_LOK_One_Shot(
	IN	void*		pDM_VOID
)
{	
	struct dm_struct    *	dm = (struct dm_struct    *)pDM_VOID;
	struct dm_iqk_info	*	pIQK_info = &dm->IQK_info;
	u8		Path = 0, delay_count = 0, ii;
	boolean		LOK_notready = false;
	u32		LOK_temp1 = 0, LOK_temp2 = 0;

	PHYDM_DBG(dm, DBG_CMN,  ("============ LOK ============\n"));
	for(Path =0; Path <=3; Path++){
		PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
			("==========S%d LOK ==========\n", Path));
		
		odm_set_bb_reg(dm, 0x9a4, BIT(21)|BIT(20), Path);	 // ADC Clock source
		odm_write_4byte(dm, 0x1b00, (0xf8000001|(1<<(4+Path))));	// LOK: CMD ID = 0  {0xf8000011, 0xf8000021, 0xf8000041, 0xf8000081}
		ODM_delay_ms(LOK_delay);
		delay_count = 0;
		LOK_notready = true;
		
		while(LOK_notready){
			LOK_notready = (boolean) odm_get_bb_reg(dm, 0x1b00, BIT(0));
			ODM_delay_ms(1);
			delay_count++;
			if(delay_count >= 10){
				PHYDM_DBG(dm, DBG_CMN,  
					("S%d LOK timeout!!!\n", Path));
				
				_IQK_ResetNCTL_8814A(dm);
				break;
			}
		}
		PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
			("S%d ==> delay_count = 0x%d\n", Path, delay_count));
		
		if(!LOK_notready){
			odm_write_4byte(dm, 0x1b00, 0xf8000000|(Path<<1));
			odm_write_4byte(dm, 0x1bd4, 0x003f0001);
			LOK_temp2 = (odm_get_bb_reg(dm, 0x1bfc, 0x003e0000)+0x10)&0x1f;
			LOK_temp1 = (odm_get_bb_reg(dm, 0x1bfc, 0x0000003e)+0x10)&0x1f;
			
			for(ii = 1; ii<5; ii++){
				LOK_temp1 = LOK_temp1 + ((LOK_temp1 & BIT(4-ii))<<(ii*2));
				LOK_temp2 = LOK_temp2 + ((LOK_temp2 & BIT(4-ii))<<(ii*2));
			}
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("LOK_temp1 = 0x%x, LOK_temp2 = 0x%x\n", LOK_temp1>>4, LOK_temp2>>4));
			
			odm_set_rf_reg(dm, Path, 0x8, 0x07c00, LOK_temp1>>4);
			odm_set_rf_reg(dm, Path, 0x8, 0xf8000, LOK_temp2>>4);
			
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("==>S%d fill LOK\n", Path));

		}
		else{
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("==>S%d LOK Fail!!!\n", Path));
			odm_set_rf_reg(dm, Path, 0x8, bRFRegOffsetMask, 0x08400);
		}
		pIQK_info->lok_fail[Path] = LOK_notready;
		
	}
	PHYDM_DBG(dm, DBG_CMN,  
		("LOK0_notready = %d, LOK1_notready = %d, LOK2_notready = %d, LOK3_notready = %d\n", 
		pIQK_info->lok_fail[0], pIQK_info->lok_fail[1], pIQK_info->lok_fail[2], pIQK_info->lok_fail[3]));
}

VOID
_IQK_One_Shot(
	IN	void*		pDM_VOID
)
{	
	struct dm_struct    *	dm = (struct dm_struct    *)pDM_VOID;
	struct dm_iqk_info	*	pIQK_info = &dm->IQK_info;
	u8		Path = 0, delay_count = 0, cal_retry = 0, idx;
	boolean		notready = true, fail = true;
	u32		IQK_CMD;
	u16		IQK_Apply[4]	= {0xc94, 0xe94, 0x1894, 0x1a94};
	
	for(idx = 0; idx <= 1; idx++){						// ii = 0:TXK , 1: RXK
	
		if(idx == TX_IQK){
			PHYDM_DBG(dm, DBG_CMN,  
				("============ WBTXIQK ============\n"));
		}
		else if(idx == RX_IQK){
			PHYDM_DBG(dm, DBG_CMN,  
				("============ WBRXIQK ============\n"));
		}	
		
		for(Path =0; Path <=3; Path++){
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("==========S%d IQK ==========\n", Path));
			cal_retry = 0;
			fail = true;
			while(fail){
				odm_set_bb_reg(dm, 0x9a4, BIT(21)|BIT(20), Path);	
				if(idx == TX_IQK){
					IQK_CMD = (0xf8000001|(*dm->band_width+3)<<8|(1<<(4+Path)));
					
					PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
						("TXK_Trigger = 0x%x\n", IQK_CMD));
						/*  
						{0xf8000311, 0xf8000321, 0xf8000341, 0xf8000381} ==> 20 WBTXK (CMD = 3)
						{0xf8000411, 0xf8000421, 0xf8000441, 0xf8000481} ==> 40 WBTXK (CMD = 4)
						{0xf8000511, 0xf8000521, 0xf8000541, 0xf8000581} ==> 80 WBTXK (CMD = 5)
						*/
					odm_write_4byte(dm, 0x1b00, IQK_CMD);
				}
				else if(idx == RX_IQK){
					IQK_CMD = (0xf8000001|(9-*dm->band_width)<<8|(1<<(4+Path)));

					PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
						("TXK_Trigger = 0x%x\n", IQK_CMD));
						/*
						{0xf8000911, 0xf8000921, 0xf8000941, 0xf8000981} ==> 20 WBRXK (CMD = 9)
						{0xf8000811, 0xf8000821, 0xf8000841, 0xf8000881} ==> 40 WBRXK (CMD = 8)
						{0xf8000711, 0xf8000721, 0xf8000741, 0xf8000781} ==> 80 WBRXK (CMD = 7)
						*/
					odm_write_4byte(dm, 0x1b00, IQK_CMD);
				}
				
				ODM_delay_ms(WBIQK_delay);
				
				delay_count = 0;
				notready = true;
				while(notready){
					notready = (boolean) odm_get_bb_reg(dm, 0x1b00, BIT(0));
					if(!notready){
							fail = (boolean) odm_get_bb_reg(dm, 0x1b08, BIT(26));
							break;
					}
					ODM_delay_ms(1);
					delay_count++;
					if(delay_count >= 20){
						PHYDM_DBG(dm, DBG_CMN,  
							("S%d IQK timeout!!!\n", Path));
						_IQK_ResetNCTL_8814A(dm);
						break;
					}
				}
				if(fail)
					cal_retry++;
				if(cal_retry >3 )
					break;
				
			}
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("S%d ==> 0x1b00 = 0x%x\n", Path, odm_read_4byte(dm, 0x1b00)));
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("S%d ==> 0x1b08 = 0x%x\n", Path, odm_read_4byte(dm, 0x1b08)));
			PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
				("S%d ==> delay_count = 0x%d, cal_retry = %x\n", Path, delay_count, cal_retry));
			
			odm_write_4byte(dm, 0x1b00, 0xf8000000|(Path<<1));
			if(!fail){
				if(idx == TX_IQK){
					pIQK_info->iqc_matrix[idx][Path] = odm_read_4byte(dm, 0x1b38);	
				}
				else if(idx == RX_IQK){
					odm_write_4byte(dm, 0x1b3c, 0x20000000);
					pIQK_info->iqc_matrix[idx][Path] = odm_read_4byte(dm, 0x1b3c);
				}
				PHYDM_DBG(dm, DBG_CMN, ODM_DBG_TRACE, 
					("S%d_IQC = 0x%x\n", Path, pIQK_info->iqc_matrix[idx][Path]));

			}
			
			if(idx == RX_IQK){
				if(pIQK_info->iqk_fail[TX_IQK][Path] == false)			// TXIQK success in RXIQK
					odm_write_4byte( dm, 0x1b38, pIQK_info->iqc_matrix[TX_IQK][Path]);
				else
					odm_set_bb_reg(dm, IQK_Apply[Path], BIT(0), 0x0);

				if(fail)												// RXIQK Fail
					odm_set_bb_reg(dm, IQK_Apply[Path], (BIT(11)|BIT(10)), 0x0);
			}		
			
			pIQK_info->iqk_fail[idx][Path] = fail;
			
		}
		PHYDM_DBG(dm, DBG_CMN,  
			("IQK0_fail = %d, IQK1_fail = %d, IQK2_fail = %d, IQK3_fail = %d\n", 
			pIQK_info->iqk_fail[idx][0], pIQK_info->iqk_fail[idx][1], pIQK_info->iqk_fail[idx][2], pIQK_info->iqk_fail[idx][3]));
	}
}

VOID
_IQK_Tx_8814A(
	IN struct dm_struct    *		dm,
	IN u8 chnlIdx
	)
{	
	PHYDM_DBG(dm, DBG_CMN,  ("BandWidth = %d, ExtPA2G = %d\n", *dm->p_band_width, dm->ext_pa));
	PHYDM_DBG(dm, DBG_CMN,  ("Interface = %d, pBandType = %d\n", dm->support_interface, *dm->p_band_type));
	PHYDM_DBG(dm, DBG_CMN,  ("CutVersion = %x\n", dm->cut_version));

	odm_set_rf_reg(dm, RF_PATH_A, 0x58, BIT(19), 0x1);
	odm_set_rf_reg(dm, RF_PATH_B, 0x58, BIT(19), 0x1);
	odm_set_rf_reg(dm, RF_PATH_C, 0x58, BIT(19), 0x1);
	odm_set_rf_reg(dm, RF_PATH_D, 0x58, BIT(19), 0x1);

	odm_set_bb_reg(dm, 0xc94, (BIT(11)|BIT(10)|BIT(0)), 0x401);
	odm_set_bb_reg(dm, 0xe94, (BIT(11)|BIT(10)|BIT(0)), 0x401);
	odm_set_bb_reg(dm, 0x1894, (BIT(11)|BIT(10)|BIT(0)), 0x401);
	odm_set_bb_reg(dm, 0x1a94, (BIT(11)|BIT(10)|BIT(0)), 0x401);
	
	if(*dm->band_type == ODM_BAND_5G)
		odm_write_4byte(dm, 0x1b00, 0xf8000ff1);
	else
		odm_write_4byte(dm, 0x1b00, 0xf8000ef1);

	ODM_delay_ms(1);

	odm_write_4byte(dm, 0x810, 0x20101063);
	odm_write_4byte(dm, 0x90c, 0x0B00C000);

	_LOK_One_Shot(dm);
	_IQK_One_Shot(dm);

}

VOID	
_phy_iq_calibrate_8814a(
	IN struct dm_struct    *		dm,
	IN u8		Channel
	)
{
	
	u32	MAC_backup[MAC_REG_NUM_8814], BB_backup[BB_REG_NUM_8814], RF_backup[RF_REG_NUM_8814][4];
	u32 	Backup_MAC_REG[MAC_REG_NUM_8814] = {0x520, 0x550}; 
	u32 	Backup_BB_REG[BB_REG_NUM_8814] = {0xa14, 0x808, 0x838, 0x90c, 0x810, 0xcb0, 0xeb0,
		0x18b4, 0x1ab4, 0x1abc, 0x9a4, 0x764, 0xcbc}; 
	u32 	Backup_RF_REG[RF_REG_NUM_8814] = {0x0, 0x8f}; 
	u8 	chnlIdx = odm_get_right_chnl_place_for_iqk(Channel);
	
	_IQK_BackupMacBB_8814A(dm, MAC_backup, BB_backup, Backup_MAC_REG, Backup_BB_REG);
	_IQK_AFESetting_8814A(dm,true);
	_IQK_BackupRF_8814A(dm, RF_backup, Backup_RF_REG);
	_IQK_ConfigureMAC_8814A(dm);
	_IQK_Tx_8814A(dm, chnlIdx);
	_IQK_ResetNCTL_8814A(dm);  //for 3-wire to  BB use
	_IQK_AFESetting_8814A(dm,false);
	_IQK_RestoreMacBB_8814A(dm, MAC_backup, BB_backup, Backup_MAC_REG, Backup_BB_REG);
  	_IQK_RestoreRF_8814A(dm, Backup_RF_REG, RF_backup);
}

/*IQK version:v1.1*/
/*update 0xcbc setting*/


VOID
phy_iq_calibrate_8814a(
	IN	void*		pDM_VOID,
	IN	boolean 	bReCovery
	)
{
	struct dm_struct    *	dm = (struct dm_struct    *)pDM_VOID;

#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	void * 		padapter = dm->adapter;
	HAL_DATA_TYPE	*hal_data = GET_HAL_DATA(padapter);	
	
	#if (MP_DRIVER == 1)
		#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)	
			PMPT_CONTEXT	pMptCtx = &(padapter->MptCtx);	
		#else// (DM_ODM_SUPPORT_TYPE == ODM_CE)
			PMPT_CONTEXT	pMptCtx = &(padapter->mppriv.mpt_ctx);		
		#endif	
	#endif//(MP_DRIVER == 1)
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
		if (odm_check_power_status(padapter) == false)
			return;
	#endif

	#if MP_DRIVER == 1	
		if( pMptCtx->is_single_tone || pMptCtx->is_carrier_suppression )
			return;
	#endif
	
#endif	
	#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
		_phy_iq_calibrate_8814a(dm, hal_data->current_channel);
		/*DBG_871X("%s,%d, do IQK %u ms\n", __func__, __LINE__, rtw_get_passing_time_ms(time_iqk));*/
	#else
		_phy_iq_calibrate_8814a(dm, *dm->pChannel);
	#endif
}

VOID
PHY_IQCalibrate_8814A_Init(
	IN	void*		pDM_VOID
	)
{
	struct dm_struct    *	dm = (struct dm_struct    *)pDM_VOID;
	struct dm_iqk_info	*pIQK_info = &dm->IQK_info;
	u8	ii, jj;
	
	PHYDM_DBG(dm, DBG_CMN,  ("=====>PHY_IQCalibrate_8814A_Init\n"));
	for(jj = 0; jj < 2; jj++){
		for(ii = 0; ii < NUM; ii++){
			pIQK_info->lok_fail[ii] = true;
			pIQK_info->iqk_fail[jj][ii] = true;
			pIQK_info->iqc_matrix[jj][ii] = 0x20000000;
		}
	}
}

