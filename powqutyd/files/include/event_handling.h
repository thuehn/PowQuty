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

void handle_event(PQResult pqResult, struct powquty_conf *conf);

#endif /* _EVENT_HANDLING_H_ */
