/*
Copyright 2013-present Barefoot Networks, Inc. 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* Headers */
header_type easy_route_header {
    fields {
        preamble : 64;   /* This is in bits*/
        num_valid : 32;
    }
}

header easy_route_header easy_route_head;

header_type easy_route_port_header {
    fields {
        port : 8;
    }
}

header easy_route_port_header easy_route_port_head;

header_type kv_header {
    fields {
        type : 8;
        key : 32;
        value: 32;
    }
}

header kv_header kv_head;

/* Parsers */
parser start {
    return select(current(0, 64)) {
        0: parse_head;
        default: ingress;
    }
    return ingress;
}

parser parse_head {
    extract(easy_route_head);
    return select(latest.num_valid) {
        0: ingress;
        default: parse_port;
    }
}

parser parse_port {
    extract(easy_route_port_head);
    return select(latest.port){
        0: ingress;
        default: parse_type;
    }
}

parser parse_type {
    extract(kv_head);
    return select(latest.kv_head) {
        0: get_request;
        1: put_request;
        2: get_reply;
        3: put_reply;
        default: ingress;
    }
}


action _drop() {
    drop();
}

action route() {
    modify_field(standard_metadata.egress_spec, easy_route_port_head.port);
    add_to_field(easy_route_head.num_valid, -1);
    remove_header(easy_route_port_head);
}

table route_pkt {
    reads {
        easy_route_port_head: valid;
    }
    actions {
        _drop;
        route;
    }
    size: 1;
}

control ingress {
    apply(route_pkt);
}

control egress {
    // leave empty
}
