/* RAS_Oammmc.h */
#ifndef _RAS_OAMMMC_H_
#define _RAS_OAMMMC_H_

#define CLI_IP				"CLI_IP"
#define USR_ID				"USR_ID"

#define ARG_ID_CLI_IP		63007
#define ARG_ID_DESC			63008
#define	ARG_ID_PERIOD_TM	63006 
#define ARG_ID_USR_ID		63009
#define ARG_ID_USR_NAME		63010
#define ARG_ID_USR_GENDER	63011
#define ARG_ID_USR_BIRTH	63012
#define ARG_ID_MALE			63013
#define ARG_ID_FEMALE		63014

oammmc_arg_info_t atArgIp[] =
{
	{ 1, CLI_IP, NULL, OAMMMC_STR, ARG_ID_CLI_IP, 7, 39, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL }
};

oammmc_arg_info_t atArgIpDesc[] =
{
	{ 1, CLI_IP, NULL, OAMMMC_STR, ARG_ID_CLI_IP, 7, 39, NULL, NULL },
	{ 2, "DESC", NULL, OAMMMC_STR, ARG_ID_DESC, 1, 32, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL }
};

oammmc_arg_info_t atArgIpPeriod[] =
{
	{ 1, CLI_IP, NULL, OAMMMC_STR, ARG_ID_CLI_IP, 7, 39, NULL, NULL },
	{ 2, "PERIOD_TM", NULL, OAMMMC_STR, ARG_ID_PERIOD_TM, 1, 10800, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL }
};

oamenumx_t atEnumGender[] =
{
	{ "MALE", ARG_ID_MALE, MSG_ID_DIS_USR_INFO },
	{ "FEMALE", ARG_ID_FEMALE, MSG_ID_DIS_USR_INFO },
	{ 0 }
};

oammmc_arg_info_t atArgInfo[] =
{
	{ 1, USR_ID, NULL, OAMMMC_INT, ARG_ID_USR_ID, 1, INT_MAX, NULL, NULL },
	{ 2, "USR_NAME", NULL, OAMMMC_STR, ARG_ID_USR_NAME, 1, 32, NULL, NULL },
	{ 3, "USR_GENDER", NULL, OAMMMC_ENUM, ARG_ID_USR_GENDER, 0, 0, atEnumGender, NULL },
	{ 4, "USR_BIRTH", NULL, OAMMMC_STR, ARG_ID_USR_BIRTH, 1, 6, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL }
};

oammmc_arg_info_t atArgId[] =
{
	{ 1, USR_ID, NULL, OAMMMC_INT, ARG_ID_USR_ID, 1, INT_MAX, NULL, NULL },
	{ 0, NULL, NULL, 0, 0, 0, 0, NULL }
};

oammmc_cmd_t atCommand[] =
{
	{ 1, "add-cli-ip", MSG_ID_ADD_CLI_IP, MMCHDLR_AddCliIp, 1, 2, atArgIpDesc, "CMD_DESC: ADD CLIENT IP" },
	{ 2, "dis-cli-ip", MSG_ID_DIS_CLI_IP, MMCHDLR_DisCliIp, 0, 1, atArgIp, "CMD_DESC: DISPLAY CLIENT IP" },
	{ 3, "del-cli-ip", MSG_ID_DEL_CLI_IP, MMCHDLR_DelCliIp, 1, 1, atArgIp, "CMD_DESC: DELETE CLIENT IP" },
	{ 4, "add-cli-ip-trc", MSG_ID_ADD_CLI_IP_TRC, MMCHDLR_AddCliIpTrc, 1, 2, atArgIpPeriod, "CMD_DESC: ADD CLIENT IP TRACE" },
	{ 5, "dis-cli-ip-trc", MSG_ID_DIS_CLI_IP_TRC, MMCHDLR_DisCliIpTrc, 0, 1, atArgIp, "CMD_DESC: DISPLAY CLIENT IP TRACE" },
	{ 6, "del-cli-ip-trc", MSG_ID_DEL_CLI_IP_TRC, MMCHDLR_DelCliIpTrc, 1, 1, atArgIp, "CMD_DESC: DELETE CLIENT IP TRACE" },
	{ 7, "dis-usr-info", MSG_ID_DIS_USR_INFO, MMCHDLR_DisUsrInfo, 0, 4, atArgInfo, "CMD_DESC: DISPLAY USER INFO" },
	{ 8, "del-usr-info", MSG_ID_DEL_USR_INFO, MMCHDLR_DelUsrInfo, 1, 1, atArgId, "CMD_DESC: DELETE USER INFO" },
	{ 0, NULL, 0, 0, 0, 0, NULL, NULL }
};

int OAMMMC_Init( mpipc_t *ptMpipc, oammmc_t *ptOammmc );

#endif /* _RAS_OAMMMC_H_ */
