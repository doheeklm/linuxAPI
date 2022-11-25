/* RAS_Mmchdl_Trc.h */
#ifndef _RAS_MMCHDL_TRC_H_
#define _RAS_MMCHDL_TRC_H_

int MMCHDL_TRC_Init( oammmc_t *ptOammmc );
int MMCHDL_TRC_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int MMCHDL_TRC_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int MMCHDL_TRC_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_TRC_H_ */
