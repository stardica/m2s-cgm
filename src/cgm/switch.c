
#include <cgm/switch.h>



struct switch_t *switches;
eventcount volatile *switches_ec;
task *switches_tasks;
int switch_pid = 0;

int switch_io_delay_factor = 4;

int switch_north_io_pid = 0;
int switch_east_io_pid = 0;
int switch_south_io_pid = 0;
int switch_west_io_pid = 0;

//supports quad core with ring bus
struct str_map_t l1_strn_map =
{ 	l1_number, {
		{ "l1_i_caches[0]", l1_i_cache_0},
		{ "l1_d_caches[0]", l1_d_cache_0},

		{ "l1_i_caches[1]", l1_i_cache_1},
		{ "l1_d_caches[1]", l1_d_cache_1},

		{ "l1_i_caches[2]", l1_i_cache_2},
		{ "l1_d_caches[2]", l1_d_cache_2},

		{ "l1_i_caches[3]", l1_i_cache_3},
		{ "l1_d_caches[3]", l1_d_cache_3},

		{ "l1_i_caches[4]", l1_i_cache_4},
		{ "l1_d_caches[4]", l1_d_cache_4},

		{ "l1_i_caches[5]", l1_i_cache_5},
		{ "l1_d_caches[5]", l1_d_cache_5},

		{ "l1_i_caches[6]", l1_i_cache_6},
		{ "l1_d_caches[6]", l1_d_cache_6},

		{ "l1_i_caches[7]", l1_i_cache_7},
		{ "l1_d_caches[7]", l1_d_cache_7},

		/*-----------------*/

		{ "l1_i_caches[8]", l1_i_cache_8},
		{ "l1_d_caches[8]", l1_d_cache_8},

		{ "l1_i_caches[9]", l1_i_cache_9},
		{ "l1_d_caches[9]", l1_d_cache_9},

		{ "l1_i_caches[10]", l1_i_cache_10},
		{ "l1_d_caches[10]", l1_d_cache_10},

		{ "l1_i_caches[11]", l1_i_cache_11},
		{ "l1_d_caches[11]", l1_d_cache_11},

		{ "l1_i_caches[12]", l1_i_cache_12},
		{ "l1_d_caches[12]", l1_d_cache_12},

		{ "l1_i_caches[13]", l1_i_cache_13},
		{ "l1_d_caches[13]", l1_d_cache_13},

		{ "l1_i_caches[14]", l1_i_cache_14},
		{ "l1_d_caches[14]", l1_d_cache_14},

		{ "l1_i_caches[15]", l1_i_cache_15},
		{ "l1_d_caches[15]", l1_d_cache_15},

		/*-----------------*/

		{ "l1_i_caches[16]", l1_i_cache_16},
		{ "l1_d_caches[16]", l1_d_cache_16},

		{ "l1_i_caches[17]", l1_i_cache_17},
		{ "l1_d_caches[17]", l1_d_cache_17},

		{ "l1_i_caches[18]", l1_i_cache_18},
		{ "l1_d_caches[18]", l1_d_cache_18},

		{ "l1_i_caches[19]", l1_i_cache_19},
		{ "l1_d_caches[19]", l1_d_cache_19},

		{ "l1_i_caches[20]", l1_i_cache_20},
		{ "l1_d_caches[20]", l1_d_cache_20},

		{ "l1_i_caches[21]", l1_i_cache_21},
		{ "l1_d_caches[21]", l1_d_cache_21},

		{ "l1_i_caches[22]", l1_i_cache_22},
		{ "l1_d_caches[22]", l1_d_cache_22},

		{ "l1_i_caches[23]", l1_i_cache_23},
		{ "l1_d_caches[23]", l1_d_cache_23},

		/*-----------------*/

		{ "l1_i_caches[24]", l1_i_cache_24},
		{ "l1_d_caches[24]", l1_d_cache_24},

		{ "l1_i_caches[25]", l1_i_cache_25},
		{ "l1_d_caches[25]", l1_d_cache_25},

		{ "l1_i_caches[26]", l1_i_cache_26},
		{ "l1_d_caches[26]", l1_d_cache_26},

		{ "l1_i_caches[27]", l1_i_cache_27},
		{ "l1_d_caches[27]", l1_d_cache_27},

		{ "l1_i_caches[28]", l1_i_cache_28},
		{ "l1_d_caches[28]", l1_d_cache_28},

		{ "l1_i_caches[29]", l1_i_cache_29},
		{ "l1_d_caches[29]", l1_d_cache_29},

		{ "l1_i_caches[30]", l1_i_cache_30},
		{ "l1_d_caches[30]", l1_d_cache_30},

		{ "l1_i_caches[31]", l1_i_cache_31},
		{ "l1_d_caches[31]", l1_d_cache_31},

		{ "l1_i_caches[32]", l1_i_cache_32},
		{ "l1_d_caches[32]", l1_d_cache_32},

		{ "l1_i_caches[33]", l1_i_cache_33},
		{ "l1_d_caches[33]", l1_d_cache_33},
		}
};

//**changes here Feb 13 2019

struct str_map_t l2_strn_map =
{ l2_number, {
		{ "l2_caches[0]", l2_caches_0},
		{ "l2_caches[1]", l2_caches_1},
		{ "l2_caches[2]", l2_caches_2},
		{ "l2_caches[3]", l2_caches_3},

		{ "l2_caches[4]", l2_caches_4},
		{ "l2_caches[5]", l2_caches_5},
		{ "l2_caches[6]", l2_caches_6},
		{ "l2_caches[7]", l2_caches_7},

		{ "l2_caches[8]", l2_caches_8},
		{ "l2_caches[9]", l2_caches_9},
		{ "l2_caches[10]", l2_caches_10},
		{ "l2_caches[11]", l2_caches_11},

		{ "l2_caches[12]", l2_caches_12},
		{ "l2_caches[13]", l2_caches_13},
		{ "l2_caches[14]", l2_caches_14},
		{ "l2_caches[15]", l2_caches_15},

		{ "l2_caches[16]", l2_caches_16},
		{ "l2_caches[17]", l2_caches_17},
		{ "l2_caches[18]", l2_caches_18},
		{ "l2_caches[19]", l2_caches_19},

		{ "l2_caches[20]", l2_caches_20},
		{ "l2_caches[21]", l2_caches_21},
		{ "l2_caches[22]", l2_caches_22},
		{ "l2_caches[23]", l2_caches_23},

		{ "l2_caches[24]", l2_caches_24},
		{ "l2_caches[25]", l2_caches_25},
		{ "l2_caches[26]", l2_caches_26},
		{ "l2_caches[27]", l2_caches_27},

		{ "l2_caches[28]", l2_caches_28},
		{ "l2_caches[29]", l2_caches_29},
		{ "l2_caches[30]", l2_caches_30},
		{ "l2_caches[31]", l2_caches_31},

		{ "l2_caches[32]", l2_caches_32},
		{ "l2_caches[33]", l2_caches_33},

		{ "gpu_l2_caches[0]", gpu_l2_caches_0_c},
		{ "gpu_l2_caches[1]", gpu_l2_caches_1_c},
		{ "gpu_l2_caches[2]", gpu_l2_caches_2_c},
		{ "gpu_l2_caches[3]", gpu_l2_caches_3_c},
		{ "gpu_l2_caches[4]", gpu_l2_caches_4_c},
		{ "gpu_l2_caches[5]", gpu_l2_caches_5_c},
		{ "gpu_l2_caches[6]", gpu_l2_caches_6_c},
		{ "gpu_l2_caches[7]", gpu_l2_caches_7_c},
		}
};

struct str_map_t gpu_l1_strn_map =
{ gpu_l1_number, {
		{ "gpu_s_caches[0]", gpu_s_cache_0},
		{ "gpu_v_caches[0]", gpu_v_cache_0},
		{ "gpu_s_caches[1]", gpu_s_cache_1},
		{ "gpu_v_caches[1]", gpu_v_cache_1},
		{ "gpu_s_caches[2]", gpu_s_cache_2},
		{ "gpu_v_caches[2]", gpu_v_cache_2},
		{ "gpu_s_caches[3]", gpu_s_cache_3},
		{ "gpu_v_caches[3]", gpu_v_cache_3},
		{ "gpu_s_caches[4]", gpu_s_cache_4},
		{ "gpu_v_caches[4]", gpu_v_cache_4},
		{ "gpu_s_caches[5]", gpu_s_cache_5},
		{ "gpu_v_caches[5]", gpu_v_cache_5},
		{ "gpu_s_caches[6]", gpu_s_cache_6},
		{ "gpu_v_caches[6]", gpu_v_cache_6},
		{ "gpu_s_caches[7]", gpu_s_cache_7},
		{ "gpu_v_caches[7]", gpu_v_cache_7},
		{ "gpu_s_caches[8]", gpu_s_cache_8},
		{ "gpu_v_caches[8]", gpu_v_cache_8},
		{ "gpu_s_caches[9]", gpu_s_cache_9},
		{ "gpu_v_caches[9]", gpu_v_cache_9},
		{ "gpu_s_caches[10]", gpu_s_cache_10},
		{ "gpu_v_caches[10]", gpu_v_cache_10},
		{ "gpu_s_caches[11]", gpu_s_cache_11},
		{ "gpu_v_caches[11]", gpu_v_cache_11},
		{ "gpu_s_caches[12]", gpu_s_cache_12},
		{ "gpu_v_caches[12]", gpu_v_cache_12},
		{ "gpu_s_caches[13]", gpu_s_cache_13},
		{ "gpu_v_caches[13]", gpu_v_cache_13},
		{ "gpu_s_caches[14]", gpu_s_cache_14},
		{ "gpu_v_caches[14]", gpu_v_cache_14},
		{ "gpu_s_caches[15]", gpu_s_cache_15},
		{ "gpu_v_caches[15]", gpu_v_cache_15},
		{ "gpu_s_caches[16]", gpu_s_cache_16},
		{ "gpu_v_caches[16]", gpu_v_cache_16},
		{ "gpu_s_caches[17]", gpu_s_cache_17},
		{ "gpu_v_caches[17]", gpu_v_cache_17},
		{ "gpu_s_caches[18]", gpu_s_cache_18},
		{ "gpu_v_caches[18]", gpu_v_cache_18},
		{ "gpu_s_caches[19]", gpu_s_cache_19},
		{ "gpu_v_caches[19]", gpu_v_cache_19},
		{ "gpu_s_caches[20]", gpu_s_cache_20},
		{ "gpu_v_caches[20]", gpu_v_cache_20},
		{ "gpu_s_caches[21]", gpu_s_cache_21},
		{ "gpu_v_caches[21]", gpu_v_cache_21},
		{ "gpu_s_caches[22]", gpu_s_cache_22},
		{ "gpu_v_caches[22]", gpu_v_cache_22},
		{ "gpu_s_caches[23]", gpu_s_cache_23},
		{ "gpu_v_caches[23]", gpu_v_cache_23},
		{ "gpu_s_caches[24]", gpu_s_cache_24},
		{ "gpu_v_caches[24]", gpu_v_cache_24},
		{ "gpu_s_caches[25]", gpu_s_cache_25},
		{ "gpu_v_caches[25]", gpu_v_cache_25},
		{ "gpu_s_caches[26]", gpu_s_cache_26},
		{ "gpu_v_caches[26]", gpu_v_cache_26},
		{ "gpu_s_caches[27]", gpu_s_cache_27},
		{ "gpu_v_caches[27]", gpu_v_cache_27},
		{ "gpu_s_caches[28]", gpu_s_cache_28},
		{ "gpu_v_caches[28]", gpu_v_cache_28},
		{ "gpu_s_caches[29]", gpu_s_cache_29},
		{ "gpu_v_caches[29]", gpu_v_cache_29},
		{ "gpu_s_caches[30]", gpu_s_cache_30},
		{ "gpu_v_caches[30]", gpu_v_cache_30},
		{ "gpu_s_caches[31]", gpu_s_cache_31},
		{ "gpu_v_caches[31]", gpu_v_cache_31},
		}
};


struct str_map_t gpu_l2_strn_map =
{ gpu_l2_number, {
		{ "gpu_l2_caches[0]", gpu_l2_caches_0},
		{ "gpu_l2_caches[1]", gpu_l2_caches_1},
		{ "gpu_l2_caches[2]", gpu_l2_caches_2},
		{ "gpu_l2_caches[3]", gpu_l2_caches_3},
		{ "gpu_l2_caches[4]", gpu_l2_caches_4},
		{ "gpu_l2_caches[5]", gpu_l2_caches_5},
		{ "gpu_l2_caches[6]", gpu_l2_caches_6},
		{ "gpu_l2_caches[7]", gpu_l2_caches_7},
		}
};

