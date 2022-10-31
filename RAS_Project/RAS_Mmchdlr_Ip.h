/* RAS_Mmchdlr_Ip.h */
#ifndef _RAS_MMCHDLR_IP_H_
#define _RAS_MMCHDLR_IP_H_

int MMCHDLR_AddCliIp( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDLR_DisCliIp( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDLR_DelCliIp( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDLR_IP_H_ */
