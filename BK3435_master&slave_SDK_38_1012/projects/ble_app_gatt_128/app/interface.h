/**
 ****************************************************************************************
 *
 * @file interface.h
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef INTERFACE_H_
#define INTERFACE_H_
#include <stddef.h>
#include <stdint.h>
void interface_data_pro(uint8_t *buf,uint8_t len);
#define SCAN_OPEN                   "CMD+SCAN+OPEN"
#define SCAN_CLOSE                "CMD+SCAN+CLOSE"
#define CON_LIST              "CMD+CON+LIST+"  // CMD+CON+LIST+01/02
#define CON_ADDR              "CMD+CON+ADDR+" //   addr_type + ADDR LSB->MSB hex
#define DIS_CON                 "CMD+DISCON"
#define PRF_WR_DATA             "CMD+PRF+WRD+" //CMD+PRF+WRD+uuid16+len+data" // uuid 16
#define PRF_WR_CFG              "CMD+PRF+WRC+" //CMD+PRF+WRC+uuid16+len+data" // uuid 16
#define PRF_RD_DATA             "CMD+PRF+RDD+" //CMD+PRF+RDD+uuid16" // uuid 16
#define PRF_RD_CFG              "CMD+PRF+RDC+" //CMD+PRF+RDD+uuid16" // uuid 16
#define PRF_RD_DESC             "CMD+PRF+RDS+" //CMD+PRF+RDS+uuid16" // uuid 16
#endif //
