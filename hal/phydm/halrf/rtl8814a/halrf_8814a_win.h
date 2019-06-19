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

#ifndef __HALRF_8814A_H__
#define __HALRF_8814A_H__

/*--------------------------Define Parameters-------------------------------*/
#define AVG_THERMAL_NUM_8814A	4

#include "halphyrf_win.h"

void configure_txpower_track_8814a(
	struct txpwrtrack_cfg	*config
);

void
get_delta_swing_table_8814a(
	void		*dm_void,
	u8 **temperature_up_a,
	u8 **temperature_down_a,
	u8 **temperature_up_b,
	u8 **temperature_down_b
);

void
get_delta_swing_table_8814a_path_cd(
	void		*dm_void,
	u8 **temperature_up_c,
	u8 **temperature_down_c,
	u8 **temperature_up_d,
	u8 **temperature_down_d
);

void
odm_tx_pwr_track_set_pwr8814a(
	void		*dm_void,
	enum pwrtrack_method	method,
	u8				rf_path,
	u8				channel_mapped_index
);

u1Byte
check_rf_gain_offset(
	void		*dm_void,
	u8				rf_path,
	);


//
// LC calibrate
//
void
phy_lc_calibrate_8814a(
	void		*dm_void
);

//
// AP calibrate
//
void
phy_iq_calibrate_8814a(
	void		*dm_void,
	boolean	is_recovery
);

void
phy_dp_calibrate_8814a(
	void		*dm_void,
);

void phy_set_rf_path_switch_8814a(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	struct dm_struct		*dm,
#else
	void	*adapter,
#endif
	boolean		is_main
);


#endif	// #ifndef __HAL_PHY_RF_8188E_H__
