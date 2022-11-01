/* RAS_Mmchdl_CliIp.h */
#ifndef _RAS_MMCHDL_CLIIP_H_
#define _RAS_MMCHDL_CLIIP_H_

#define ARG_ID_CLI_IP	63007
#define ARG_ID_DESC		63008

int MMCHDL_CLIIP_Init( oammmc_t *ptOammmc );

int MMCHDL_CLIIP_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_CLIIP_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_CLIIP_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_CLIIP_H_ */
