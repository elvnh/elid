#pragma once

struct scan_RegionMatches {
    usize base_address;
    usize region_size;
    scan_MatchList *head;
    scan_RegionMatches *next;
};

struct scan_Result {
    // TODO: store type and mode?
    // TODO: total match count?
    proc_Status         status;
    scan_RegionMatches *first;
    scan_RegionMatches *last;
};
