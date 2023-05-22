/* Get id-name lookup table */
xstats_names = malloc(sizeof(struct rte_eth_xstat_name) * cnt_xstats);
if (xstats_names == NULL) {
        printf("Cannot allocate memory for xstats lookup\n");
        return;
}
if (cnt_xstats != rte_eth_xstats_get_names(
                port_id, xstats_names, cnt_xstats)) {
        printf("Error: Cannot get xstats lookup\n");
        free(xstats_names);
        return;
}

/* Get stats themselves */
xstats = malloc(sizeof(struct rte_eth_xstat) * cnt_xstats);
if (xstats == NULL) {
        printf("Cannot allocate memory for xstats\n");
        free(xstats_names);
        return;
}
if (cnt_xstats != rte_eth_xstats_get(port_id, xstats, cnt_xstats)) {
        printf("Error: Unable to get xstats\n");
        free(xstats_names);
        free(xstats);
        return;
}
/* Display xstats */
for (idx_xstat = 0; idx_xstat < cnt_xstats; idx_xstat++) {
        if (xstats_hide_zero && !xstats[idx_xstat].value)
                continue;
        printf("%s: %"PRIu64"\n",
                xstats_names[idx_xstat].name,
                xstats[idx_xstat].value);
}
free(xstats_names);
free(xstats);
