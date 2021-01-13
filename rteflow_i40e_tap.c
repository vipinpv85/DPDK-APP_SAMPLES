
{
        struct rte_flow_attr attr = { .ingress = 1 };
        struct rte_flow_item pattern[10];
        struct rte_flow_action actions[10];
        struct rte_flow_action_queue actionqueue = { .index = 1 };
        struct rte_flow_item_eth eth;
        struct rte_flow_item_vlan vlan;
        struct rte_flow_item_ipv4 ipv4;
        struct rte_flow *flow;
        struct rte_flow_error error;

        /* setting the eth to pass all packets */
        pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;
        pattern[0].spec = &eth;
        pattern[0].last = NULL;
        pattern[0].mask = NULL;

        /* set the vlan to pass all packets */
        pattern[1].type = RTE_FLOW_ITEM_TYPE_VLAN;
        pattern[1].spec = &vlan;
        pattern[1].last = NULL;
        pattern[1].mask = NULL;

        /* set the dst ipv4 packet to the required value */
        ipv4.hdr.dst_addr = htonl(0x0E000001);
        pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
        pattern[1].spec = &ipv4;
        pattern[1].last = NULL;
        pattern[1].mask = NULL;

        /* end the pattern array */
        pattern[2].type = RTE_FLOW_ITEM_TYPE_END;

        /* create the drop action */
        actions[0].type = RTE_FLOW_ACTION_TYPE_DROP;
        actions[0].type = RTE_FLOW_ACTION_TYPE_DROP;
        actions[0].type = RTE_FLOW_ACTION_TYPE_VOID;
        actions[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
        actions[0].conf = &actionqueue;
        actions[1].type = RTE_FLOW_ACTION_TYPE_END;

        /* validate and create the flow rule */
        if (!rte_flow_validate(port, &attr, pattern, actions, &error))
                flow = rte_flow_create(port, &attr, pattern, actions, &error);
        else
                printf("rte_flow err %s\n", error.message);
}
