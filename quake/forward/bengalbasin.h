/*
 * bengalbasin.h
 *
 *  Created on: Mar 27, 2016
 *      Author: monsurul huda
 */

#ifndef BENGALBASIN_H_
#define BENGALBASIN_H_

#include "cvm.h"

int bengal_cvm_query(double east_m, double north_m, double depth_m, cvmpayload_t* result, double ** surfaces);

#endif /* BENGALBASIN_H_ */
