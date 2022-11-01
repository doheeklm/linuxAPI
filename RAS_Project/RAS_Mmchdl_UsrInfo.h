/* RAS_Mmchdl_UsrInfo.h */
#ifndef _RAS_MMCHDL_USRINFO_H_
#define _RAS_MMCHDL_USRINFO_H_

#define ARG_ID_USR_ID		63009
#define ARG_ID_USR_NAME		63010
#define ARG_ID_USR_GENDER	63011
#define ARG_ID_USR_BIRTH	63012
#define ARG_ID_MALE			63013
#define ARG_ID_FEMALE		63014

int MMCHDL_USRINFO_Init( oammmc_t *ptOammmc );

int MMCHDL_USRINFO_Add( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_USRINFO_Dis( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

int MMCHDL_USRINFO_Del( oammmc_t *ptOammmc, oammmc_cmd_t *ptCmd,
		oammmc_arg_t *patArgList, int nArgNum, void *ptUarg );

#endif /* _RAS_MMCHDL_USRINFO_H_ */
