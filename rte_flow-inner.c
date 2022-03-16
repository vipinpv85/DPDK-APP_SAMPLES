			const int MAX_PATTERN_IN_FLOW = 10;
			const int MAX_ACTIONS_IN_FLOW = 10;
			
			struct rte_flow_attr attr;// = {.ingress = 1};
			
			struct rte_flow_item pattern[MAX_PATTERN_IN_FLOW];
			struct rte_flow_action actions[MAX_ACTIONS_IN_FLOW];
			struct rte_flow_item_eth eth,eth2;
			//struct rte_flow_item_vlan vlan;
			struct rte_flow_item_ipv4 ipv4,ipv4_2;
			struct rte_flow_item_gre gre;
			struct rte_flow_item_udp udp;
			//struct rte_flow_item_ipv4 ipv4_mask;
			struct rte_flow *flow;
			struct rte_flow_error error;

			memset(&eth, 0, sizeof(eth));
			memset(&eth2, 0 ,sizeof(eth2));
            memset(&ipv4, 0 ,sizeof(ipv4));
            memset(&ipv4_2, 0, sizeof(ipv4_2));
            memset(&gre, 0, sizeof(gre));
            memset(&udp, 0, sizeof(udp));



			memset(pattern, 0, sizeof(pattern));
			memset(actions, 0, sizeof(actions));

			memset(&attr, 0, sizeof(struct rte_flow_attr));
			attr.ingress = 1;

			// setting the eth to pass all packets 
			pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;
			pattern[0].spec = &eth;

			// set the vlan to pass all packets 
			pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
			pattern[1].spec = &ipv4;

			pattern[2].type = RTE_FLOW_ITEM_TYPE_GRE;
			pattern[2].spec = &gre;

			pattern[3].type = RTE_FLOW_ITEM_TYPE_ETH;
			pattern[3].spec = &eth2;

			// set the vlan to pass all packets 
			pattern[4].type = RTE_FLOW_ITEM_TYPE_IPV4;
			pattern[4].spec = &ipv4_2;

			// set the vlan to pass all packets 
			pattern[5].type = RTE_FLOW_ITEM_TYPE_UDP;
			pattern[5].spec = &udp;


			// end the pattern array 
			pattern[6].type = RTE_FLOW_ITEM_TYPE_END;

			struct rte_flow_action_rss rss_conf;
			uint16_t queues[pi_nNumRxQueues];
			rss_conf.func = RTE_ETH_HASH_FUNCTION_DEFAULT;
			rss_conf.types = ETH_RSS_IP | ETH_RSS_UDP | ETH_RSS_TCP;
			rss_conf.queue_num = pi_nNumRxQueues;
			for (int nqQueueIndex= 0; nqQueueIndex < pi_nNumRxQueues; nqQueueIndex++)
		         queues[nqQueueIndex] = nqQueueIndex;

			rss_conf.queue = queues;
			
			rss_conf.key_len = 0;
			rss_conf.key = NULL;
			rss_conf.level = 2;

			// create the drop action 
			actions[0].type = RTE_FLOW_ACTION_TYPE_RSS;
			actions[0].conf = &rss_conf;
			
			actions[0].type = NULL;
			actions[0].conf = NULL;
			actions[1].type = RTE_FLOW_ACTION_TYPE_END;
	
		
			// validate and create the flow rule 
			if (rte_flow_validate(pi_nPort, &attr, pattern, actions, &error)==0)
			{
				printf("Succeded to validate flow");

				flow = rte_flow_create(pi_nPort, &attr, pattern, actions, &error);
				if(!flow)
					printf("Failed to create flow %s ", error.message);
			}
