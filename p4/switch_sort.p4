/* -*- P4_16 -*- */
#include <core.p4>
#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "count.p4"

typedef bit<9>  egress_spec_t;
typedef bit<48> mac_addr_t;
typedef bit<32> ip4_addr_t;
typedef bit<16> port_t;
typedef bit<16> checksum_t;

const bit<8>  TCP_PROTOCOL = 0x06;
const bit<8>  UDP_PROTOCOL = 0x11;

const bit<16> TYPE_IPV4 = 0x800;

/*************************************************************************
*********************** H E A D E R S  ***********************************
*************************************************************************/

header ethernet_t {
    mac_addr_t dst_addr;
    mac_addr_t src_addr;
    bit<16> ether_type;
}

header ip_t {
    bit<8> ver_hl; // 4 + 4
    bit<8> dscp_ecn; // tos, 6 + 2
    bit<16> length;
    bit<16> id;
    bit<16> flag_offset; // flag and offset of fragment, 3 + 13
    bit<8> ttl;
    bit<8> protocol;
    bit<16> checksum;
    bit<32> sip;
    bit<32> dip;
}

header udp_t{
    port_t src_port;
    port_t dst_port;
    bit<16> length;
    checksum_t checksum;
}

header keys_t {
    bit<32> key0;
    bit<32> key1;
    bit<32> key2;
    bit<32> key3;
    bit<32> key4;
    // bit<32> key5;
    // bit<32> key6;
    // bit<32> key7;
    // bit<32> key8;
    // bit<32> key9;
}

header res_t {
    bit<32> val0;
    bit<32> val1;
    bit<32> val2;
    bit<32> val3;
    bit<32> val4;
    // bit<32> val5;
    // bit<32> val6;
    // bit<32> val7;
    // bit<32> val8;
    // bit<32> val9;
}

struct headers {
    ethernet_t ethernet;
    ip_t ip;
    udp_t udp;
    keys_t keys;
    res_t res;
}

struct port_metadata_t {
    bit<16> unused;
}

struct metadata {
    port_metadata_t port_metadata;
    bit<32> inc;
}

/*************************************************************************
*********************** P A R S E R  ***********************************
*************************************************************************/

parser IngressParser(packet_in packet,
               out headers hdr,
               out metadata meta,
               out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        packet.extract(ig_intr_md);
        transition select(ig_intr_md.resubmit_flag) {
            0 : parse_port_metadata;
        }
    }

    state parse_port_metadata {
        meta.port_metadata = port_metadata_unpack<port_metadata_t>(packet);
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            TYPE_IPV4: parse_ip;
            default: accept;
        }
    }

    state parse_ip {
        packet.extract(hdr.ip);
        transition select(hdr.ip.dscp_ecn) {
            8w0x04 &&& 8w0xfc: parse_switch_sort;
            default: accept;
        }
    }

    state parse_switch_sort{
        packet.extract(hdr.udp);
        packet.extract(hdr.keys);
        packet.extract(hdr.res);
        transition accept;
    }
}


/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

define_CM_Sketch(32, 32)   
define_Hash_Sketch(32, 32, 32)
define_multiway_sketch(32, 32, 32)
define_count(32, 32, 32)

control Ingress(
        inout headers hdr,
        inout metadata meta,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_prsr_md,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
        inout ingress_intrinsic_metadata_for_tm_t ig_intr_tm_md) {

    action drop() {
        ig_intr_dprs_md.drop_ctl = 0x1;
    }

    action l2_forward(bit<9> port) {
        ig_intr_tm_md.ucast_egress_port = port;
    }

    table l2_forward_table{
        key = {
            hdr.ethernet.dst_addr: exact;
        }
        actions = {
            l2_forward;
            drop;
        }
        size = 32;
        default_action = drop();
    }

    Count_32_32_32(65536) count;// WARNING: user should make sure a key ONLY exist in the SAME position of hdr.keys, e.g. hash based addressing

    apply {
        l2_forward_table.apply();
        if(hdr.ip.dscp_ecn[7:2] == 1) {
            //meta.inc = 1;
            //count.apply(hdr.keys, meta.inc, hdr.res);
            count.apply(hdr.keys, 1, hdr.res);
        } 
    }
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control IngressDeparser(
        packet_out packet,
        inout headers hdr,
        in metadata meta,
        in ingress_intrinsic_metadata_for_deparser_t ig_intr_dprsr_md) {

    apply{
        packet.emit(hdr);
    }
}

parser EgressParser(packet_in packet,
               out headers hdr,
               out metadata meta,
               out egress_intrinsic_metadata_t eg_intr_md) {
    state start {
        packet.extract(eg_intr_md);
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            0x0800: parse_ip;
            default: accept;
        }
    }

    state parse_ip{
        packet.extract(hdr.ip);
        transition accept;
    }
}

control dcqcn(
    inout headers hdr,
    in egress_intrinsic_metadata_t eg_intr_md) {

    Wred<bit<19>, bit<32>>(32w1, 8w1, 8w0) wred;
    apply {
        if(hdr.ip.isValid()) {
            if(hdr.ip.dscp_ecn[1:0] == 0) { // Using "!=" and "&&" sometimes causes BUG
            }
            else {
                bit<8> drop_flag = wred.execute(eg_intr_md.deq_qdepth, 0);
                if(drop_flag == 1) hdr.ip.dscp_ecn[1:0] = 3;
            }
        }
    }
}

control Egress(
        inout headers hdr,
        inout metadata meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_intr_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_intr_oport_md) {
    apply {
        dcqcn.apply(hdr, eg_intr_md);
    }
}

control EgressChecksum(inout headers hdr) {
    Checksum() csum;
    apply{
        hdr.ip.checksum = csum.update({
            hdr.ip.ver_hl,
            hdr.ip.dscp_ecn,
            hdr.ip.length,
            hdr.ip.id,
            hdr.ip.flag_offset,
            hdr.ip.ttl,
            hdr.ip.protocol,
            hdr.ip.sip,
            hdr.ip.dip
        });
    }
}

control EgressDeparser(packet_out packet,
                  inout headers hdr,
                  in metadata meta,
                  in egress_intrinsic_metadata_for_deparser_t ig_intr_dprs_md) {

    apply {
        EgressChecksum.apply(hdr);
        packet.emit(hdr);
    }
}

Pipeline(IngressParser(), Ingress(), IngressDeparser(), EgressParser(), Egress(), EgressDeparser()) pipe;

Switch(pipe) main;