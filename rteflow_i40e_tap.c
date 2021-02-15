
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


{
        struct rte_flow_action actions[10];
struct rte_flow_item_eth eth;
struct rte_flow_item_eth eth_mask;
struct rte_flow_item_vlan vlan;
struct rte_flow_item_vlan vlan_mask;
struct rte_flow_item_ipv4 ipv4;
struct rte_flow_item_ipv4 ipv4_mask;
struct rte_flow *flow;
struct rte_flow_action_mark mark = { .id = 0xdeadbeef };
struct rte_flow_action_queue queue = { .index = 0x3 };

memset(&pattern, 0, sizeof(pattern));
memset(&actions, 0, sizeof(actions));
memset(&attr, 0, sizeof(attr));
attr.group = 0;
attr.priority = 0;
attr.ingress = 1;
attr.egress = 0;

memset(&eth_mask, 0, sizeof(struct rte_flow_item_eth));
pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;
pattern[0].spec = &eth;
pattern[0].last = NULL;
pattern[0].mask = NULL;

memset(&vlan_mask, 0, sizeof(struct rte_flow_item_vlan));
pattern[1].type = RTE_FLOW_ITEM_TYPE_VLAN;
pattern[1].spec = &vlan;
pattern[1].last = NULL;
pattern[1].mask = NULL;


/* set the dst ipv4 packet to the required value */
pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
pattern[1].spec = NULL;
pattern[1].last = NULL;
pattern[1].mask = NULL;

pattern[2].type = RTE_FLOW_ITEM_TYPE_UDP;
pattern[2].spec = NULL;
pattern[2].last = NULL;
pattern[2].mask = NULL;

/* end the pattern array */
pattern[3].type = RTE_FLOW_ITEM_TYPE_END;

/* create the drop action */
actions[0].type = RTE_FLOW_ACTION_TYPE_MARK;
actions[0].conf = &mark;
actions[1].type = RTE_FLOW_ACTION_TYPE_END;
}
