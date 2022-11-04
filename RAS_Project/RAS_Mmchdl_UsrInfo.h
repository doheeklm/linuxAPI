/* RAS_Mmchdl_UsrInfo.h */
#ifndef _RAS_MMCHDL_USRINFO_H_
#define _RAS_MMCHDL_USRINFO_H_

#define ARG_USR_ID_ID		63009
#define ARG_USR_ID_DESC		"USR_ID"
#define ARG_USR_NAME_ID		63010
#define ARG_USR_NAME_DESC	"USR_NAME"
#define ARG_USR_GENDER_ID	63011
#define ARG_USR_GENDER_DESC	"USR_GENDER"
#define ARG_USR_BIRTH_ID	63012
#define ARG_USR_BIRTH_DESC	"USR_BIRTH"
#define ARG_MALE_ID			63013
#define ARG_MALE_DESC		"MALE"
#define ARG_FEMALE_ID		63014
#define ARG_FEMALE_DESC		"FEMALE"

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