struct str_map_t node_strn_map_p34 =
{ node_number_p34, {
		{ "l2_caches[0]", l2_cache_0_p34},
		{ "switch[0]", switch_0_p34},
		{ "l3_caches[0]", l3_cache_0_p34},

		{ "l2_caches[1]", l2_cache_1_p34},
		{ "switch[1]", switch_1_p34},
		{ "l3_caches[1]", l3_cache_1_p34},

		{ "l2_caches[2]", l2_cache_2_p34},
		{ "switch[2]", switch_2_p34},
		{ "l3_caches[2]", l3_cache_2_p34},

		{ "l2_caches[3]", l2_cache_3_p34},
		{ "switch[3]", switch_3_p34},
		{ "l3_caches[3]", l3_cache_3_p34},

		/*-----------------*/

		{ "l2_caches[4]", l2_cache_4_p34},
		{ "switch[4]", switch_4_p34},
		{ "l3_caches[4]", l3_cache_4_p34},

		{ "l2_caches[5]", l2_cache_5_p34},
		{ "switch[5]", switch_5_p34},
		{ "l3_caches[5]", l3_cache_5_p34},

		{ "l2_caches[6]", l2_cache_6_p34},
		{ "switch[6]", switch_6_p34},
		{ "l3_caches[6]", l3_cache_6_p34},

		{ "l2_caches[7]", l2_cache_7_p34},
		{ "switch[7]", switch_7_p34},
		{ "l3_caches[7]", l3_cache_7_p34},

		/*-----------------*/

		{ "l2_caches[8]", l2_cache_8_p34},
		{ "switch[8]", switch_8_p34},
		{ "l3_caches[8]", l3_cache_8_p34},

		{ "l2_caches[9]", l2_cache_9_p34},
		{ "switch[9]", switch_9_p34},
		{ "l3_caches[9]", l3_cache_9_p34},

		{ "l2_caches[10]", l2_cache_10_p34},
		{ "switch[10]", switch_10_p34},
		{ "l3_caches[10]", l3_cache_10_p34},

		{ "l2_caches[11]", l2_cache_11_p34},
		{ "switch[11]", switch_11_p34},
		{ "l3_caches[11]", l3_cache_11_p34},

		/*-----------------*/

		{ "l2_caches[12]", l2_cache_12_p34},
		{ "switch[12]", switch_12_p34},
		{ "l3_caches[12]", l3_cache_12_p34},

		{ "l2_caches[13]", l2_cache_13_p34},
		{ "switch[13]", switch_13_p34},
		{ "l3_caches[13]", l3_cache_13_p34},

		{ "l2_caches[14]", l2_cache_14_p34},
		{ "switch[14]", switch_14_p34},
		{ "l3_caches[14]", l3_cache_14_p34},

		{ "l2_caches[15]", l2_cache_15_p34},
		{ "switch[15]", switch_15_p34},
		{ "l3_caches[15]", l3_cache_15_p34},

		/*-----------------*/

		{ "l2_caches[16]", l2_cache_16_p34},
		{ "switch[16]", switch_16_p34},
		{ "l3_caches[16]", l3_cache_16_p34},

		{ "l2_caches[17]", l2_cache_17_p34},
		{ "switch[17]", switch_17_p34},
		{ "l3_caches[17]", l3_cache_17_p34},

		{ "l2_caches[18]", l2_cache_18_p34},
		{ "switch[18]", switch_18_p34},
		{ "l3_caches[18]", l3_cache_18_p34},

		{ "l2_caches[19]", l2_cache_19_p34},
		{ "switch[19]", switch_19_p34},
		{ "l3_caches[19]", l3_cache_19_p34},

		/*-----------------*/

		{ "l2_caches[20]", l2_cache_20_p34},
		{ "switch[20]", switch_20_p34},
		{ "l3_caches[20]", l3_cache_20_p34},

		{ "l2_caches[21]", l2_cache_21_p34},
		{ "switch[21]", switch_21_p34},
		{ "l3_caches[21]", l3_cache_21_p34},

		{ "l2_caches[22]", l2_cache_22_p34},
		{ "switch[22]", switch_22_p34},
		{ "l3_caches[22]", l3_cache_22_p34},

		{ "l2_caches[23]", l2_cache_23_p34},
		{ "switch[23]", switch_23_p34},
		{ "l3_caches[23]", l3_cache_23_p34},

		/*-----------------*/

		{ "l2_caches[24]", l2_cache_24_p34},
		{ "switch[24]", switch_24_p34},
		{ "l3_caches[24]", l3_cache_24_p34},

		{ "l2_caches[25]", l2_cache_25_p34},
		{ "switch[25]", switch_25_p34},
		{ "l3_caches[25]", l3_cache_25_p34},

		{ "l2_caches[26]", l2_cache_26_p34},
		{ "switch[26]", switch_26_p34},
		{ "l3_caches[26]", l3_cache_26_p34},

		{ "l2_caches[27]", l2_cache_27_p34},
		{ "switch[27]", switch_27_p34},
		{ "l3_caches[27]", l3_cache_27_p34},

		/*-----------------*/

		{ "l2_caches[28]", l2_cache_28_p34},
		{ "switch[28]", switch_28_p34},
		{ "l3_caches[28]", l3_cache_28_p34},

		{ "l2_caches[29]", l2_cache_29_p34},
		{ "switch[29]", switch_29_p34},
		{ "l3_caches[29]", l3_cache_29_p34},

		{ "l2_caches[30]", l2_cache_30_p34},
		{ "switch[30]", switch_30_p34},
		{ "l3_caches[30]", l3_cache_30_p34},

		{ "l2_caches[31]", l2_cache_31_p34},
		{ "switch[31]", switch_31_p34},
		{ "l3_caches[31]", l3_cache_31_p34},

		{ "l2_caches[32]", l2_cache_32_p34},
		{ "switch[32]", switch_32_p34},
		{ "l3_caches[32]", l3_cache_32_p34},

		{ "l2_caches[33]", l2_cache_33_p34},
		{ "switch[33]", switch_33_p34},
		{ "l3_caches[33]", l3_cache_33_p34},

		{ "hub_iommu", hub_iommu_34_p34},
		{ "switch[34]", switch_34_p34},
		{ "sys_agent", sys_agent_34_p34},
		}
};

struct str_map_t node_strn_map_p32 =
{ node_number_p32, {
		{ "l2_caches[0]", l2_cache_0_p32},
		{ "switch[0]", switch_0_p32},
		{ "l3_caches[0]", l3_cache_0_p32},

		{ "l2_caches[1]", l2_cache_1_p32},
		{ "switch[1]", switch_1_p32},
		{ "l3_caches[1]", l3_cache_1_p32},

		{ "l2_caches[2]", l2_cache_2_p32},
		{ "switch[2]", switch_2_p32},
		{ "l3_caches[2]", l3_cache_2_p32},

		{ "l2_caches[3]", l2_cache_3_p32},
		{ "switch[3]", switch_3_p32},
		{ "l3_caches[3]", l3_cache_3_p32},

		/*-----------------*/

		{ "l2_caches[4]", l2_cache_4_p32},
		{ "switch[4]", switch_4_p32},
		{ "l3_caches[4]", l3_cache_4_p32},

		{ "l2_caches[5]", l2_cache_5_p32},
		{ "switch[5]", switch_5_p32},
		{ "l3_caches[5]", l3_cache_5_p32},

		{ "l2_caches[6]", l2_cache_6_p32},
		{ "switch[6]", switch_6_p32},
		{ "l3_caches[6]", l3_cache_6_p32},

		{ "l2_caches[7]", l2_cache_7_p32},
		{ "switch[7]", switch_7_p32},
		{ "l3_caches[7]", l3_cache_7_p32},

		/*-----------------*/

		{ "l2_caches[8]", l2_cache_8_p32},
		{ "switch[8]", switch_8_p32},
		{ "l3_caches[8]", l3_cache_8_p32},

		{ "l2_caches[9]", l2_cache_9_p32},
		{ "switch[9]", switch_9_p32},
		{ "l3_caches[9]", l3_cache_9_p32},

		{ "l2_caches[10]", l2_cache_10_p32},
		{ "switch[10]", switch_10_p32},
		{ "l3_caches[10]", l3_cache_10_p32},

		{ "l2_caches[11]", l2_cache_11_p32},
		{ "switch[11]", switch_11_p32},
		{ "l3_caches[11]", l3_cache_11_p32},

		/*-----------------*/

		{ "l2_caches[12]", l2_cache_12_p32},
		{ "switch[12]", switch_12_p32},
		{ "l3_caches[12]", l3_cache_12_p32},

		{ "l2_caches[13]", l2_cache_13_p32},
		{ "switch[13]", switch_13_p32},
		{ "l3_caches[13]", l3_cache_13_p32},

		{ "l2_caches[14]", l2_cache_14_p32},
		{ "switch[14]", switch_14_p32},
		{ "l3_caches[14]", l3_cache_14_p32},

		{ "l2_caches[15]", l2_cache_15_p32},
		{ "switch[15]", switch_15_p32},
		{ "l3_caches[15]", l3_cache_15_p32},

		/*-----------------*/

		{ "l2_caches[16]", l2_cache_16_p32},
		{ "switch[16]", switch_16_p32},
		{ "l3_caches[16]", l3_cache_16_p32},

		{ "l2_caches[17]", l2_cache_17_p32},
		{ "switch[17]", switch_17_p32},
		{ "l3_caches[17]", l3_cache_17_p32},

		{ "l2_caches[18]", l2_cache_18_p32},
		{ "switch[18]", switch_18_p32},
		{ "l3_caches[18]", l3_cache_18_p32},

		{ "l2_caches[19]", l2_cache_19_p32},
		{ "switch[19]", switch_19_p32},
		{ "l3_caches[19]", l3_cache_19_p32},

		/*-----------------*/

		{ "l2_caches[20]", l2_cache_20_p32},
		{ "switch[20]", switch_20_p32},
		{ "l3_caches[20]", l3_cache_20_p32},

		{ "l2_caches[21]", l2_cache_21_p32},
		{ "switch[21]", switch_21_p32},
		{ "l3_caches[21]", l3_cache_21_p32},

		{ "l2_caches[22]", l2_cache_22_p32},
		{ "switch[22]", switch_22_p32},
		{ "l3_caches[22]", l3_cache_22_p32},

		{ "l2_caches[23]", l2_cache_23_p32},
		{ "switch[23]", switch_23_p32},
		{ "l3_caches[23]", l3_cache_23_p32},

		/*-----------------*/

		{ "l2_caches[24]", l2_cache_24_p32},
		{ "switch[24]", switch_24_p32},
		{ "l3_caches[24]", l3_cache_24_p32},

		{ "l2_caches[25]", l2_cache_25_p32},
		{ "switch[25]", switch_25_p32},
		{ "l3_caches[25]", l3_cache_25_p32},

		{ "l2_caches[26]", l2_cache_26_p32},
		{ "switch[26]", switch_26_p32},
		{ "l3_caches[26]", l3_cache_26_p32},

		{ "l2_caches[27]", l2_cache_27_p32},
		{ "switch[27]", switch_27_p32},
		{ "l3_caches[27]", l3_cache_27_p32},

		/*-----------------*/

		{ "l2_caches[28]", l2_cache_28_p32},
		{ "switch[28]", switch_28_p32},
		{ "l3_caches[28]", l3_cache_28_p32},

		{ "l2_caches[29]", l2_cache_29_p32},
		{ "switch[29]", switch_29_p32},
		{ "l3_caches[29]", l3_cache_29_p32},

		{ "l2_caches[30]", l2_cache_30_p32},
		{ "switch[30]", switch_30_p32},
		{ "l3_caches[30]", l3_cache_30_p32},

		{ "l2_caches[31]", l2_cache_31_p32},
		{ "switch[31]", switch_31_p32},
		{ "l3_caches[31]", l3_cache_31_p32},

		{ "hub_iommu", hub_iommu_32_p32},
		{ "switch[32]", switch_32_p32},
		{ "sys_agent", sys_agent_32_p32},
		}
};

struct str_map_t node_strn_map_p22 =
{ node_number_p22, {
		{ "l2_caches[0]", l2_cache_0_p22},
		{ "switch[0]", switch_0_p22},
		{ "l3_caches[0]", l3_cache_0_p22},

		{ "l2_caches[1]", l2_cache_1_p22},
		{ "switch[1]", switch_1_p22},
		{ "l3_caches[1]", l3_cache_1_p22},

		{ "l2_caches[2]", l2_cache_2_p22},
		{ "switch[2]", switch_2_p22},
		{ "l3_caches[2]", l3_cache_2_p22},

		{ "l2_caches[3]", l2_cache_3_p22},
		{ "switch[3]", switch_3_p22},
		{ "l3_caches[3]", l3_cache_3_p22},

		/*-----------------*/

		{ "l2_caches[4]", l2_cache_4_p22},
		{ "switch[4]", switch_4_p22},
		{ "l3_caches[4]", l3_cache_4_p22},

		{ "l2_caches[5]", l2_cache_5_p22},
		{ "switch[5]", switch_5_p22},
		{ "l3_caches[5]", l3_cache_5_p22},

		{ "l2_caches[6]", l2_cache_6_p22},
		{ "switch[6]", switch_6_p22},
		{ "l3_caches[6]", l3_cache_6_p22},

		{ "l2_caches[7]", l2_cache_7_p22},
		{ "switch[7]", switch_7_p22},
		{ "l3_caches[7]", l3_cache_7_p22},

		/*-----------------*/

		{ "l2_caches[8]", l2_cache_8_p22},
		{ "switch[8]", switch_8_p22},
		{ "l3_caches[8]", l3_cache_8_p22},

		{ "l2_caches[9]", l2_cache_9_p22},
		{ "switch[9]", switch_9_p22},
		{ "l3_caches[9]", l3_cache_9_p22},

		{ "l2_caches[10]", l2_cache_10_p22},
		{ "switch[10]", switch_10_p22},
		{ "l3_caches[10]", l3_cache_10_p22},

		{ "l2_caches[11]", l2_cache_11_p22},
		{ "switch[11]", switch_11_p22},
		{ "l3_caches[11]", l3_cache_11_p22},

		/*-----------------*/

		{ "l2_caches[12]", l2_cache_12_p22},
		{ "switch[12]", switch_12_p22},
		{ "l3_caches[12]", l3_cache_12_p22},

		{ "l2_caches[13]", l2_cache_13_p22},
		{ "switch[13]", switch_13_p22},
		{ "l3_caches[13]", l3_cache_13_p22},

		{ "l2_caches[14]", l2_cache_14_p22},
		{ "switch[14]", switch_14_p22},
		{ "l3_caches[14]", l3_cache_14_p22},

		{ "l2_caches[15]", l2_cache_15_p22},
		{ "switch[15]", switch_15_p22},
		{ "l3_caches[15]", l3_cache_15_p22},

		/*-----------------*/

		{ "l2_caches[16]", l2_cache_16_p22},
		{ "switch[16]", switch_16_p22},
		{ "l3_caches[16]", l3_cache_16_p22},

		{ "l2_caches[17]", l2_cache_17_p22},
		{ "switch[17]", switch_17_p22},
		{ "l3_caches[17]", l3_cache_17_p22},

		{ "l2_caches[18]", l2_cache_18_p22},
		{ "switch[18]", switch_18_p22},
		{ "l3_caches[18]", l3_cache_18_p22},

		{ "l2_caches[19]", l2_cache_19_p22},
		{ "switch[19]", switch_19_p22},
		{ "l3_caches[19]", l3_cache_19_p22},

		/*-----------------*/

		{ "l2_caches[20]", l2_cache_20_p22},
		{ "switch[20]", switch_20_p22},
		{ "l3_caches[20]", l3_cache_20_p22},

		{ "l2_caches[21]", l2_cache_21_p22},
		{ "switch[21]", switch_21_p22},
		{ "l3_caches[21]", l3_cache_21_p22},

		{ "hub_iommu", hub_iommu_22_p22},
		{ "switch[22]", switch_22_p22},
		{ "sys_agent", sys_agent_22_p22},
		}
};

