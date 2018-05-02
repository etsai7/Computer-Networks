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
header_type keyvalue_head_t {
    fields {
        preamble: 64; /* In bits */
        num_valid: 32;
        port: 8;
        mtype: 8;  /* can be 0,1,2,3 */
        key: 32;
        value: 32;
    }
}

header keyvalue_head_t keyvalue_head;

/* Parsers */
parser start {
    return select(current(0, 64)) {
        1: parse_type;
        default: ingress;
    }
}

parser parse_type {
    extract(keyvalue_head);
    return ingress;
}

/*  Register */
register my_register {
    width: 32;
    static: kv_table;
    instance_count: 16384;
}

/* Actions */
action _drop() {
    drop();
}

action put(){
    register_write(my_register, keyvalue_head.key, keyvalue_head.value);
    modify_field(keyvalue_head.mtype, 3);
    modify_field(standard_metadata.egress_spec, standard_metadata.ingress_port);
}

action get(){
    register_read(keyvalue_head.value , my_register, keyvalue_head.key);
    modify_field(keyvalue_head.mtype, 2);
    modify_field(standard_metadata.egress_spec, standard_metadata.ingress_port);
}

/* Table */
table kv_table {
    reads {
        keyvalue_head.mtype: exact; 
    }
    actions {
        put;
        get;
    }
    size : 16384;
}

control ingress {
    apply(kv_table);
}

control egress {
}