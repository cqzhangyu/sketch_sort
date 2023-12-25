#define SKETCH_ESCAPE 2

#define define_CM_Sketch(CTRBIT, REGBIT)                                \
control CM_Sketch_##CTRBIT##_##REGBIT##(                                \
    in bit<##REGBIT##> index,                                           \
    in bit<##CTRBIT##> value,                                           \
    out bit<##CTRBIT##> res)                                            \
    (bit<##REGBIT##> reg_siz)                                           \
{                                                                       \
    Register<bit<##CTRBIT##>, bit<##REGBIT##> >(reg_siz) cm_table;      \
    RegisterAction<bit<##CTRBIT##>, bit<##REGBIT##>, bit<##CTRBIT##>>(cm_table) cm_action = {    \
        void apply(inout bit<##CTRBIT##> reg_data, out bit<##CTRBIT##> result) {                 \
            if (value != SKETCH_ESCAPE) {                               \
                reg_data = reg_data |+| value;                          \
                result = reg_data;                                      \
            }                                                           \
            else {                                                      \
                result = reg_data;                                      \
                reg_data = 0;                                           \
            }                                                           \
        }                                                               \
    };                                                                  \
    apply {                                                             \
        res = cm_action.execute(index);                                 \
    }                                                                   \
}

#define define_Hash_Sketch(KEYBIT, CTRBIT, REGBIT)                \
control Hash_Sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(          \
    in bit<##KEYBIT##> key,                                             \
    in bit<##CTRBIT##> value,                                           \
    out bit<##CTRBIT##> res)                                            \
    (bit<##REGBIT##> reg_siz, bit<32>POLY)                                           \
{                                                                       \
    CRCPolynomial<bit<32>>(POLY, false, false, false, 32w0, 32w0) polynomial;\
    Hash<bit<REGBIT>>(HashAlgorithm_t.CUSTOM, polynomial) crchash;      \
    CM_Sketch_##CTRBIT##_##REGBIT##(reg_siz) cm;                         \
    apply {                                                             \
        bit<REGBIT>index = crchash.get({key});                          \
        cm.apply(index, value, res);                                    \
    }                                                                   \
}

#define define_multiway_sketch(KEYBIT, CTRBIT, REGBIT)              \
                                                                    \
control multiway_sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(           \
    in bit<##KEYBIT##> key,                                         \
    in bit<##CTRBIT##> value,                                       \
    out bit<##CTRBIT##> res)                                        \
    (bit<##REGBIT##> reg_siz)                                       \
{                                                                   \
    Hash_Sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz, 0x04C11DB7) hash_sketch0;     \
    Hash_Sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz, 0x1EDC6F41) hash_sketch1;     \
    apply{                                                                              \
        bit<##CTRBIT##> res0;                                                           \
        bit<##CTRBIT##> res1;                                                           \
        hash_sketch0.apply(key, value, res0);                                           \
        hash_sketch1.apply(key, value, res1);                                           \
        res = min(res0, res1);                                                          \
    }                                                                                   \
}

// WARNING: user should make sure a key ONLY exist in the SAME position of hdr.keys, e.g. hash based addressing
#define define_count(KEYBIT, CTRBIT, REGBIT)    \
                                                \
control Count_##KEYBIT##_##CTRBIT##_##REGBIT##( \
        in keys_t keys,                         \
        in bit<##CTRBIT##> value,               \
        out res_t res)                          \
        (bit<##REGBIT##> reg_siz)               \
{                                               \
    multiway_sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz) s0;   \
    multiway_sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz) s1;   \
    multiway_sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz) s2;   \
    multiway_sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz) s3;   \
    multiway_sketch_##KEYBIT##_##CTRBIT##_##REGBIT##(reg_siz) s4;   \
    apply{                                                          \
        s0.apply(keys.key0, value, res.val0);                      \
        s1.apply(keys.key1, value, res.val1);                      \
        s2.apply(keys.key2, value, res.val2);                      \
        s3.apply(keys.key3, value, res.val3);                      \
        s4.apply(keys.key4, value, res.val4);                      \
    }                                                       \
}
