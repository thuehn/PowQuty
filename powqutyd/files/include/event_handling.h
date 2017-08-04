/*
 * event_handling.h
 *
 *  Created on: Jun 27, 2017
 *  	Author: ikstream
 */
#ifndef _EVENT_HANDLING_H_
#define _EVENT_HANDLING_H_

#include "PQ_App.h"
#include "uci_config.h"

#define SECONDS_IN_WEEK (60 * 60 * 24 * 7)

/*
 * use this function on en50160 event occurrence
 * @param pqResult: power quality result struct with event information
 * @param conf: powquty configuration
 * @param event: struct for individual event tim logging
 */
void handle_event(PQResult pqResult, struct powquty_conf *conf);

#endif /* _EVENT_HANDLING_H_ */
