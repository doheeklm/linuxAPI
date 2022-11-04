/* RAS_Mmchdl_CliIp.h */
#ifndef _RAS_MMCHDL_CLIIP_H_
#define _RAS_MMCHDL_CLIIP_H_

#define ARG_CLI_IP_ID			63007
#define ARG_CLI_IP_DESC			"CLI_IP"
#define ARG_CLI_IP_DESC_ID		63008
#define ARG_CLI_IP_DESC_DESC	"CLI_IP_DESC"

int MMCHDL_CLIIP_Init( oammmc_t *ptOammmc );

int MMCHDL_CLIIP_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_CLIIP_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_CLIIP_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_CLIIP_H_ */
