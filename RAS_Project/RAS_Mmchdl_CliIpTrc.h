/* RAS_Mmchdl_CliIpTrc.h */
#ifndef _RAS_MMCHDL_CLIIPTRC_H_
#define _RAS_MMCHDL_CLIIPTRC_H_

#define ARG_ID_CLI_IP		63007
#define	ARG_ID_PERIOD_TM	63006 

int MMCHDL_CLIIPTRC_Init( oammmc_t *ptOammmc );

int MMCHDL_CLIIPTRC_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_CLIIPTRC_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_CLIIPTRC_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_CLIIPTRC_H_ */
