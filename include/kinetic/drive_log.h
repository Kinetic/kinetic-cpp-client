/*
 * kinetic-cpp-client
 * Copyright (C) 2014 Seagate Technology.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
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