struct str_map_t node_strn_map_p18 =
{ node_number_p18, {
		{ "l2_caches[0]", l2_cache_0_p18},
		{ "switch[0]", switch_0_p18},
		{ "l3_caches[0]", l3_cache_0_p18},

		{ "l2_caches[1]", l2_cache_1_p18},
		{ "switch[1]", switch_1_p18},
		{ "l3_caches[1]", l3_cache_1_p18},

		{ "l2_caches[2]", l2_cache_2_p18},
		{ "switch[2]", switch_2_p18},
		{ "l3_caches[2]", l3_cache_2_p18},

		{ "l2_caches[3]", l2_cache_3_p18},
		{ "switch[3]", switch_3_p18},
		{ "l3_caches[3]", l3_cache_3_p18},

		/*-----------------*/

		{ "l2_caches[4]", l2_cache_4_p18},
		{ "switch[4]", switch_4_p18},
		{ "l3_caches[4]", l3_cache_4_p18},

		{ "l2_caches[5]", l2_cache_5_p18},
		{ "switch[5]", switch_5_p18},
		{ "l3_caches[5]", l3_cache_5_p18},

		{ "l2_caches[6]", l2_cache_6_p18},
		{ "switch[6]", switch_6_p18},
		{ "l3_caches[6]", l3_cache_6_p18},

		{ "l2_caches[7]", l2_cache_7_p18},
		{ "switch[7]", switch_7_p18},
		{ "l3_caches[7]", l3_cache_7_p18},

		/*-----------------*/

		{ "l2_caches[8]", l2_cache_8_p18},
		{ "switch[8]", switch_8_p18},
		{ "l3_caches[8]", l3_cache_8_p18},

		{ "l2_caches[9]", l2_cache_9_p18},
		{ "switch[9]", switch_9_p18},
		{ "l3_caches[9]", l3_cache_9_p18},

		{ "l2_caches[10]", l2_cache_10_p18},
		{ "switch[10]", switch_10_p18},
		{ "l3_caches[10]", l3_cache_10_p18},

		{ "l2_caches[11]", l2_cache_11_p18},
		{ "switch[11]", switch_11_p18},
		{ "l3_caches[11]", l3_cache_11_p18},

		/*-----------------*/

		{ "l2_caches[12]", l2_cache_12_p18},
		{ "switch[12]", switch_12_p18},
		{ "l3_caches[12]", l3_cache_12_p18},

		{ "l2_caches[13]", l2_cache_13_p18},
		{ "switch[13]", switch_13_p18},
		{ "l3_caches[13]", l3_cache_13_p18},

		{ "l2_caches[14]", l2_cache_14_p18},
		{ "switch[14]", switch_14_p18},
		{ "l3_caches[14]", l3_cache_14_p18},

		{ "l2_caches[15]", l2_cache_15_p18},
		{ "switch[15]", switch_15_p18},
		{ "l3_caches[15]", l3_cache_15_p18},

		{ "l2_caches[16]", l2_cache_16_p18},
		{ "switch[16]", switch_16_p18},
		{ "l3_caches[16]", l3_cache_16_p18},

		{ "l2_caches[17]", l2_cache_17_p18},
		{ "switch[17]", switch_17_p18},
		{ "l3_caches[17]", l3_cache_17_p18},

		{ "hub_iommu", hub_iommu_18_p18},
		{ "switch[18]", switch_18_p18},
		{ "sys_agent", sys_agent_18_p18},
		}
};

struct str_map_t node_strn_map_p16 =
{ node_number_p16, {
		{ "l2_caches[0]", l2_cache_0_p16},
		{ "switch[0]", switch_0_p16},
		{ "l3_caches[0]", l3_cache_0_p16},

		{ "l2_caches[1]", l2_cache_1_p16},
		{ "switch[1]", switch_1_p16},
		{ "l3_caches[1]", l3_cache_1_p16},

		{ "l2_caches[2]", l2_cache_2_p16},
		{ "switch[2]", switch_2_p16},
		{ "l3_caches[2]", l3_cache_2_p16},

		{ "l2_caches[3]", l2_cache_3_p16},
		{ "switch[3]", switch_3_p16},
		{ "l3_caches[3]", l3_cache_3_p16},

		/*-----------------*/

		{ "l2_caches[4]", l2_cache_4_p16},
		{ "switch[4]", switch_4_p16},
		{ "l3_caches[4]", l3_cache_4_p16},

		{ "l2_caches[5]", l2_cache_5_p16},
		{ "switch[5]", switch_5_p16},
		{ "l3_caches[5]", l3_cache_5_p16},

		{ "l2_caches[6]", l2_cache_6_p16},
		{ "switch[6]", switch_6_p16},
		{ "l3_caches[6]", l3_cache_6_p16},

		{ "l2_caches[7]", l2_cache_7_p16},
		{ "switch[7]", switch_7_p16},
		{ "l3_caches[7]", l3_cache_7_p16},

		/*-----------------*/

		{ "l2_caches[8]", l2_cache_8_p16},
		{ "switch[8]", switch_8_p16},
		{ "l3_caches[8]", l3_cache_8_p16},

		{ "l2_caches[9]", l2_cache_9_p16},
		{ "switch[9]", switch_9_p16},
		{ "l3_caches[9]", l3_cache_9_p16},

		{ "l2_caches[10]", l2_cache_10_p16},
		{ "switch[10]", switch_10_p16},
		{ "l3_caches[10]", l3_cache_10_p16},

		{ "l2_caches[11]", l2_cache_11_p16},
		{ "switch[11]", switch_11_p16},
		{ "l3_caches[11]", l3_cache_11_p16},

		/*-----------------*/

		{ "l2_caches[12]", l2_cache_12_p16},
		{ "switch[12]", switch_12_p16},
		{ "l3_caches[12]", l3_cache_12_p16},

		{ "l2_caches[13]", l2_cache_13_p16},
		{ "switch[13]", switch_13_p16},
		{ "l3_caches[13]", l3_cache_13_p16},

		{ "l2_caches[14]", l2_cache_14_p16},
		{ "switch[14]", switch_14_p16},
		{ "l3_caches[14]", l3_cache_14_p16},

		{ "l2_caches[15]", l2_cache_15_p16},
		{ "switch[15]", switch_15_p16},
		{ "l3_caches[15]", l3_cache_15_p16},

		{ "hub_iommu", hub_iommu_16_p16},
		{ "switch[16]", switch_16_p16},
		{ "sys_agent", sys_agent_16_p16},
		}
};

struct str_map_t node_strn_map_p8 =
{ node_number_p8, {
		{ "l2_caches[0]", l2_cache_0_p8},
		{ "switch[0]", switch_0_p8},
		{ "l3_caches[0]", l3_cache_0_p8},

		{ "l2_caches[1]", l2_cache_1_p8},
		{ "switch[1]", switch_1_p8},
		{ "l3_caches[1]", l3_cache_1_p8},

		{ "l2_caches[2]", l2_cache_2_p8},
		{ "switch[2]", switch_2_p8},
		{ "l3_caches[2]", l3_cache_2_p8},

		{ "l2_caches[3]", l2_cache_3_p8},
		{ "switch[3]", switch_3_p8},
		{ "l3_caches[3]", l3_cache_3_p8},

		/*-----------------*/

		{ "l2_caches[4]", l2_cache_4_p8},
		{ "switch[4]", switch_4_p8},
		{ "l3_caches[4]", l3_cache_4_p8},

		{ "l2_caches[5]", l2_cache_5_p8},
		{ "switch[5]", switch_5_p8},
		{ "l3_caches[5]", l3_cache_5_p8},

		{ "l2_caches[6]", l2_cache_6_p8},
		{ "switch[6]", switch_6_p8},
		{ "l3_caches[6]", l3_cache_6_p8},

		{ "l2_caches[7]", l2_cache_7_p8},
		{ "switch[7]", switch_7_p8},
		{ "l3_caches[7]", l3_cache_7_p8},

		{ "hub_iommu", hub_iommu_8_p8},
		{ "switch[8]", switch_8_p8},
		{ "sys_agent", sys_agent_8_p8},
		}
};

struct str_map_t node_strn_map_p4 =
{ node_number_p4, {
		{ "l2_caches[0]", l2_cache_0_p4},
		{ "switch[0]", switch_0_p4},
		{ "l3_caches[0]", l3_cache_0_p4},

		{ "l2_caches[1]", l2_cache_1_p4},
		{ "switch[1]", switch_1_p4},
		{ "l3_caches[1]", l3_cache_1_p4},

		{ "l2_caches[2]", l2_cache_2_p4},
		{ "switch[2]", switch_2_p4},
		{ "l3_caches[2]", l3_cache_2_p4},

		{ "l2_caches[3]", l2_cache_3_p4},
		{ "switch[3]", switch_3_p4},
		{ "l3_caches[3]", l3_cache_3_p4},

		{ "hub_iommu", hub_iommu_4_p4},
		{ "switch[4]", switch_4_p4},
		{ "sys_agent", sys_agent_4_p4},
		}
};

struct str_map_t port_name_map =
{ 	port_num, {
		{ "north_queue", north_queue},
		{ "east_queue", east_queue},
		{ "south_queue", south_queue},
		{ "west_queue", west_queue},
		}
};

struct str_map_t *node_strn_map;

void switch_init(void){

	switch_create();
	//route_create();
	switch_create_tasks();
	return;
}


void switch_create(void){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/

	//for now the number of GPU connected switches is hard coded
	//this one switch for all of the GPU.
	//star todo fix this
	int extras = 1;

	//for now model a ring bus on each CPU
	switches = (void *) calloc((num_cores + extras), sizeof(struct switch_t));

	return;
}

void switch_create_tasks(void){

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/

	//star todo fix this
	int extras = 1;

	char buff[100];
	int i = 0;

	switches_ec = (void *) calloc((num_cores + extras), sizeof(eventcount));
	for(i = 0; i < (num_cores + extras); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_%d", i);
		switches_ec[i] = *(new_eventcount(strdup(buff)));
	}

	switches_tasks = (void *) calloc((num_cores + extras), sizeof(task));
	for(i = 0; i < (num_cores + extras); i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "switch_ctrl_%d", i);
		switches_tasks[i] = *(create_task(switch_ctrl, DEFAULT_STACK_SIZE, strdup(buff)));
	}

	//event counts and tasks for io ctrl are over in configure.

	return;
}

struct crossbar_t *switch_crossbar_create(){

	struct crossbar_t *crossbar;

	crossbar = (void *) malloc(sizeof(struct crossbar_t));

	//set up the cross bar variables
	crossbar->num_ports = 4;
	crossbar->num_pairs = 0;

	crossbar->north_in_out_linked_queue = invalid_queue;
	crossbar->east_in_out_linked_queue = invalid_queue;
	crossbar->south_in_out_linked_queue = invalid_queue;
	crossbar->west_in_out_linked_queue = invalid_queue;

	return crossbar;
}

void switch_crossbar_clear_state(struct switch_t *switches){

	//clear the cross bar state
	switches->crossbar->num_pairs = 0;

	switches->crossbar->north_in_out_linked_queue = invalid_queue;
	switches->crossbar->east_in_out_linked_queue = invalid_queue;
	switches->crossbar->south_in_out_linked_queue = invalid_queue;
	switches->crossbar->west_in_out_linked_queue = invalid_queue;

	return;
}

