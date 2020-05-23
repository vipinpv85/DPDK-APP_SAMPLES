#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>

#include <rte_eal.h>
#include <rte_memzone.h>
#include <rte_malloc.h>
#include <rte_fbarray.h>
#include <rte_cycles.h>

static int
lcore_hello(__attribute__((unused)) void *arg)
{
        unsigned lcore_id = rte_lcore_id();

        struct rte_mp_msg mp_resp;
        strncpy(mp_resp.name, "app-mp", 7);
        mp_resp.num_fds = 0;
        strncpy(mp_resp.param, "Hey, I am here!", 15);
        mp_resp.len_param = 12;

        printf("hello from core %u\n", lcore_id);

        if (rte_eal_process_type() != 0) {
                printf(" secondary sending out msg!!\n");
                if (rte_mp_sendmsg(&mp_resp) < 0)
                        printf("fail\n");
        }

        strncpy(mp_resp.param, "Here is your data!", 21);
        mp_resp.len_param = 20;

        while(1) {
                rte_delay_ms(1000);
                if (rte_eal_process_type() == 0) {
                        if (rte_mp_sendmsg(&mp_resp) < 0)
                                printf("fail\n");
                        fprintf(stdout, "data is ready from priamry!\n");
                }
        }

        return 0;
}

static int
app_mp_action(const struct rte_mp_msg *mp_msg, const void *peer)
{
        struct rte_mp_msg resp_msg;

        fprintf(stdout, " I am %s!\n",
                        (rte_eal_process_type() == 0) ? "primary" : "secondary");
        fprintf(stdout, "mp_msg - name (%s); len_param %d; num_fds %d, param (%s), fds (%s)\n",
                        mp_msg->name, mp_msg->len_param, mp_msg->num_fds, mp_msg->param, mp_msg->fds);

        if (mp_msg->len_param > 2) {
                if (mp_msg->param[0] == 'O' && mp_msg->param[1] == 'k') {

                        resp_msg.num_fds = 0;
                        strncpy(resp_msg.name, "app-mp", sizeof(resp_msg.name));
                        strncpy(resp_msg.param, "Done!", 5);
                        resp_msg.len_param = 5;

                        if (rte_mp_reply(&resp_msg, peer) < 0)
                                rte_panic(" fail to send reply to peer\n");

                        fprintf(stdout, " Reply send to peer!\n");
                        return 1;
                }
                else if (mp_msg->param[0] == 'H' &&
                                mp_msg->param[1] == 'e') {
                        fprintf(stdout, "asdasddsdsdfsf\n");
                        resp_msg.num_fds = 0;
                        strncpy(resp_msg.name, "app-mp", sizeof(resp_msg.name));
                        strncpy(resp_msg.param, "Got Data!", 10);
                        resp_msg.len_param = 9;

                        if (rte_mp_reply(&resp_msg, peer) < 0)
                                rte_panic(" fail to send reply to peer\n");

                        fprintf(stdout, " Reply send to peer!\n");
                        return 2;
                }
        }

        resp_msg.num_fds = 0;
        strncpy(resp_msg.name, "app-mp", sizeof(resp_msg.name));
        strncpy(resp_msg.param, "Ok, I see you!", 15);
        resp_msg.len_param = 14;

        fprintf(stdout, "resp_msg - name (%s); len_param %d; num_fds %d, param (%s), fds (%s)\n",
                        resp_msg.name, resp_msg.len_param, resp_msg.num_fds, resp_msg.param, resp_msg.fds);

        if (rte_mp_sendmsg(&resp_msg) < 0)
                        printf("fail\n");

//      rte_mp_action_unregister("app-mp");

        return 0;
}

int
main(int argc, char **argv)
{
        int ret;
        unsigned lcore_id;

        ret = rte_eal_init(argc, argv);
        if (ret < 0)
                rte_panic("Cannot init EAL\n");

        printf("current lcore-master %u process-type %u\n",
                rte_get_master_lcore(), rte_eal_process_type());

        ret = rte_mp_action_register("app-mp", app_mp_action);
        if (ret < 0) {
                rte_panic("Cannot register action for app-mp from %s\n",
                                (rte_eal_process_type() == 0) ? "primary" : "secondary");
        }
        fprintf(stdout, "registered action for app-mp from %s\n",
                        (rte_eal_process_type() == 0) ? "primary" : "secondary");

        /* call it on master lcore too */
        lcore_hello(NULL);

        rte_eal_mp_wait_lcore();

        return 0;
}
