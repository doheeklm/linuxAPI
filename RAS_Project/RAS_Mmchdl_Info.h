/* RAS_Mmchdl_Info.h */
#ifndef _RAS_MMCHDL_INFO_H_
#define _RAS_MMCHDL_INFO_H_

int MMCHDL_INFO_Init( oammmc_t *ptOammmc );
int MMCHDL_INFO_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int MMCHDL_INFO_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );
int MMCHDL_INFO_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_INFO_H_ */