void switch_set_link(struct switch_t *switches, enum port_name tx_port){

	/*we have the in queue with switches->queue and the tx_queue
	try to link them...*/

	//don't exceed QueueSizes...
	//int tx_north_queue_size = list_count(switches->Tx_north_queue);
	//int tx_east_queue_size = list_count(switches->Tx_east_queue);
	//int tx_south_queue_size = list_count(switches->Tx_south_queue);
	//int tx_west_queue_size = list_count(switches->Tx_west_queue);


	switch(tx_port)
	{
		//North queues
		case north_queue:
			assert(switches->queue != north_queue); //this is saying that your output queue can't be your input queue
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					if((list_count(switches->north_tx_request_queue) < QueueSize) && (switches->crossbar->north_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->north_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_reply:
					if((list_count(switches->north_tx_reply_queue) < QueueSize) && (switches->crossbar->north_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->north_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_coherenece:
					if((list_count(switches->north_tx_coherence_queue) < QueueSize) && (switches->crossbar->north_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->north_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//East queues
		case east_queue:
			assert(switches->queue != east_queue); //this is saying that your output queue can't be your input queue
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					if((list_count(switches->east_tx_request_queue) < QueueSize) && (switches->crossbar->east_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->east_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_reply:
					if((list_count(switches->east_tx_reply_queue) < QueueSize) && (switches->crossbar->east_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->east_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_coherenece:
					if((list_count(switches->east_tx_coherence_queue) < QueueSize) && (switches->crossbar->east_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->east_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//South queues
		case south_queue:
			assert(switches->queue != south_queue); //this is saying that your output queue can't be your input queue
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					if((list_count(switches->south_tx_request_queue) < QueueSize) && (switches->crossbar->south_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->south_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_reply:
					if((list_count(switches->south_tx_reply_queue) < QueueSize) && (switches->crossbar->south_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->south_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_coherenece:
					if((list_count(switches->south_tx_coherence_queue) < QueueSize) && (switches->crossbar->south_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->south_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//West queues
		case west_queue:
			assert(switches->queue != west_queue); //this is saying that your output queue can't be your input queue
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					if((list_count(switches->west_tx_request_queue) < QueueSize) && (switches->crossbar->west_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->west_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_reply:
					if((list_count(switches->west_tx_reply_queue) < QueueSize) && (switches->crossbar->west_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->west_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_coherenece:
					if((list_count(switches->west_tx_coherence_queue) < QueueSize) && (switches->crossbar->west_in_out_linked_queue == invalid_queue))
					{
							switches->crossbar->west_in_out_linked_queue = switches->queue;
							switches->crossbar->num_pairs++;
					}
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//Error checking
		case invalid_queue:
		default:
			fatal("switch_get_rx_packet() Invalid port name\n");
			break;
	}


	//check if the out on the cross bar is busy. If not assign the link.
	/*if(tx_port == north_queue && (tx_north_queue_size < QueueSize))
	{
		if(switches->queue == north_queue)
			printf("error here cycle %llu\n", P_TIME);

		assert(switches->queue != north_queue);
		if(switches->crossbar->north_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->north_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}*/

	/*if(tx_port == east_queue && (tx_east_queue_size < QueueSize))
	{
		assert(switches->queue != east_queue);
		if(switches->crossbar->east_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->east_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}*/

	/*if(tx_port == south_queue && (tx_south_queue_size < QueueSize))
	{
		assert(switches->queue != south_queue);
		if(switches->crossbar->south_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->south_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}*/

	/*if(tx_port == west_queue && (tx_west_queue_size < QueueSize))
	{
		assert(switches->queue != west_queue);
		if(switches->crossbar->west_in_out_linked_queue == invalid_queue)
		{
			switches->crossbar->west_in_out_linked_queue = switches->queue;
			switches->crossbar->num_pairs++;
		}
	}*/

	/*if(tx_north_queue_size > QueueSize)
		warning("switch_set_link(): tx_north_queue full size %d cycle %llu\n", tx_north_queue_size, P_TIME);

	if(tx_east_queue_size > QueueSize)
		warning("switch_set_link(): tx_east_queue full size %d cycle %llu\n", tx_east_queue_size, P_TIME);

	if(tx_south_queue_size > QueueSize)
		warning("switch_set_link(): tx_south_queue full size %d cycle %llu\n", tx_south_queue_size, P_TIME);

	if(tx_west_queue_size > QueueSize)
		warning("switch_set_link(): tx_west_queue full size %d cycle %llu\n", tx_west_queue_size, P_TIME);*/

	return;
}

enum port_name switch_get_route(struct switch_t *switches, struct cgm_packet_t *message_packet){

	enum port_name tx_port = invalid_queue;

	int dest_node;
	int src_node;
	int switch_node = switches->switch_node_number;
	float distance;

	//send the packet to it's destination OR on to the next hop
	//look up the node number of the destination
	dest_node = message_packet->dest_id;
	src_node = message_packet->src_id;

	/*if(message_packet->access_id == 1)
	{
		warning("dest id %d switch id %d\n", dest_node, switch_node);
		getchar();
	}*/

	//if dest is an L2/L3/HUB-IOMMU/SA connected to this switch.
	if(dest_node == (switch_node - 1) || dest_node == (switch_node + 1))
	{
		//if the node number is lower this means it is an L2 cache or HUB IOMMU
		if(dest_node < switch_node)
		{
			tx_port = north_queue; //switches[switches->switch_id].Tx_north_queue;
		}
		//if the node number is high this means it is an L3 cache or the SYS Agent
		else if(dest_node > switch_node)
		{
			tx_port = south_queue; //switches[switches->switch_id].Tx_south_queue;
		}
		else
		{
			fatal("switch_get_route() bad dest (source = dest) cycle %llu\n", P_TIME);
		}
	}
	else
	{
		//dest should not equal the source

		if(dest_node == src_node)
		{
			fatal("crashing switch dest_node == src_node id %llu phy addr 0x%08x vtl addr 0x%08x src %d dest %d\n",
				message_packet->access_id,
				(message_packet->address & l2_caches[0].block_address_mask),
				(message_packet->vtladdress & l2_caches[0].block_address_mask),
				src_node,
				dest_node);
		}

		assert(dest_node != src_node);

		//send packet to adjacent switch
		//there is no transfer direction established.
		if(switches->queue == north_queue || switches->queue == south_queue)
		{
			//new packets from connected L2 or L3 cache.
			if(dest_node > src_node)
			{
				distance = switch_get_distance(dest_node, src_node);

				//go in the direction with the shortest number of hops.
				if(distance <= switches->switch_median_node)
				{//go east

					tx_port = east_queue; //switches[switches->switch_id].Tx_east_queue;
				}
				else
				{//go west
					tx_port = west_queue; //switches[switches->switch_id].Tx_west_queue;
				}
			}
			else if(dest_node < src_node)
			{
				distance = switch_get_distance(dest_node, src_node);

				//go in the direction with the shortest number of hops.
				if(distance <= switches->switch_median_node)
				{//go west
					tx_port = west_queue; //switches[switches->switch_id].Tx_west_queue;
				}
				else
				{//go east
					tx_port = east_queue; //switches[switches->switch_id].Tx_east_queue;
				}
			}
		}
		else if(switches->queue == east_queue || switches->queue == west_queue)
		{
			//packet came from another switch, but needs to continue on.
			if(switches->queue == east_queue)
			{//continue going west
				tx_port = west_queue; //switches[switches->switch_id].Tx_west_queue;
			}
			else if(switches->queue == west_queue)
			{//continue going east
				tx_port = east_queue; //switches[switches->switch_id].Tx_east_queue;
			}
			else
			{
				fatal("switch_get_route() bad queue look into this cycle %llu\n", P_TIME);
			}
		}
		else
		{
			fatal("switch_get_route(): directional queue error as %d.\n", switches->queue);
		}
	}

	if(tx_port == invalid_queue)
	{

		printf("\tswtich routing id %llu block 0x%08x src %d dest %d\n",
				message_packet->access_id, (message_packet->address & l2_caches[0].block_address_mask), src_node, dest_node);
	}

	assert(tx_port != invalid_queue);
	return tx_port;
}


void switch_crossbar_link(struct switch_t *switches){

	struct cgm_packet_t *packet = NULL;
	//struct list_t *switch_rx_queue = NULL;
	enum port_name tx_port = invalid_queue;
	int i = 0;

	if(switches->arb_style == round_robin)
	{
		/*printf("start_2 queue %d\n", switches->queue);*/

		for(i = 0; i < switches->crossbar->num_ports; i++)
		{
			//see if there is a packet waiting...

			packet = switch_get_rx_packet(switches);

			//found a new packet, try to link it...
			if(packet)
			{

				tx_port = switch_get_route(switches, packet);

				//try to assign the link
				/*if(P_TIME > 466500000)
				{
					printf("Switch id %d setting link for blk 0x%08x and blk 0x%08x id %llu type %d\n",
						 switches->switch_id,
						 packet->address & l2_caches[0].block_address_mask,
						 packet->vtladdress & l2_caches[0].block_address_mask,
						 packet->access_id,
						 packet->access_type);

					fflush(stdout);
					fflush(stderr);
				}*/


				switch_set_link(switches, tx_port);
			}


			/*if(switches->queue == east_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->east_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					printf("\teast hit\n");
					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					switch_set_link(switches, tx_queue);
				}
			}

			if(switches->queue == south_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->south_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					printf("\tsouth hit\n");
					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					switch_set_link(switches, tx_queue);
				}
			}

			if(switches->queue == west_queue)
			{
				//see if there is a packet waiting...
				packet = list_get(switches->west_queue, 0);

				//found a new packet, try to link it...
				if(packet)
				{
					printf("\twest hit\n");

					tx_queue = switch_get_route(switches, packet);

					//try to assign the link
					switch_set_link(switches, tx_queue);
				}
			}*/

			assert(switches->queue >= 1 && switches->queue <= switches->crossbar->num_ports);

			//advance the queue pointer.
			switches->queue = get_next_queue_rb(switches->queue);

		}
	}
	else
	{
		fatal("switch_crossbar_link(): Invalid ARB set\n");
	}

	//we should have at least one pair, but no more than the number of ports on the switch.
	assert(switches->crossbar->num_pairs >= 0 && switches->crossbar->num_pairs <= switches->crossbar->num_ports);
	return;
}


int switch_can_access(struct list_t *queue){

	//check if target queue is full
	if(QueueSize <= list_count(queue))
	{
		return 0;
	}

	return 1;
}

enum port_name crossbar_get_port_link_status(struct switch_t *switches){

	enum port_name port_status = invalid_queue;

	//North queues
	switch(switches->crossbar->current_port)
	{
		case north_queue:
			port_status = switches->crossbar->north_in_out_linked_queue;
			break;
		case east_queue:
			port_status = switches->crossbar->east_in_out_linked_queue;
			break;
		case south_queue:
			port_status = switches->crossbar->south_in_out_linked_queue;
			break;
		case west_queue:
			port_status = switches->crossbar->west_in_out_linked_queue;
			break;
		case invalid_queue:
		default:
			fatal("switch_get_rx_packet() Invalid port name\n");
			break;
	}

	return port_status;
}


eventcount volatile *switch_get_io_ec_counter(struct switch_t *switches){

	eventcount volatile * switch_io_ec = NULL;

	switch(switches->crossbar->current_port)
	{
		case north_queue:
			switch_io_ec = switches->switches_north_io_ec;
			break;
		case east_queue:
			switch_io_ec = switches->switches_east_io_ec;
			break;
		case south_queue:
			switch_io_ec = switches->switches_south_io_ec;
			break;
		case west_queue:
			switch_io_ec = switches->switches_west_io_ec;
			break;
		case invalid_queue:
		default:
			fatal("switch_get_rx_packet() Invalid port name\n");
			break;
	}

	return switch_io_ec;
}


void switch_ctrl(void){

	int my_pid = switch_pid++;
	int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;
	struct cgm_packet_t *message_packet = NULL;
	struct list_t *out_queue = NULL;
	long long step = 1;
	int i = 0;

	/*long long queue_depth;*/

	long long occ_start = 0;

	assert(my_pid <= (num_cores + num_cus));

	set_id((unsigned int)my_pid);

	while(1)
	{
		/*we have been advanced. Note that it's possible for a
		switch to be advanced more than once per cycle*/
		await(&switches_ec[my_pid], step);

		occ_start = P_TIME;

		SYSTEM_PAUSE(switches[my_pid].latency);

		//assert(next_queue == switches[my_pid].queue);

		/*models a cross bar.
		link as many inputs to outputs as possible*/
		switch_crossbar_link(&switches[my_pid]);

		//printf("made it here sw id %d cycle %llu\n", system_agent->switch_id, P_TIME);
			//cache_dump_queue(switches[system_agent->switch_id].south_rx_reply_queue);
			//printf("\n\n");



		for(i = 0; i < switches[my_pid].crossbar->num_ports; i++)
		{

			if(crossbar_get_port_link_status(&switches[my_pid]) != invalid_queue)
			{
				/*the out queue is linked to an input queue and the output queue isn't full
				move the packet from the input queue to the correct output queue*/

				//careful, the next line is for the INPUT queue...
				message_packet = list_remove_at(switch_get_rx_queue(&switches[my_pid], crossbar_get_port_link_status(&switches[my_pid])), 0);
				assert(message_packet);

				DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing %d access id %llu type %d cycle %llu\n",
						(message_packet->address & ~mem_ctrl->block_mask), switches[my_pid].name, switches[my_pid].crossbar->current_port, message_packet->access_id, message_packet->access_type, P_TIME);

				out_queue = switch_get_tx_queue(&switches[my_pid], switches[my_pid].crossbar->current_port);
				assert(out_queue);


				/*if(message_packet->access_id == 1)
					warning("%s Sending id %llu src %s dest %s out queue %s cycle %llu\n",
						switches[my_pid].name, message_packet->access_id, message_packet->src_name, message_packet->dest_name, out_queue->name, P_TIME);*/

				if(list_count(out_queue) > QueueSize)
					warning("%s size = %d\n", out_queue->name, list_count(out_queue));

				/*if(message_packet->evict_id == 203170)
					printf("block 0x%08x %s routing %d access evict id %llu type %d out queue %s cycle %llu\n",
						(message_packet->address & ~mem_ctrl->block_mask),
						switches[my_pid].name,
						switches[my_pid].crossbar->current_port,
						message_packet->evict_id,
						message_packet->access_type,
						out_queue->name,
						P_TIME);*/

				list_enqueue(out_queue, message_packet);
				advance(switch_get_io_ec_counter(&switches[my_pid]));

				/*if(switches[my_pid].switch_id == 8)
					cache_dump_queue(out_queue);*/

				/*stats*/
				//switches[my_pid].north_tx_inserts++;
				//queue_depth = list_count(switches[my_pid].Tx_north_queue);
				/*max depth*/
				//if(queue_depth > switches[my_pid].north_txqueue_max_depth)
				//		switches[my_pid].north_txqueue_max_depth = queue_depth;

				/*ave depth = ((old count * old data) + next data) / next count*/
				//switches[my_pid].north_txqueue_ave_depth =
				//		((((double) switches[my_pid].north_tx_inserts - 1) * switches[my_pid].north_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].north_tx_inserts;

				message_packet = NULL;

			}

			switches[my_pid].crossbar->current_port = get_next_queue_rb(switches[my_pid].crossbar->current_port);
		}



		//-------------------------------

		/*crossbar state is set. now run through and move each packet as required.*/
		/*if(switches[my_pid].crossbar->north_in_out_linked_queue != invalid_queue)
		{
			the north out queue is linked to an input queue and the output queue isn't full
			move the packet from the input queue to the correct output queue
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->north_in_out_linked_queue), 0);
			assert(message_packet);

			DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing north ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), switches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);

			if(list_count(switches[my_pid].Tx_north_queue) > QueueSize)
				warning("%s size = %d\n", switches[my_pid].Tx_north_queue->name, list_count(switches[my_pid].Tx_north_queue));

			list_enqueue(switches[my_pid].Tx_north_queue, message_packet);
			advance(switches[my_pid].switches_north_io_ec);

			stats
			switches[my_pid].north_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_north_queue);
			max depth
			if(queue_depth > switches[my_pid].north_txqueue_max_depth)
					switches[my_pid].north_txqueue_max_depth = queue_depth;

			ave depth = ((old count * old data) + next data) / next count
			switches[my_pid].north_txqueue_ave_depth =
					((((double) switches[my_pid].north_tx_inserts - 1) * switches[my_pid].north_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].north_tx_inserts;

			message_packet = NULL;
		}*/

		/*if(switches[my_pid].crossbar->east_in_out_linked_queue != invalid_queue)
		{
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->east_in_out_linked_queue), 0);
			assert(message_packet);

			DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing east ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), switches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);

			if(list_count(switches[my_pid].Tx_east_queue) > QueueSize)
				warning("%s size = %d\n", switches[my_pid].Tx_east_queue->name, list_count(switches[my_pid].Tx_east_queue));

			list_enqueue(switches[my_pid].Tx_east_queue, message_packet);
			advance(switches[my_pid].switches_east_io_ec);

			stats
			switches[my_pid].east_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_east_queue);
			max depth
			if(queue_depth > switches[my_pid].east_txqueue_max_depth)
					switches[my_pid].east_txqueue_max_depth = queue_depth;

			ave depth = ((old count * old data) + next data) / next count
			switches[my_pid].east_txqueue_ave_depth =
					((((double) switches[my_pid].east_tx_inserts - 1) * switches[my_pid].east_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].east_tx_inserts;

			message_packet = NULL;
		}*/

		/*if(switches[my_pid].crossbar->south_in_out_linked_queue != invalid_queue)
		{
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->south_in_out_linked_queue), 0);
			assert(message_packet);

			DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing south ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), switches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);

			if(list_count(switches[my_pid].Tx_south_queue) > QueueSize)
				warning("%s size = %d\n", switches[my_pid].Tx_south_queue->name, list_count(switches[my_pid].Tx_south_queue));

			list_enqueue(switches[my_pid].Tx_south_queue, message_packet);
			advance(switches[my_pid].switches_south_io_ec);

			stats
			switches[my_pid].south_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_south_queue);
			max depth
			if(queue_depth > switches[my_pid].south_txqueue_max_depth)
					switches[my_pid].south_txqueue_max_depth = queue_depth;

			ave depth = ((old count * old data) + next data) / next count
			switches[my_pid].south_txqueue_ave_depth =
					((((double) switches[my_pid].south_tx_inserts - 1) * switches[my_pid].south_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].south_tx_inserts;

			message_packet = NULL;
		}*/

		/*if(switches[my_pid].crossbar->west_in_out_linked_queue != invalid_queue)
		{
			message_packet = list_remove_at(switch_get_in_queue(&switches[my_pid], switches[my_pid].crossbar->west_in_out_linked_queue), 0);
			assert(message_packet);

			DEBUGSYS(SYSTEM == 1, "block 0x%08x %s routing west ID %llu type %d cycle %llu\n",
					(message_packet->address & ~mem_ctrl->block_mask), switches[my_pid].name, message_packet->access_id, message_packet->access_type, P_TIME);

			if(list_count(switches[my_pid].Tx_west_queue) > QueueSize)
				warning("%s size = %d\n", switches[my_pid].Tx_west_queue->name, list_count(switches[my_pid].Tx_west_queue));

			list_enqueue(switches[my_pid].Tx_west_queue, message_packet);
			advance(switches[my_pid].switches_west_io_ec);

			stats
			switches[my_pid].west_tx_inserts++;
			queue_depth = list_count(switches[my_pid].Tx_west_queue);
			max depth
			if(queue_depth > switches[my_pid].west_txqueue_max_depth)
					switches[my_pid].west_txqueue_max_depth = queue_depth;

			ave depth = ((old count * old data) + next data) / next count
			switches[my_pid].west_txqueue_ave_depth =
					((((double) switches[my_pid].west_tx_inserts - 1) * switches[my_pid].west_txqueue_ave_depth) + (double) queue_depth) / (double) switches[my_pid].west_tx_inserts;

			message_packet = NULL;
		}*/

		/*stats*/
		if(switches[my_pid].switch_max_links < switches[my_pid].crossbar->num_pairs)
			switches[my_pid].switch_max_links = switches[my_pid].crossbar->num_pairs;

		switches[my_pid].switch_total_links += switches[my_pid].crossbar->num_pairs;
		//switches[my_pid].switch_total_wakes++;

		//increase step by number of pairs formed
		step += switches[my_pid].crossbar->num_pairs;

		//clear the current cross bar state
		switch_crossbar_clear_state(&switches[my_pid]);

		//set the next direction and lane to process
		switches[my_pid].next_crossbar_lane = switch_get_next_crossbar_lane(switches[my_pid].next_crossbar_lane);
		switches[my_pid].crossbar->current_port = get_next_queue_rb(switches[my_pid].crossbar->current_port);
		switches[my_pid].queue = get_next_queue_rb(switches[my_pid].queue);

		//set message_paket null
		message_packet = NULL;


		/*stats occupancy*/
		switches[my_pid].switch_occupance += (P_TIME - occ_start);
	}

	fatal("switch_ctrl() quit\n");
	return;
}


enum switch_crossbar_lane_map switch_get_next_crossbar_lane(enum switch_crossbar_lane_map current_crossbar_lane){

	enum switch_crossbar_lane_map next_lane = crossbar_invalid_lane;

	switch(current_crossbar_lane)
	{
		case crossbar_request:
			next_lane = crossbar_reply;
			break;
		case crossbar_reply:
			next_lane = crossbar_coherenece;
			break;
		case crossbar_coherenece:
			next_lane = crossbar_request;
			break;
		case crossbar_invalid_lane:
		default:
			fatal("get_next_io_lane_rb() Invalid port name\n");
			break;
	}

	assert(next_lane != crossbar_invalid_lane);
	return next_lane;
}


struct cgm_packet_t *switch_get_rx_packet(struct switch_t *switches){

	struct cgm_packet_t *packet = NULL;

	switch(switches->queue)
	{
		//North queues
		case north_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					packet = list_get(switches->north_rx_request_queue, 0);
					break;
				case crossbar_reply:
					packet = list_get(switches->north_rx_reply_queue, 0);
					break;
				case crossbar_coherenece:
					packet = list_get(switches->north_rx_coherence_queue, 0);
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//East queues
		case east_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					packet = list_get(switches->east_rx_request_queue, 0);
					break;
				case crossbar_reply:
					packet = list_get(switches->east_rx_reply_queue, 0);
					break;
				case crossbar_coherenece:
					packet = list_get(switches->east_rx_coherence_queue, 0);
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//South queues
		case south_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					packet = list_get(switches->south_rx_request_queue, 0);
					break;
				case crossbar_reply:
					packet = list_get(switches->south_rx_reply_queue, 0);
					break;
				case crossbar_coherenece:
					packet = list_get(switches->south_rx_coherence_queue, 0);
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//West queues
		case west_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					packet = list_get(switches->west_rx_request_queue, 0);
					break;
				case crossbar_reply:
					packet = list_get(switches->west_rx_reply_queue, 0);
					break;
				case crossbar_coherenece:
					packet = list_get(switches->west_rx_coherence_queue, 0);
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//Error checking
		case invalid_queue:
		default:
			fatal("switch_get_rx_packet() Invalid port name\n");
			break;
	}

	return packet;
}

enum switch_io_lane_map get_next_io_lane_rb(enum switch_io_lane_map current_io_lane){

	enum switch_io_lane_map next_lane = io_invalid_lane;

	switch(current_io_lane)
	{
		case io_request:
			next_lane = io_reply;
			break;
		case io_reply:
			next_lane = io_coherence;
			break;
		case io_coherence:
			next_lane = io_request;
			break;
		case io_invalid_lane:
		default:
			fatal("get_next_io_lane_rb() Invalid port name\n");
			break;
	}


	assert(next_lane != io_invalid_lane);
	return next_lane;
}



float switch_get_distance(int dest_node, int src_node){

	float distance = 0;

	if(dest_node > src_node)
	{
		//get the distance from this switch to the destination (left to right)
		if(dest_node % 3 == 0 && src_node % 3 == 0)
		{
			//L2 to L2
			distance = (dest_node - src_node)/3;
		}
		else if(dest_node % 3 != 0 && src_node % 3 == 0)
		{
			//L2 to L3/SA || L3 to L2 (works for both ways)
			distance = (dest_node - (src_node + 2))/3;
		}
		else
		{
			//L3 to L3/SA
			distance = (dest_node - src_node)/3;
		}
	}
	else
	{
		//get the distance from this switch to the destination (right to left)
		if(src_node % 3 == 0 && dest_node % 3 == 0)
		{
			//L2 to L2
			distance = (src_node - dest_node)/3;
		}
		else if(src_node % 3 == 0 && dest_node % 3 != 0)
		{
			//L2 to L3/SA || L3 to L2 (works for both ways)
			distance = ((src_node + 2) - dest_node)/3;
		}
		else
		{
			//L3 to L3/SA
			distance = (src_node - dest_node)/3;
		}
	}
	return distance;
}


struct cgm_packet_t *get_from_queue(struct switch_t *switches){

	struct cgm_packet_t *new_packet = NULL;
	int i = 0;

	//choose a port this cycle to work from
	if(switches->arb_style == round_robin)
	{

		//get a packet
		for(i = 0; i < switches->port_num; i++)
		{
			//set switches->queue to the next queue.
			switches->queue = get_next_queue_rb(switches->queue);

			switch(switches->queue)
			{
				case north_queue:
					new_packet = list_get(switches->north_queue, 0);
					switches->current_queue = switches->north_queue;
					break;
				case east_queue:
					new_packet = list_get(switches->east_queue, 0);
					switches->current_queue = switches->east_queue;
					break;
				case south_queue:
					new_packet = list_get(switches->south_queue, 0);
					switches->current_queue = switches->south_queue;
					break;
				case west_queue:
					new_packet = list_get(switches->west_queue, 0);
					switches->current_queue = switches->west_queue;
					break;
				case invalid_queue:
				default:
					fatal("get_next_queue() Invalid port name\n");
					break;
			}

			//if we don't have a message go on to the next.
			/*if(switches->queue == north_queue)
			{
				new_packet = list_get(switches->north_queue, 0);
				switches->current_queue = switches->north_queue;
			}
			else if(switches->queue == east_queue)
			{
				new_packet = list_get(switches->east_queue, 0);
				switches->current_queue = switches->east_queue;
			}
			else if(switches->queue == south_queue)
			{
				new_packet = list_get(switches->south_queue, 0);
				switches->current_queue = switches->south_queue;
			}
			else if(switches->queue == west_queue)
			{
				new_packet = list_get(switches->west_queue, 0);
				switches->current_queue = switches->west_queue;
			}*/

			//when we have a packet break out.
			//next advance start with the next queue
			if(new_packet)
			{
				i = 0;
				break;
			}
		}
	}
	else
	{
		fatal("get_from_queue() invalid arbitration set switch %s\n", switches->name);
	}

	/*CGM_DEBUG(switch_debug_file, "%s access_id %llu cycle %llu get from %s with size %d\n",
			switches->name, new_packet->access_id, P_TIME, (char *)str_map_value(&port_name_map, switches->queue), list_count(switches->current_queue));*/

	assert(new_packet != NULL);
	return new_packet;
}

struct list_t *switch_get_tx_queue(struct switch_t *switches, enum port_name queue){

	struct list_t *switch_queue = NULL;

	switch(queue)
	{
		//North queues
		case north_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->north_tx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->north_tx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->north_tx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//East queues
		case east_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->east_tx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->east_tx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->east_tx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//South queues
		case south_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->south_tx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->south_tx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->south_tx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//West queues
		case west_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->west_tx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->west_tx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->west_tx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//Error checking
		case invalid_queue:
		default:
			fatal("switch_get_rx_packet() Invalid port name\n");
			break;
	}

	assert(switch_queue != NULL);
	return switch_queue;
}


struct list_t *switch_get_rx_queue(struct switch_t *switches, enum port_name queue){

	struct list_t *switch_queue = NULL;

	switch(queue)
	{
		//North queues
		case north_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->north_rx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->north_rx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->north_rx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//East queues
		case east_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->east_rx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->east_rx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->east_rx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//South queues
		case south_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->south_rx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->south_rx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->south_rx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//West queues
		case west_queue:
			switch(switches->next_crossbar_lane)
			{
				case crossbar_request:
					switch_queue = switches->west_rx_request_queue;
					break;
				case crossbar_reply:
					switch_queue = switches->west_rx_reply_queue;
					break;
				case crossbar_coherenece:
					switch_queue = switches->west_rx_coherence_queue;
					break;
				case crossbar_invalid_lane:
				default:
					fatal("switch_get_rx_packet() Invalid port name\n");
					break;
			}
			break;

		//Error checking
		case invalid_queue:
		default:
			fatal("switch_get_rx_packet() Invalid port name\n");
			break;
	}


	/*switch(queue)
	{
		case north_queue:
			in_queue = switches->north_queue;
			break;
		case east_queue:
			in_queue = switches->east_queue;
			break;
		case south_queue:
			in_queue = switches->south_queue;
			break;
		case west_queue:
			in_queue = switches->west_queue;
			break;
		case invalid_queue:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}*/

	/*if(queue == north_queue)
	{
		in_queue = switches->north_queue;
	}
	else if(queue == east_queue)
	{
		in_queue = switches->east_queue;
	}
	else if(queue == south_queue)
	{
		in_queue = switches->south_queue;
	}
	else if(queue == west_queue)
	{
		in_queue = switches->west_queue;
	}
	else
	{
		fatal("switche_get_out_queue() invalid port name\n");
	}*/

	assert(switch_queue != NULL);
	return switch_queue;
}

void remove_from_queue(struct switch_t *switches, struct cgm_packet_t *message_packet){


	switch(switches->queue)
	{
		case north_queue:
			list_remove(switches->north_queue, message_packet);
			break;
		case east_queue:
			list_remove(switches->east_queue, message_packet);
			break;
		case south_queue:
			list_remove(switches->south_queue, message_packet);
			break;
		case west_queue:
			list_remove(switches->west_queue, message_packet);
			break;
		case invalid_queue:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}


	/*if(switches->queue == north_queue)
	{
		list_remove(switches->north_queue, message_packet);
	}
	else if(switches->queue == east_queue)
	{
		list_remove(switches->east_queue, message_packet);
	}
	else if(switches->queue == south_queue)
	{
		list_remove(switches->south_queue, message_packet);
	}
	else if(switches->queue == west_queue)
	{
		list_remove(switches->west_queue, message_packet);
	}
	else
	{
		fatal("remove_from_queue() invalid port name\n");
	}*/

	CGM_DEBUG(switch_debug_file, "%s access_id %llu cycle %llu removed from %s with size %d\n",
			switches->name, message_packet->access_id, P_TIME, (char *)str_map_value(&port_name_map, switches->queue), list_count(switches->current_queue));

	return;
}

enum port_name get_next_lane_queue(enum port_name queue){

	enum port_name next_queue = invalid_queue;

	switch(queue)
	{
		case west_queue:
			next_queue = north_queue;
			break;
		case north_queue:
			next_queue = east_queue;
			break;
		case east_queue:
			next_queue = south_queue;
			break;
		case south_queue:
			next_queue = west_queue;
			break;
		case invalid_queue:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}

	assert(next_queue != invalid_queue);
	return next_queue;
}



enum port_name get_next_queue_rb(enum port_name queue){

	enum port_name next_queue = invalid_queue;

	switch(queue)
	{
		case west_queue:
			next_queue = north_queue;
			break;
		case north_queue:
			next_queue = east_queue;
			break;
		case east_queue:
			next_queue = south_queue;
			break;
		case south_queue:
			next_queue = west_queue;
			break;
		case invalid_queue:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}


	/*if(queue == west_queue)
	{
		next_queue = north_queue;
	}
	else if(queue == north_queue)
	{
		next_queue = east_queue;
	}
	else if(queue == east_queue)
	{
		next_queue = south_queue;
	}
	else if(queue == south_queue)
	{
		next_queue = west_queue;
	}
	else
	{
		fatal("get_next_queue() Invalid port name\n");
	}*/

	assert(next_queue != invalid_queue);
	return next_queue;
}

struct cgm_packet_t *switch_north_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane){

	struct cgm_packet_t *message_packet = NULL;

	switch(current_io_lane)
	{
		case io_request:
			message_packet = list_get(switches->north_tx_request_queue, 0);
			break;
		case io_reply:
			message_packet = list_get(switches->north_tx_reply_queue, 0);
			break;
		case io_coherence:
			message_packet = list_get(switches->north_tx_coherence_queue, 0);
			break;
		case io_invalid_lane:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}

	return message_packet;
}

struct cgm_packet_t *switch_east_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane){

	struct cgm_packet_t *message_packet = NULL;

	switch(current_io_lane)
	{
		case io_request:
			message_packet = list_get(switches->east_tx_request_queue, 0);
			break;
		case io_reply:
			message_packet = list_get(switches->east_tx_reply_queue, 0);
			break;
		case io_coherence:
			message_packet = list_get(switches->east_tx_coherence_queue, 0);
			break;
		case io_invalid_lane:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}

	return message_packet;
}

struct cgm_packet_t *switch_south_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane){

	struct cgm_packet_t *message_packet = NULL;

	switch(current_io_lane)
	{
		case io_request:
			message_packet = list_get(switches->south_tx_request_queue, 0);
			break;
		case io_reply:
			message_packet = list_get(switches->south_tx_reply_queue, 0);
			break;
		case io_coherence:
			message_packet = list_get(switches->south_tx_coherence_queue, 0);
			break;
		case io_invalid_lane:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}

	return message_packet;
}


struct cgm_packet_t *switch_west_io_ctrl_get_packet(struct switch_t *switches, enum switch_io_lane_map current_io_lane){

	struct cgm_packet_t *message_packet = NULL;

	switch(current_io_lane)
	{
		case io_request:
			message_packet = list_get(switches->west_tx_request_queue, 0);
			break;
		case io_reply:
			message_packet = list_get(switches->west_tx_reply_queue, 0);
			break;
		case io_coherence:
			message_packet = list_get(switches->west_tx_coherence_queue, 0);
			break;
		case io_invalid_lane:
		default:
			fatal("get_next_queue() Invalid port name\n");
			break;
	}

	return message_packet;
}


void switch_north_io_ctrl(void){

	int my_pid = switch_north_io_pid++;
	long long step = 1;
	int num_cores = x86_cpu_num_cores;
	struct cgm_packet_t *message_packet;
	int transfer_time = 0;

	enum switch_io_lane_map current_lane = io_request;

	long long occ_start = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_north_io_ec, step);

		/*stats*/
		occ_start = P_TIME;

		message_packet = switch_north_io_ctrl_get_packet(&switches[my_pid], current_lane);

		if(!message_packet) //try the next lane...
		{
			current_lane = get_next_io_lane_rb(current_lane);
			continue;
		}

		assert(message_packet);

		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;

		/*if(message_packet->evict_id == 203170)
			printf("block 0x%08x North IO routing access evict id %llu type %d cycle %llu\n",
				(message_packet->address & ~mem_ctrl->block_mask),
				message_packet->evict_id,
				message_packet->access_type,
				P_TIME);*/

		//try to send
		//L2 switches
		if(my_pid < num_cores)
		{

			switch (current_lane)
			{

				//funnel everything into the bottom request/reply queue
				case io_request:

					/*warning("switch_north_io_ctrl(): routing packing along request lane %s id %llu addr 0x%08x access type %d cycle %llu\n",
							switches[my_pid].name,
							message_packet->access_id,
							(message_packet->address & l2_caches[my_pid].block_address_mask),
							message_packet->access_type,
							P_TIME);*/

					if(list_count(l2_caches[my_pid].Rx_queue_bottom) >= QueueSize)
					{
						SYSTEM_PAUSE(1);
					}
					else
					{
						step++;

						SYSTEM_PAUSE(transfer_time);

						if(list_count(l2_caches[my_pid].Rx_queue_bottom) > QueueSize)
							warning("switch_north_io_ctrl(): %s %s size exceeded %d\n",
									l2_caches[my_pid].name, l2_caches[my_pid].Rx_queue_bottom->name, list_count(l2_caches[my_pid].Rx_queue_bottom));

						message_packet = list_remove(switches[my_pid].north_tx_request_queue, message_packet);
						list_enqueue(l2_caches[my_pid].Rx_queue_bottom, message_packet);
						advance(&l2_cache[my_pid]);

					}
					break;


				case io_reply:

					if(list_count(l2_caches[my_pid].Rx_queue_bottom) >= QueueSize)
					{
						SYSTEM_PAUSE(1);
					}
					else
					{
						step++;

						SYSTEM_PAUSE(transfer_time);

						if(list_count(l2_caches[my_pid].Rx_queue_bottom) > QueueSize)
							warning("switch_north_io_ctrl(): %s %s size exceeded %d\n",
									l2_caches[my_pid].name, l2_caches[my_pid].Rx_queue_bottom->name, list_count(l2_caches[my_pid].Rx_queue_bottom));

						message_packet = list_remove(switches[my_pid].north_tx_reply_queue, message_packet);
						list_enqueue(l2_caches[my_pid].Rx_queue_bottom, message_packet);
						advance(&l2_cache[my_pid]);

					}
					break;

				case io_coherence:

					if(list_count(l2_caches[my_pid].Coherance_Rx_queue) >= QueueSize)
					{
						SYSTEM_PAUSE(1);
					}
					else
					{
						step++;

						SYSTEM_PAUSE(transfer_time);

						if(list_count(l2_caches[my_pid].Coherance_Rx_queue) > QueueSize)
							warning("switch_north_io_ctrl(): %s %s size exceeded %d\n",
									l2_caches[my_pid].name, l2_caches[my_pid].Coherance_Rx_queue->name, list_count(l2_caches[my_pid].Coherance_Rx_queue));

						message_packet = list_remove(switches[my_pid].north_tx_coherence_queue, message_packet);
						list_enqueue(l2_caches[my_pid].Coherance_Rx_queue, message_packet);
						advance(&l2_cache[my_pid]);
					}
					break;

				case io_invalid_lane:
				default:
					fatal("switch_north_io_ctrl(): bad CPU L2 lane type %s id %llu addr 0x%08x access type %d cycle %llu\n",
							switches[my_pid].name,
							message_packet->access_id,
							(message_packet->address & l2_caches[my_pid].block_address_mask),
							message_packet->access_type,
							P_TIME);
					break;

				/*stats*/
				switches[my_pid].switch_north_io_transfers++;
				switches[my_pid].switch_north_io_transfer_cycles += transfer_time;
				switches[my_pid].switch_north_io_bytes_transfered += message_packet->size;
				//store_stat_bandwidth(bytes_rx, my_pid, transfer_time, switches[my_pid].bus_width);
			}
		}
		//hub-iommu
		else if(my_pid >= num_cores)
		{
			if(list_count(hub_iommu->Rx_queue_bottom) >= QueueSize)
			{
				SYSTEM_PAUSE(1);
			}
			else
			{
				step++;

				SYSTEM_PAUSE(transfer_time);

				if(list_count(hub_iommu->Rx_queue_bottom) > QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n",
							hub_iommu->name, hub_iommu->Rx_queue_bottom->name, list_count(hub_iommu->Rx_queue_bottom));

				if(current_lane == io_request)
				{
					message_packet = list_remove(switches[my_pid].north_tx_request_queue, message_packet);
					list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
					advance(hub_iommu_ec);
				}
				else if(current_lane == io_reply)
				{
					message_packet = list_remove(switches[my_pid].north_tx_reply_queue, message_packet);
					list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
					advance(hub_iommu_ec);
				}
				else if(current_lane == io_coherence)
				{
					message_packet = list_remove(switches[my_pid].north_tx_coherence_queue, message_packet);
					list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
					advance(hub_iommu_ec);
				}
				else
				{
					fatal("switch_north_io_ctrl(): bad GPU hub lane type\n");
				}

				/*stats*/
				switches[my_pid].switch_north_io_transfers++;
				switches[my_pid].switch_north_io_transfer_cycles += transfer_time;
				switches[my_pid].switch_north_io_bytes_transfered += message_packet->size;
				//store_stat_bandwidth(bytes_rx, my_pid, transfer_time, switches[my_pid].bus_width);
			}
		}
		else
		{
			fatal("switch_north_io_ctrl(): my_pid is out of bounds %d\n", my_pid);
		}


		current_lane = get_next_io_lane_rb(current_lane);

		/*stats occupancy*/
		switches[my_pid].switch_north_io_occupance += (P_TIME - occ_start);

	}

	fatal("switch_north_io_ctrl(): out of while loop\n");

	return;
}

void switch_east_io_ctrl(void){

	int my_pid = switch_east_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;
	//long long queue_depth = 0;

	enum switch_io_lane_map current_lane = io_request;

	long long occ_start = 0;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_east_io_ec, step);

		/*stats*/
		occ_start = P_TIME;

		//message_packet = list_get(switches[my_pid].Tx_east_queue, 0);
		//message_packet = list_get(switches[my_pid].east_tx_request_queue, 0);
		//assert(message_packet);

		message_packet = switch_east_io_ctrl_get_packet(&switches[my_pid], current_lane);

		if(!message_packet) //try the next lane...
		{
			current_lane = get_next_io_lane_rb(current_lane);
			continue;
		}

		assert(message_packet);

		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;

		/*if(message_packet->evict_id == 203170)
			printf("block 0x%08x East IO routing access evict id %llu type %d cycle %llu\n",
				(message_packet->address & ~mem_ctrl->block_mask),
				message_packet->evict_id,
				message_packet->access_type,
				P_TIME);*/


		switch(current_lane)
		{

			case io_request:

				if(list_count(switches[my_pid].next_east_rx_request_queue) >= QueueSize)
				{
					SYSTEM_PAUSE(1);
				}
				else
				{
					step++;

					SYSTEM_PAUSE(transfer_time);

					message_packet = list_remove(switches[my_pid].east_tx_request_queue, message_packet);
					list_enqueue(switches[my_pid].next_east_rx_request_queue, message_packet);
					advance(&switches_ec[switches[my_pid].next_east_id]);
				}
				break;

			case io_reply:

				if(list_count(switches[my_pid].next_east_rx_reply_queue) >= QueueSize)
				{
					SYSTEM_PAUSE(1);
				}
				else
				{
					step++;

					SYSTEM_PAUSE(transfer_time);

					message_packet = list_remove(switches[my_pid].east_tx_reply_queue, message_packet);
					list_enqueue(switches[my_pid].next_east_rx_reply_queue, message_packet);
					advance(&switches_ec[switches[my_pid].next_east_id]);
				}
				break;

			case io_coherence:

				if(list_count(switches[my_pid].next_east_rx_coherence_queue) >= QueueSize)
				{
					SYSTEM_PAUSE(1);
				}
				else
				{
					step++;

					SYSTEM_PAUSE(transfer_time);

					message_packet = list_remove(switches[my_pid].east_tx_coherence_queue, message_packet);
					list_enqueue(switches[my_pid].next_east_rx_coherence_queue, message_packet);
					advance(&switches_ec[switches[my_pid].next_east_id]);
				}
				break;

			case io_invalid_lane:
			default:
				fatal("switch_east_io_ctrl() Invalid lane\n");
				break;


		}


		/*if(current_lane == io_request)
		{
			if(list_count(switches[my_pid].next_east_rx_request_queue) >= QueueSize)
			{
				SYSTEM_PAUSE(1);
			}
			else
			{
				step++;

				SYSTEM_PAUSE(transfer_time);

				message_packet = list_remove(switches[my_pid].east_tx_request_queue, message_packet);
				list_enqueue(switches[my_pid].next_east_rx_request_queue, message_packet);
				advance(&switches_ec[switches[my_pid].next_east_id]);
			}
		}
		else if(current_lane == io_reply)
		{
			if(list_count(switches[my_pid].next_east_rx_reply_queue) >= QueueSize)
			{
				SYSTEM_PAUSE(1);
			}
			else
			{
				step++;

				SYSTEM_PAUSE(transfer_time);

				message_packet = list_remove(switches[my_pid].east_tx_reply_queue, message_packet);
				list_enqueue(switches[my_pid].next_east_rx_reply_queue, message_packet);
				advance(&switches_ec[switches[my_pid].next_east_id]);
			}

		}
		else if(current_lane == io_coherenece)
		{
			if(list_count(switches[my_pid].next_east_rx_reply_queue) >= QueueSize)
			{
				SYSTEM_PAUSE(1);
			}
			else
			{
				step++;

				SYSTEM_PAUSE(transfer_time);

				message_packet = list_remove(switches[my_pid].east_tx_reply_queue, message_packet);
				list_enqueue(switches[my_pid].next_east_rx_reply_queue, message_packet);
				advance(&switches_ec[switches[my_pid].next_east_id]);
			}

		}
		else
		{
			fatal("switch_north_io_ctrl(): bad lane type\n");
		}*/


		/*if(list_count(switches[my_pid].next_east) >= QueueSize)
		{
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			SYSTEM_PAUSE(transfer_time);

			if(list_count(switches[my_pid].next_east) > QueueSize)
				warning("switch_east_io_ctrl(): %s %s size exceeded %d\n", switches[my_pid].name, switches[my_pid].next_east->name, list_count(switches[my_pid].next_east));

			//drop into next east queue.
			message_packet = list_remove(switches[my_pid].east_tx_request_queue, message_packet);
			list_enqueue(switches[my_pid].next_east, message_packet);
			advance(&switches_ec[switches[my_pid].next_east_id]);

			stats
			switches[my_pid].switch_east_io_transfers++;
			switches[my_pid].switch_east_io_transfer_cycles += transfer_time;
			switches[my_pid].switch_east_io_bytes_transfered += message_packet->size;

			//note these stats are for the adjacent switch to the east which puts packets in the west rx_queue
			switches[switches[my_pid].next_east_id].west_rx_inserts++;
			queue_depth = list_count(switches[my_pid].next_east); //tricky
			max depth
			if(queue_depth > switches[switches[my_pid].next_east_id].west_rxqueue_max_depth)
				switches[switches[my_pid].next_east_id].west_rxqueue_max_depth = queue_depth;

			ave depth = ((old count * old data) + next data) / next count
			switches[switches[my_pid].next_east_id].west_rxqueue_ave_depth =
				((((double) switches[switches[my_pid].next_east_id].west_rx_inserts - 1) * switches[switches[my_pid].next_east_id].west_rxqueue_ave_depth)
						+ (double) queue_depth) / (double) switches[switches[my_pid].next_east_id].west_rx_inserts;
		}*/

		current_lane = get_next_io_lane_rb(current_lane);

		/*stats occupancy*/
		switches[my_pid].switch_east_io_occupance += (P_TIME - occ_start);

	}

	fatal("switch_east_io_ctrl(): out of while loop\n");

	return;
}

void switch_west_io_ctrl(void){

	int my_pid = switch_west_io_pid++;
	long long step = 1;

	/*int num_cores = x86_cpu_num_cores;
	int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;
	//long long queue_depth = 0;
	long long occ_start = 0;

	enum switch_io_lane_map current_lane = io_request;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_west_io_ec, step);

		/*stats*/
		occ_start = P_TIME;

		//message_packet = list_get(switches[my_pid].Tx_west_queue, 0);
		//message_packet = list_get(switches[my_pid].west_tx_request_queue, 0);

		message_packet = switch_west_io_ctrl_get_packet(&switches[my_pid], current_lane);

		if(!message_packet) //try the next lane...
		{
			current_lane = get_next_io_lane_rb(current_lane);
			continue;
		}


		if(!message_packet)
			printf("switch_west_io_ctrl(): %s io error cycle %llu\n", switches[my_pid].name, P_TIME);

		assert(message_packet);

		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;


		/*if(message_packet->evict_id == 203170)
			printf("block 0x%08x West IO routing access evict id %llu type %d cycle %llu\n",
				(message_packet->address & ~mem_ctrl->block_mask),
				message_packet->evict_id,
				message_packet->access_type,
				P_TIME);*/


		switch(current_lane)
		{

			case io_request:

				if(list_count(switches[my_pid].next_west_rx_request_queue) >= QueueSize)
				{
					SYSTEM_PAUSE(1);
				}
				else
				{
					step++;

					SYSTEM_PAUSE(transfer_time);

					message_packet = list_remove(switches[my_pid].west_tx_request_queue, message_packet);
					list_enqueue(switches[my_pid].next_west_rx_request_queue, message_packet);
					advance(&switches_ec[switches[my_pid].next_west_id]);
				}
				break;

			case io_reply:

				if(list_count(switches[my_pid].next_west_rx_reply_queue) >= QueueSize)
				{
					SYSTEM_PAUSE(1);
				}
				else
				{
					step++;

					SYSTEM_PAUSE(transfer_time);

					message_packet = list_remove(switches[my_pid].west_tx_reply_queue, message_packet);
					list_enqueue(switches[my_pid].next_west_rx_reply_queue, message_packet);
					advance(&switches_ec[switches[my_pid].next_west_id]);
				}
				break;

			case io_coherence:

				if(list_count(switches[my_pid].next_west_rx_coherence_queue) >= QueueSize)
				{
					SYSTEM_PAUSE(1);
				}
				else
				{
					step++;

					SYSTEM_PAUSE(transfer_time);

					message_packet = list_remove(switches[my_pid].west_tx_coherence_queue, message_packet);
					list_enqueue(switches[my_pid].next_west_rx_coherence_queue, message_packet);
					advance(&switches_ec[switches[my_pid].next_west_id]);
				}
				break;

			case io_invalid_lane:
			default:
				fatal("switch_east_io_ctrl() Invalid lane\n");
				break;

		}

		/*if(current_lane == io_request)
		{

			if(list_count(hub_iommu->Rx_queue_bottom) >= QueueSize)
			{
				SYSTEM_PAUSE(1);
			}
			else
			{
				step++;

				SYSTEM_PAUSE(transfer_time);

				if(list_count(hub_iommu->Rx_queue_bottom) > QueueSize)
					warning("switch_north_io_ctrl(): %s %s size exceeded %d\n",
							hub_iommu->name, hub_iommu->Rx_queue_bottom->name, list_count(hub_iommu->Rx_queue_bottom));
			}


		}
		else if(current_lane == io_reply)
		{
			message_packet = list_remove(switches[my_pid].north_tx_reply_queue, message_packet);
			list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
			advance(hub_iommu_ec);
		}
		else if(current_lane == io_coherence)
		{
			message_packet = list_remove(switches[my_pid].north_tx_coherence_queue, message_packet);
			list_enqueue(hub_iommu->Rx_queue_bottom, message_packet);
			advance(hub_iommu_ec);
		}
		else
		{
			fatal("switch_north_io_ctrl(): bad lane type\n");
		}*/






		/*if(list_count(switches[my_pid].next_west) >= QueueSize)
		{
			SYSTEM_PAUSE(1);
		}
		else
		{
			step++;

			SYSTEM_PAUSE(transfer_time);

			if(list_count(switches[my_pid].next_west) > QueueSize)
				warning("switch_west_io_ctrl(): %s %s size exceeded %d\n", switches[my_pid].name, switches[my_pid].next_west->name, list_count(switches[my_pid].next_west));

			//drop into next east queue.
			message_packet = list_remove(switches[my_pid].west_tx_request_queue, message_packet);
			list_enqueue(switches[my_pid].next_west, message_packet);
			advance(&switches_ec[switches[my_pid].next_west_id]);

			stats
			switches[my_pid].switch_west_io_transfers++;
			switches[my_pid].switch_west_io_transfer_cycles += transfer_time;
			switches[my_pid].switch_west_io_bytes_transfered += message_packet->size;

			//note these stats are for the adjacent switch
			switches[switches[my_pid].next_west_id].east_rx_inserts++;
			queue_depth = list_count(switches[my_pid].next_west); //tricky

			max depth
			if(queue_depth > switches[switches[my_pid].next_west_id].east_rxqueue_max_depth)
				switches[switches[my_pid].next_west_id].east_rxqueue_max_depth = queue_depth;

			ave depth = ((old count * old data) + next data) / next count
			switches[switches[my_pid].next_west_id].east_rxqueue_ave_depth =
				((((double) switches[switches[my_pid].next_west_id].east_rx_inserts - 1) * switches[switches[my_pid].next_west_id].east_rxqueue_ave_depth)
						+ (double) queue_depth) / (double) switches[switches[my_pid].next_west_id].east_rx_inserts;
		}*/

		current_lane = get_next_io_lane_rb(current_lane);

		/*stats occupancy*/
		switches[my_pid].switch_west_io_occupance += (P_TIME - occ_start);

	}

	fatal("switch_west_io_ctrl(): out of while loop\n");

	return;

}

void switch_check_access_type(struct cgm_packet_t *message_packet){

	if(!(message_packet->access_type == cgm_access_gets || message_packet->access_type == cgm_access_getx
		|| message_packet->access_type == cgm_access_get || message_packet->access_type == cgm_access_upgrade
		|| message_packet->access_type == cgm_access_mc_put || message_packet->access_type == cgm_access_downgrade_ack
		|| message_packet->access_type == cgm_access_getx_fwd_ack || message_packet->access_type == cgm_access_getx_fwd_nack
		|| message_packet->access_type == cgm_access_getx_fwd_upgrade_nack || message_packet->access_type == cgm_access_get_fwd_upgrade_nack
		|| message_packet->access_type == cgm_access_flush_block_ack || message_packet->access_type == cgm_access_write_back
		|| message_packet->access_type == cgm_access_upgrade_ack || message_packet->access_type == cgm_access_downgrade_nack
		|| message_packet->access_type == cgm_access_mc_load || message_packet->access_type == cgm_access_mc_store
		|| message_packet->access_type == cgm_access_mc_put)
	  )

	return;
}


void switch_south_io_ctrl(void){

	int my_pid = switch_south_io_pid++;
	long long step = 1;

	int num_cores = x86_cpu_num_cores;
	/*int num_cus = si_gpu_num_compute_units;*/

	struct cgm_packet_t *message_packet;
	/*long long access_id = 0;*/
	int transfer_time = 0;
	int queue_depth = 0;

	long long occ_start = 0;

	enum switch_io_lane_map current_lane = io_request;

	set_id((unsigned int)my_pid);

	while(1)
	{
		await(switches[my_pid].switches_south_io_ec, step);

		/*stats*/
		occ_start = P_TIME;

		/*find out what queue the packet needs to go into if the queue is full stall
		if not process and get ready for the next queue*/

		//message_packet = list_get(switches[my_pid].Tx_south_queue, 0);
		//message_packet = list_get(switches[my_pid].south_tx_request_queue, 0);
		//assert(message_packet);

		message_packet = switch_south_io_ctrl_get_packet(&switches[my_pid], current_lane);

		if(!message_packet) //try the next lane...
		{
			current_lane = get_next_io_lane_rb(current_lane);
			continue;
		}

		//get the transfer time
		transfer_time = (message_packet->size/switches[my_pid].bus_width);

		if(transfer_time == 0)
			transfer_time = 1;

		/*if(message_packet->evict_id == 203170)
			printf("block 0x%08x South IO routing access evict id %llu type %d cycle %llu\n",
				(message_packet->address & ~mem_ctrl->block_mask),
				message_packet->evict_id,
				message_packet->access_type,
				P_TIME);*/

		//try to send
		//L3 caches
		if(my_pid < num_cores)
		{

			switch(current_lane)
			{

				case io_request:

					if(list_count(l3_caches[my_pid].Rx_queue_top) >= QueueSize)
					{
						//queue is full so stall
						SYSTEM_PAUSE(1);
					}
					else
					{
						step++;

						SYSTEM_PAUSE(transfer_time);

						//should never happen
						if(list_count(l3_caches[my_pid].Rx_queue_top) > QueueSize)
						warning("switch_south_io_ctrl(): %s %s size exceeded %d\n", l3_caches[my_pid].name, l3_caches[my_pid].Rx_queue_top->name, list_count(l3_caches[my_pid].Rx_queue_top));

						message_packet = list_remove(switches[my_pid].south_tx_request_queue, message_packet);
						list_enqueue(l3_caches[my_pid].Rx_queue_top, message_packet);
						advance(&l3_cache[my_pid]);

						/*stats*/
						switches[my_pid].switch_south_io_transfers++;
						switches[my_pid].switch_south_io_transfer_cycles += transfer_time;
						switches[my_pid].switch_south_io_bytes_transfered += message_packet->size;
					}
					break;

				case io_reply:

					if(list_count(l3_caches[my_pid].Rx_queue_bottom) >= QueueSize)
					{
						//queue is full so stall
						SYSTEM_PAUSE(1);
					}
					else
					{
						step++;

						SYSTEM_PAUSE(transfer_time);

						//should never happen
						if(list_count(l3_caches[my_pid].Rx_queue_bottom) > QueueSize)
						warning("switch_south_io_ctrl(): %s %s size exceeded %d\n",
								l3_caches[my_pid].name, l3_caches[my_pid].Rx_queue_bottom->name, list_count(l3_caches[my_pid].Rx_queue_bottom));

						message_packet = list_remove(switches[my_pid].south_tx_reply_queue, message_packet);
						list_enqueue(l3_caches[my_pid].Rx_queue_bottom, message_packet);
						advance(&l3_cache[my_pid]);

						/*stats*/
						switches[my_pid].switch_south_io_transfers++;
						switches[my_pid].switch_south_io_transfer_cycles += transfer_time;
						switches[my_pid].switch_south_io_bytes_transfered += message_packet->size;
					}
					break;

				case io_coherence:

					if(list_count(l3_caches[my_pid].Coherance_Rx_queue) >= QueueSize)
					{
						//queue is full so stall
						SYSTEM_PAUSE(1);
					}
					else
					{
						step++;

						SYSTEM_PAUSE(transfer_time);

						//should never happen
						if(list_count(l3_caches[my_pid].Coherance_Rx_queue) > QueueSize)
						warning("switch_south_io_ctrl(): %s %s size exceeded %d\n",
								l3_caches[my_pid].name, l3_caches[my_pid].Coherance_Rx_queue->name, list_count(l3_caches[my_pid].Coherance_Rx_queue));

						message_packet = list_remove(switches[my_pid].south_tx_coherence_queue, message_packet);
						list_enqueue(l3_caches[my_pid].Coherance_Rx_queue, message_packet);
						advance(&l3_cache[my_pid]);

						/*stats*/
						switches[my_pid].switch_south_io_transfers++;
						switches[my_pid].switch_south_io_transfer_cycles += transfer_time;
						switches[my_pid].switch_south_io_bytes_transfered += message_packet->size;
					}
					break;

				case io_invalid_lane:
				default:
					fatal("switch_south_io_ctrl() Invalid lane\n");
					break;
			}

		}
		//Sys Agent
		else if(my_pid >= num_cores)
		{

			if(list_count(system_agent->Rx_queue_top) >= QueueSize)
			{
				SYSTEM_PAUSE(1);
			}
			else
			{
				step++;

				SYSTEM_PAUSE(transfer_time);

				if(list_count(system_agent->Rx_queue_top) > QueueSize)
					warning("switch_south_io_ctrl(): %s %s size exceeded %d\n", system_agent->name, system_agent->Rx_queue_top->name, list_count(system_agent->Rx_queue_top));

				if(current_lane == io_request)
				{
					message_packet = list_remove(switches[my_pid].south_tx_request_queue, message_packet);
					list_enqueue(system_agent->Rx_queue_top, message_packet);
					advance(system_agent_ec);
				}
				else if(current_lane == io_reply)
				{
					message_packet = list_remove(switches[my_pid].south_tx_reply_queue, message_packet);
					list_enqueue(system_agent->Rx_queue_top, message_packet);
					advance(system_agent_ec);
				}
				else if(current_lane == io_coherence)
				{
					message_packet = list_remove(switches[my_pid].south_tx_coherence_queue, message_packet);
					list_enqueue(system_agent->Rx_queue_top, message_packet);
					advance(system_agent_ec);
				}
				else
				{
					fatal("switch_north_io_ctrl(): bad lane type\n");
				}

				//message_packet = list_remove(switches[my_pid].south_tx_request_queue, message_packet);
				//list_enqueue(system_agent->Rx_queue_top, message_packet);
				//advance(system_agent_ec);

				/*stats*/
				if(system_agent->max_north_rxqueue_depth < list_count(system_agent->Rx_queue_top))
					system_agent->max_north_rxqueue_depth = list_count(system_agent->Rx_queue_top);

				system_agent->north_gets++;
				queue_depth = list_count(system_agent->Rx_queue_top);
				system_agent->ave_north_rxqueue_depth = ((((double) system_agent->north_gets - 1) * system_agent->ave_north_rxqueue_depth) + (double) queue_depth) / (double) system_agent->north_gets;

				switches[my_pid].switch_south_io_transfers++;
				switches[my_pid].switch_south_io_transfer_cycles += transfer_time;
				switches[my_pid].switch_south_io_bytes_transfered += message_packet->size;
			}
		}
		else
		{
			fatal("switch_south_io_ctrl(): my_pid is out of bounds %d\n", my_pid);
		}

		current_lane = get_next_io_lane_rb(current_lane);

		/*stats occupancy*/
		switches[my_pid].switch_south_io_occupance += (P_TIME - occ_start);

	}

	fatal("switch_south_io_ctrl(): out of while loop\n");

	return;
}

void switch_dump_queue(struct list_t *queue){

	int i = 0;
	struct cgm_packet_t *packet = NULL;

	LIST_FOR_EACH(queue, i)
	{
		//get pointer to access in queue and check it's status.
		packet = list_get(queue, i);
		printf("\t %s slot %d packet id %llu access type %s addr 0x%08x blk addr 0x%08x dest_name %s start_cycle %llu\n",
				queue->name, i, packet->access_id, str_map_value(&cgm_mem_access_strn_map, packet->access_type), packet->address,
				(packet->address & l3_caches[0].block_address_mask), packet->dest_name, packet->start_cycle);
	}

	return;
}


void switch_store_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	//switch stats
	for(i = 0; i < (num_cores + 1); i++)
	{
		cgm_stat_container->switch_total_links[i] = switches[i].switch_total_links;
		cgm_stat_container->switch_max_links[i] = switches[i].switch_max_links;
		cgm_stat_container->switch_occupance[i] = switches[i].switch_occupance;
		cgm_stat_container->switch_north_io_transfers[i] = switches[i].switch_north_io_transfers;
		cgm_stat_container->switch_north_io_transfer_cycles[i] = switches[i].switch_north_io_transfer_cycles;
		cgm_stat_container->switch_north_io_bytes_transfered[i] = switches[i].switch_north_io_bytes_transfered;
		cgm_stat_container->switch_east_io_transfers[i] = switches[i].switch_east_io_transfers;
		cgm_stat_container->switch_east_io_transfer_cycles[i] = switches[i].switch_east_io_transfer_cycles;
		cgm_stat_container->switch_east_io_bytes_transfered[i] = switches[i].switch_east_io_bytes_transfered;
		cgm_stat_container->switch_south_io_transfers[i] = switches[i].switch_south_io_transfers;
		cgm_stat_container->switch_south_io_transfer_cycles[i] = switches[i].switch_south_io_transfer_cycles;
		cgm_stat_container->switch_south_io_bytes_transfered[i] = switches[i].switch_south_io_bytes_transfered;
		cgm_stat_container->switch_west_io_transfers[i] = switches[i].switch_west_io_transfers;
		cgm_stat_container->switch_west_io_transfer_cycles[i] = switches[i].switch_west_io_transfer_cycles;
		cgm_stat_container->switch_west_io_bytes_transfered[i] = switches[i].switch_west_io_bytes_transfered;
		cgm_stat_container->switch_north_txqueue_max_depth[i] = switches[i].north_txqueue_max_depth;
		cgm_stat_container->switch_north_txqueue_ave_depth[i] = switches[i].north_txqueue_ave_depth;
		cgm_stat_container->switch_east_txqueue_max_depth[i] = switches[i].east_txqueue_max_depth;
		cgm_stat_container->switch_east_txqueue_ave_depth[i] = switches[i].east_txqueue_ave_depth;
		cgm_stat_container->switch_south_txqueue_max_depth[i] = switches[i].south_txqueue_max_depth;
		cgm_stat_container->switch_south_txqueue_ave_depth[i] = switches[i].south_txqueue_ave_depth;
		cgm_stat_container->switch_west_txqueue_max_depth[i] = switches[i].west_txqueue_max_depth;
		cgm_stat_container->switch_west_txqueue_ave_depth[i] = switches[i].west_txqueue_ave_depth;

		cgm_stat_container->switch_north_tx_inserts[i] = switches[i].north_tx_inserts;
		cgm_stat_container->switch_east_tx_inserts[i] = switches[i].east_tx_inserts;
		cgm_stat_container->switch_south_tx_inserts[i] = switches[i].south_tx_inserts;
		cgm_stat_container->switch_west_tx_inserts[i] = switches[i].west_tx_inserts;

		cgm_stat_container->switch_north_rxqueue_max_depth[i] = switches[i].north_rxqueue_max_depth;
		cgm_stat_container->switch_north_rxqueue_ave_depth[i] = switches[i].north_rxqueue_ave_depth;
		cgm_stat_container->switch_east_rxqueue_max_depth[i] = switches[i].east_rxqueue_max_depth;
		cgm_stat_container->switch_east_rxqueue_ave_depth[i] = switches[i].east_rxqueue_ave_depth;

		cgm_stat_container->switch_south_rxqueue_max_depth[i] = switches[i].south_rxqueue_max_depth;
		cgm_stat_container->switch_south_rxqueue_ave_depth[i] = switches[i].south_rxqueue_ave_depth;
		cgm_stat_container->switch_west_rxqueue_max_depth[i] = switches[i].west_rxqueue_max_depth;
		cgm_stat_container->switch_west_rxqueue_ave_depth[i] = switches[i].west_rxqueue_ave_depth;

		cgm_stat_container->switch_north_rx_inserts[i] = switches[i].north_rx_inserts;
		cgm_stat_container->switch_east_rx_inserts[i] = switches[i].east_rx_inserts;
		cgm_stat_container->switch_south_rx_inserts[i] = switches[i].south_rx_inserts;
		cgm_stat_container->switch_west_rx_inserts[i] = switches[i].west_rx_inserts;

		//IO Ctrl
		cgm_stat_container->switch_north_io_occupance[i] = switches[i].switch_north_io_occupance;
		cgm_stat_container->switch_east_io_occupance[i] = switches[i].switch_east_io_occupance;
		cgm_stat_container->switch_south_io_occupance[i] = switches[i].switch_south_io_occupance;
		cgm_stat_container->switch_west_io_occupance[i] = switches[i].switch_west_io_occupance;
	}

	return;
}

void switch_reset_stats(void){

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	//switch stats
	for(i = 0; i < (num_cores + 1); i++)
	{
		switches[i].switch_occupance = 0;


		switches[i].switch_total_links = 0;
		switches[i].switch_max_links = 0;
		switches[i].switch_north_io_transfers = 0;
		switches[i].switch_north_io_transfer_cycles = 0;
		switches[i].switch_north_io_bytes_transfered = 0;
		switches[i].switch_east_io_transfers = 0;
		switches[i].switch_east_io_transfer_cycles = 0;
		switches[i].switch_east_io_bytes_transfered = 0;
		switches[i].switch_south_io_transfers = 0;
		switches[i].switch_south_io_transfer_cycles = 0;
		switches[i].switch_south_io_bytes_transfered = 0;
		switches[i].switch_west_io_transfers = 0;
		switches[i].switch_west_io_transfer_cycles = 0;
		switches[i].switch_west_io_bytes_transfered = 0;
		switches[i].north_txqueue_max_depth = 0;
		switches[i].north_txqueue_ave_depth = 0;
		switches[i].east_txqueue_max_depth = 0;
		switches[i].east_txqueue_ave_depth = 0;
		switches[i].south_txqueue_max_depth = 0;
		switches[i].south_txqueue_ave_depth = 0;
		switches[i].west_txqueue_max_depth = 0;
		switches[i].west_txqueue_ave_depth = 0;

		switches[i].north_tx_inserts = 0;
		switches[i].east_tx_inserts = 0;
		switches[i].south_tx_inserts = 0;
		switches[i].west_tx_inserts = 0;

		switches[i].north_rxqueue_max_depth = 0;
		switches[i].north_rxqueue_ave_depth = 0;
		switches[i].east_rxqueue_max_depth = 0;
		switches[i].east_rxqueue_ave_depth = 0;
		switches[i].south_rxqueue_max_depth = 0;
		switches[i].south_rxqueue_ave_depth = 0;
		switches[i].west_rxqueue_max_depth = 0;
		switches[i].west_rxqueue_ave_depth = 0;

		switches[i].north_rx_inserts = 0;
		switches[i].east_rx_inserts = 0;
		switches[i].south_rx_inserts = 0;
		switches[i].west_rx_inserts = 0;

		//IO Ctrl
		switches[i].switch_north_io_occupance = 0;
		switches[i].switch_east_io_occupance = 0;
		switches[i].switch_south_io_occupance = 0;
		switches[i].switch_west_io_occupance = 0;

	}


	return;
}


void switch_dump_stats(struct cgm_stats_t *cgm_stat_container){

	int num_cores = x86_cpu_num_cores;
	int i = 0;

	/*there is a switch for the GPU this for loop will pick it up*/
	for(i = 0; i <= num_cores; i++)
	{

		CGM_STATS(cgm_stats_file, "s_%d_Occupancy = %llu\n", i, cgm_stat_container->switch_occupance[i]);
		CGM_STATS(cgm_stats_file, "s_%d_IONorthOccupancy = %llu\n", i, cgm_stat_container->switch_north_io_occupance[i]);
		CGM_STATS(cgm_stats_file, "s_%d_IOEastOccupancy = %llu\n", i, cgm_stat_container->switch_east_io_occupance[i]);
		CGM_STATS(cgm_stats_file, "s_%d_IOSouthOccupancy = %llu\n", i, cgm_stat_container->switch_south_io_occupance[i]);
		CGM_STATS(cgm_stats_file, "s_%d_IOWestOccupancy = %llu\n", i, cgm_stat_container->switch_west_io_occupance[i]);
		if(cgm_stat_container->stats_type == systemStats)
		{
			CGM_STATS(cgm_stats_file, "s_%d_OccupancyPct = %0.6f\n", i, ((double) cgm_stat_container->switch_occupance[i]/(double) P_TIME));
		}
		else if (cgm_stat_container->stats_type == parallelSection)
		{
			CGM_STATS(cgm_stats_file, "s_%d_OccupancyPct = %0.6f\n", i, (((double) cgm_stat_container->switch_occupance[i])/((double) cgm_stat_container->total_parallel_section_cycles)));

			CGM_STATS(cgm_stats_file, "s_%d_IONorthOccupancyPct = %0.6f\n"
					, i, (((double) cgm_stat_container->switch_north_io_occupance[i])/((double) cgm_stat_container->total_parallel_section_cycles)));
			CGM_STATS(cgm_stats_file, "s_%d_IOEastOccupancyPct = %0.6f\n"
					, i, (((double) cgm_stat_container->switch_east_io_occupance[i])/((double) cgm_stat_container->total_parallel_section_cycles)));
			CGM_STATS(cgm_stats_file, "s_%d_IOSouthOccupancyPct = %0.6f\n"
					, i, (((double) cgm_stat_container->switch_south_io_occupance[i])/((double) cgm_stat_container->total_parallel_section_cycles)));
			CGM_STATS(cgm_stats_file, "s_%d_IOWestOccupancyPct = %0.6f\n"
					, i, (((double) cgm_stat_container->switch_west_io_occupance[i])/((double) cgm_stat_container->total_parallel_section_cycles)));
		}
		else
		{
			fatal("cache_dump_stats(): bad container type\n");
		}


		//------------------
		/*CGM_STATS(cgm_stats_file, "[Switch_%d]\n", i);*/
		//CGM_STATS(cgm_stats_file, "s_%d_TotalSwitchCtrlLoops = %llu\n", i, cgm_stat_container->switch_total_wakes[i]);

		CGM_STATS(cgm_stats_file, "s_%d_NumberLinks = %llu\n", i, cgm_stat_container->switch_total_links[i]);
		CGM_STATS(cgm_stats_file, "s_%d_MaxNumberLinks = %d\n", i, cgm_stat_container->switch_max_links[i]);
		//CGM_STATS(cgm_stats_file, "s_%d_AveNumberLinksPerAccess = %.02f\n", i, (double)cgm_stat_container->switch_total_links[i]/(double)cgm_stat_container->switch_total_wakes[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthIOTransfers = %llu\n", i, cgm_stat_container->switch_north_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthIOCycles = %llu\n", i, cgm_stat_container->switch_north_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_north_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_north_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_north_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_north_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_NorthTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_north_txqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastIOTransfers = %llu\n", i, cgm_stat_container->switch_east_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastIOCycles = %llu\n", i, cgm_stat_container->switch_east_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_east_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_east_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_east_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_east_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_EastTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_east_txqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthIOTransfers = %llu\n", i, cgm_stat_container->switch_south_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthIOCycles = %llu\n", i, cgm_stat_container->switch_south_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_south_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_south_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_south_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_south_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_SouthTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_south_txqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestIOTransfers = %llu\n", i, cgm_stat_container->switch_west_io_transfers[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestIOCycles = %llu\n", i, cgm_stat_container->switch_west_io_transfer_cycles[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestIOBytesTransfered = %llu\n", i, cgm_stat_container->switch_west_io_bytes_transfered[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestRxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_west_rxqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestRxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_west_rxqueue_ave_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestTxQueueMaxDepth = %llu\n", i, cgm_stat_container->switch_west_txqueue_max_depth[i]);
		CGM_STATS(cgm_stats_file, "s_%d_WestTxQueueAveDepth = %0.2f\n", i, cgm_stat_container->switch_west_txqueue_ave_depth[i]);
		/*CGM_STATS(cgm_stats_file, "\n");*/
	}

	/*CGM_STATS(cgm_stats_file, "[Switch_SA]\n");
	CGM_STATS(cgm_stats_file, "NumberSwitchCtrlLoops = %llu\n", cgm_stat_container->switch_total_wakes[num_cores]);
	CGM_STATS(cgm_stats_file, "SwitchOccupance = %0.2f\n", (double) cgm_stat_container->switch_total_wakes[num_cores]/ (double) P_TIME);
	CGM_STATS(cgm_stats_file, "NumberLinks = %llu\n", cgm_stat_container->switch_total_links[num_cores]);
	CGM_STATS(cgm_stats_file, "MaxNumberLinks = %d\n", cgm_stat_container->switch_max_links[num_cores]);
	CGM_STATS(cgm_stats_file, "AveNumberLinksPerCtrlLoop = %.02f\n", (double)cgm_stat_container->switch_total_links[num_cores]/(double)cgm_stat_container->switch_total_wakes[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthIOTransfers = %llu\n", cgm_stat_container->switch_north_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthIOCycles = %llu\n", cgm_stat_container->switch_north_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthIOBytesTransfered = %llu\n", cgm_stat_container->switch_north_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_north_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_north_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_north_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "NorthTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_north_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastIOTransfers = %llu\n", cgm_stat_container->switch_east_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "EastIOCycles = %llu\n", cgm_stat_container->switch_east_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "EastIOBytesTransfered = %llu\n", cgm_stat_container->switch_east_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "EastRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_east_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_east_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_east_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "EastTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_east_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthIOTransfers = %llu\n", cgm_stat_container->switch_south_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthIOCycles = %llu\n", cgm_stat_container->switch_south_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthIOBytesTransfered = %llu\n", cgm_stat_container->switch_south_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_south_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_south_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_south_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "SouthTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_south_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestIOTransfers = %llu\n", cgm_stat_container->switch_west_io_transfers[num_cores]);
	CGM_STATS(cgm_stats_file, "WestIOCycles = %llu\n", cgm_stat_container->switch_west_io_transfer_cycles[num_cores]);
	CGM_STATS(cgm_stats_file, "WestIOBytesTransfered = %llu\n", cgm_stat_container->switch_west_io_bytes_transfered[num_cores]);
	CGM_STATS(cgm_stats_file, "WestRxQueueMaxDepth = %llu\n", cgm_stat_container->switch_west_rxqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestRxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_west_rxqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestTxQueueMaxDepth = %llu\n", cgm_stat_container->switch_west_txqueue_max_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "WestTxQueueAveDepth = %0.2f\n", cgm_stat_container->switch_west_txqueue_ave_depth[num_cores]);
	CGM_STATS(cgm_stats_file, "\n");*/

	return;
}
