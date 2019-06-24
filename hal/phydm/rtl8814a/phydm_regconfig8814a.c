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
#include "../phydm_precomp.h"

#if (RTL8814A_SUPPORT == 1)  

void
odm_config_rf_reg_8814a(
	struct dm_struct    *dm,
	u32 					Addr,
	u32 					Data,
	enum rf_path     RF_PATH,
	u32				    RegAddr
	)
{
    if(Addr == 0xfe || Addr == 0xffe)
	{ 					  
		#ifdef CONFIG_LONG_DELAY_ISSUE
		ODM_sleep_ms(50);
		#else		
		ODM_delay_ms(50);
		#endif
	}
	else
	{
		odm_set_rf_reg(dm, RF_PATH, RegAddr, bRFRegOffsetMask, Data);
		// Add 1us delay between BB/RF register setting.
		ODM_delay_us(1);
	}	
}


void 
odm_config_rf_radio_a_8814a(
	struct dm_struct    *dm,
	u32 					Addr,
	u32 					Data
	)
{
	u4Byte  content = 0x1000; // RF_Content: radioa_txt
	u4Byte	maskforPhySet= (u4Byte)(content&0xE000);

    odm_config_rf_reg_8814a(dm, Addr, Data, RF_PATH_A, Addr|maskforPhySet);

    PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigRFWithHeaderFile: [RadioA] %08X %08X\n", Addr, Data);
}

void 
odm_config_rf_radio_b_8814a(
	struct dm_struct    *dm,
	u32 					Addr,
	u32 					Data
	)
{
	u4Byte  content = 0x1001; // RF_Content: radiob_txt
	u4Byte	maskforPhySet= (u4Byte)(content&0xE000);

    odm_config_rf_reg_8814a(dm, Addr, Data, RF_PATH_B, Addr|maskforPhySet);
	
	PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigRFWithHeaderFile: [RadioB] %08X %08X\n", Addr, Data);
    
}

void 
odm_config_rf_radio_c_8814a(
	struct dm_struct    *dm,
	u32 					Addr,
	u32 					Data
	)
{
	u4Byte  content = 0x1001; // RF_Content: radiob_txt
	u4Byte	maskforPhySet= (u4Byte)(content&0xE000);

    odm_config_rf_reg_8814a(dm, Addr, Data, RF_PATH_C, Addr|maskforPhySet);
	
	PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigRFWithHeaderFile: [RadioC] %08X %08X\n", Addr, Data);
    
}

void 
odm_config_rf_radio_d_8814a(
	struct dm_struct    *dm,
	u32 					Addr,
	u32 					Data
	)
{
	u4Byte  content = 0x1001; // RF_Content: radiob_txt
	u4Byte	maskforPhySet= (u4Byte)(content&0xE000);

    odm_config_rf_reg_8814a(dm, Addr, Data, RF_PATH_D, Addr|maskforPhySet);
	
	PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigRFWithHeaderFile: [RadioD] %08X %08X\n", Addr, Data);
    
}

void 
odm_config_mac_8814a(
 	struct dm_struct    *dm,
 	u32 		Addr,
 	IN 	u1Byte 		Data
 	)
{
	odm_write_1byte(dm, Addr, Data);
    PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigMACWithHeaderFile: [MAC_REG] %08X %08X\n", Addr, Data);
}

void 
odm_config_bb_agc_8814a(
    struct dm_struct    *dm,
    u32 		Addr,
    u32 		Bitmask,
    u32 		Data
    )
{
	odm_set_bb_reg(dm, Addr, Bitmask, Data);		
	// Add 1us delay between BB/RF register setting.
	ODM_delay_us(1);

    PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigBBWithHeaderFile: [AGC_TAB] %08X %08X\n", Addr, Data);
}

void
odm_config_bb_phy_reg_pg_8814a(
	struct dm_struct    *dm,
	u32		Band,
	u32		RfPath,
	u32		TxNum,
    u32 		Addr,
    u32 		Bitmask,
    u32 		Data
    )
{    
	if (Addr == 0xfe || Addr == 0xffe)
		#ifdef CONFIG_LONG_DELAY_ISSUE
		ODM_sleep_ms(50);
		#else		
		ODM_delay_ms(50);
		#endif
    else 
    {
#if	!(DM_ODM_SUPPORT_TYPE&ODM_AP)
	    phy_store_tx_power_by_rate(dm->adapter, Band, RfPath, TxNum, Addr, Bitmask, Data);
#endif
    }
	PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigBBWithHeaderFile: [PHY_REG] %08X %08X %08X\n", Addr, Bitmask, Data);
}

void 
odm_config_bb_phy_8814a(
	struct dm_struct    *dm,
    u32 		Addr,
    u32 		Bitmask,
    u32 		Data
    )
{    
	if (Addr == 0xfe)
		#ifdef CONFIG_LONG_DELAY_ISSUE
		ODM_sleep_ms(50);
		#else		
		ODM_delay_ms(50);
		#endif
	else if (Addr == 0xfd)
		ODM_delay_ms(5);
	else if (Addr == 0xfc)
		ODM_delay_ms(1);
	else if (Addr == 0xfb)
		ODM_delay_us(50);
	else if (Addr == 0xfa)
		ODM_delay_us(5);
	else if (Addr == 0xf9)
		ODM_delay_us(1);
	else 
	{
		odm_set_bb_reg(dm, Addr, Bitmask, Data);		
	}
	
	// Add 1us delay between BB/RF register setting.
	ODM_delay_us(1);
    PHYDM_DBG(dm,ODM_COMP_INIT,"===> ODM_ConfigBBWithHeaderFile: [PHY_REG] %08X %08X\n", Addr, Data);
}

void
odm_config_bb_txpwr_lmt_8814a(
	struct dm_struct    *dm,
	u8*		Regulation,
	u8*		Band,
	u8*		Bandwidth,
	u8*		RateSection,
	u8*		RfPath,
	u8* 	Channel,
	u8*		PowerLimit
    )
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	phy_set_tx_power_limit(dm, Regulation, Band,
		Bandwidth, RateSection, RfPath, Channel, PowerLimit);
#endif
}
#endif

