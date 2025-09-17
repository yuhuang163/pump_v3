#if defined(UTILS_ESHELL_EN) && defined(IMS_SUPPORT)
#include "eshell.h"
#include "../../../net/lwip_2.1.2/src/include/lwip/tcpip.h"
#include "hal_sysfreq.h"

extern int tmtc_voip_init(void);
extern int tmtc_voip_reg(void);
extern int tmtc_voip_call(char *pcUri);
extern int tmtc_volte_init(void);
extern int tmtc_volte_reg(void);
extern int tmtc_volte_answer(void);
extern int tmtc_volte_term(void);
extern int tmtc_volte_setsmsreport(char * bDeliReportEnable);
extern int tmtc_volte_sendmsg(char *pcPhone, char *pcMsg);
extern int tmtc_volte_senddtmf(char * pcDtmfType);
extern int tmtc_volte_unreg(void);
extern int tmtc_volte_hold(char *pcCallId);
extern int tmtc_volte_unhold(char *pcCallId);
extern void debug_task(void);

// #include "zos_task.h"
#include "cmsis_os2.h"
extern uint32_t OsGetAllTskInfo(void);
static void do_debugtask(int argc, char *argv[])
{
    OsGetAllTskInfo();
}

static void do_voip_init(int argc, char *argv[])
{
    tmtc_voip_init();
}

static void do_voip_reg(int argc, char *argv[])
{
    tmtc_voip_reg();
}

static void do_voip_call(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! Usage:call [phone number]", __func__);
        return ;
    }

    tmtc_voip_call(argv[1]);
}

static void do_volte_init(int argc, char *argv[])
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_104M);
    tcpip_init(NULL,NULL);
    tmtc_volte_init();
}

static void do_volte_reg(int argc, char *argv[])
{
    tmtc_volte_reg();
}

static void do_volte_answer(int argc, char *argv[])
{
    tmtc_volte_answer();
}

static void do_volte_term(int argc, char *argv[])
{
    tmtc_volte_term();
}

static void do_volte_setsmsreport(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! Usage:setsmsreport [0 or 1]", __func__);
        return ;
    }

    tmtc_volte_setsmsreport(argv[2]);
}

static void do_volte_sendmsg(int argc, char *argv[])
{
    if (argc < 3) {
        eshell_putstring("%s param err! Usage:sendmsg [phone number] [content]", __func__);
        return ;
    }

    tmtc_volte_sendmsg(argv[2], argv[3]);
}

static void do_volte_senddtmf(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! Usage:dtmf [number]", __func__);
        return ;
    }

    tmtc_volte_senddtmf(argv[2]);
}

static void do_volte_hold(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! Usage:hold [1 or 2]", __func__);
        return ;
    }
    tmtc_volte_hold(argv[2]);
}

static void do_volte_unhold(int argc, char *argv[])
{
    if (argc < 2) {
        eshell_putstring("%s param err! Usage:unhold [1 or 2]", __func__);
        return ;
    }
    tmtc_volte_unhold(argv[2]);
}

static void do_volte_unreg(int argc, char *argv[])
{
    tmtc_volte_unreg();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "tmtc", "voip init, no arg", do_voip_init);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "reg", "voip resiter to registar,no arg", do_voip_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "call", "call xxx", do_voip_call);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "tmtclte", "volte init,no arg", do_volte_init);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "ereg", "volte resiter to lte registar", do_volte_reg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "answer", "answer an incoming call", do_volte_answer);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "term", "terminate the call", do_volte_term);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "setsmsreport", "set sms report", do_volte_setsmsreport);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "sendmsg", "send a message", do_volte_sendmsg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "dtmf", "send a dtmf ", do_volte_senddtmf);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "hold", "hold a call", do_volte_hold);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "unhold", "unhold a held call", do_volte_unhold);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "unreg", "de-resiter to registar", do_volte_unreg);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_IMS, "debugtask", "show all task info", do_debugtask);

 #endif /* end UTILS_ESHELL_EN*/
