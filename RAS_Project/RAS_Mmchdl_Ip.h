/* RAS_Mmchdl_Ip.h */
#ifndef _RAS_MMCHDL_IP_H_
#define _RAS_MMCHDL_IP_H_

int MMCHDL_IP_Init( oammmc_t *ptOammmc );

int MMCHDL_IP_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_IP_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_IP_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_IP_H_ */
