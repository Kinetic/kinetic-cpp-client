/**
 * Copyright 2013-2015 Seagate Technology LLC.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at
 * https://mozilla.org/MP:/2.0/.
 * 
 * This program is distributed in the hope that it will be useful,
 * but is provided AS-IS, WITHOUT ANY WARRANTY; including without
 * the implied warranty of MERCHANTABILITY, NON-INFRINGEMENT or
 * FITNESS FOR A PARTICULAR PURPOSE. See the Mozilla Public
 * License for more details.
 *
 * See www.openkinetic.org for more project information
 */

#ifndef KINETIC_CPP_CLIENT_DRIVE_LOG_H_
#define KINETIC_CPP_CLIENT_DRIVE_LOG_H_

#include <list>

namespace kinetic {

typedef struct {
    uint64_t nominal_capacity_in_bytes;
    float portion_full;
} Capacity;

typedef struct {
    std::string name;
    uint64_t count;
    uint64_t bytes;
} OperationStatistic;

typedef struct {
    std::string name;
    float percent;
} Utilization;

typedef struct {
    std::string name;
    float current_degc;
    float min_degc;
    float max_degc;
    float target_degc;
} Temperature;

typedef struct {
    std::string vendor;
    std::string model;
    std::string serial_number;
    std::string version;
    int port;
    int tls_port;
    std::string compilation_date;
    std::string source_hash;
} Configuration;

typedef struct {
    uint32_t max_key_size;
    uint32_t max_value_size;
    uint32_t max_version_size;
    uint32_t max_tag_size;
    uint32_t max_connections;
    uint32_t max_outstanding_read_requests;
    uint32_t max_outstanding_write_requests;
    uint32_t max_message_size;
} Limits;


typedef struct {
    Configuration configuration;
    Capacity capacity;
    std::list<OperationStatistic> operation_statistics;
    std::list<Utilization> utilizations;
    std::list<Temperature> temperatures;
    Limits limits;
    std::string messages;
} DriveLog;


} // namespace kinetic

#endif  // KINETIC_CPP_CLIENT_DRIVE_LOG_H_
