// if input value is SKETCH_ESCAPE, clear the register instead
#define SKETCH_ESCAPE 2

#define CM_Sketch(CTRBIT, CTRMAX, REGBIT)                               \
    control CM_Sketch_##CTRBIT##_##REGBIT##(                            \
        in bit<##REGBIT##> index,                                       \
        in bit<##CTRBIT##> value,                                       \
        out bit<##CTRBIT##> res)(                                       \
        bit<##REGBIT##> reg_siz) {                                      \
    Register<bit<##CTRBIT##>, bit<##REGBIT##> >(reg_siz) cm_table;      \
    RegisterAction<bit<##CTRBIT##>, bit<##REGBIT##>, bit<##CTRBIT##>>(cm_table) cm_action = {                                                       \
        void apply(inout bit<##CTRBIT##> reg_data, out bit<##CTRBIT##> result) {                                                               \
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

#define FCM_Sketch(CTRBIT, CTRMAX, REGBIT)                              \
    control FCM_Sketch_##CTRBIT##_##REGBIT##(                           \
        in bit<##REGBIT##> index,                                       \
        in bool do_clear,                                               \
        in bit<##CTRBIT##> value,                                       \
        out bit<##CTRBIT##> res)(                                       \
        bit<##REGBIT##> reg_siz) {                                      \
    Register<bit<##CTRBIT##>, bit<##REGBIT##> >(reg_siz) fcm_table;     \
    RegisterAction<bit<##CTRBIT##>, bit<##REGBIT##>, bit<##CTRBIT##> >(fcm_table) fcm_action = {                                                       \
        void apply(inout bit<##CTRBIT##> reg_data, out bit<##CTRBIT##> result) {                                                               \
            result = reg_data;                                          \
            if (do_clear) {                                             \
                if (value == CTRMAX) {                                  \
                    reg_data = reg_data |+| 1;                          \
                }                                                       \
            }                                                           \
            else {                                                      \
                reg_data = 0;                                           \
            }                                                           \
        }                                                               \
    };                                                                  \
    apply {                                                             \
        res = fcm_action.execute(index);                                \
    }                                                                   \
}

#define CU_Sketch(CTRBIT, REGBIT)         	                            \
    control CU_Sketch_##CTRBIT##_##REGBIT##(       						\
        in bit<##REGBIT##> index,                                       \
        in bit<##CTRBIT##> val,                                         \
        out bit<##CTRBIT##> est)(                                       \
        bit<##REGBIT##> reg_siz) {                                      \
    Register<bit<##CTRBIT##>, bit<##REGBIT##> >(reg_siz) cu_table;      \
    RegisterAction<bit<##CTRBIT##>, bit<##REGBIT##>, bit<##CTRBIT##>>(cu_table) cu_action = {                                                       \
        void apply(inout bit<##CTRBIT##> reg_data, out bit<##CTRBIT##> result) {                                                               \
            if (reg_data <= val) {                                      \
                reg_data = reg_data + 1;                                \
                result = reg_data;                                      \
            }                                                           \
            else {                                                      \
                result = val;                                           \
            }                                                           \
        }                                                               \
    };                                                                  \
    apply {                                                             \
        est = cu_action.execute(index);                                 \
    }                                                                   \
}

#define BloomFilter(REGBIT)                                             \
    control BloomFilter_##REGBIT##(                                     \
        in bit<##REGBIT##> index,                                       \
        in bool val,                                                    \
        out bit<1> res)(                                                \
        bit<##REGBIT##> reg_siz) {                                      \
    Register<bit<1>, bit<##REGBIT##> >(reg_siz) bitmap;                 \
    RegisterAction<bit<1>, bit<##REGBIT##>, bit<1> >(bitmap) set1_action = { \
        void apply(inout bit<1> reg_data, out bit<1> result) {          \
            result = reg_data;                                          \
            reg_data = 1;                                               \
        }                                                               \
    };                                                                  \
    RegisterAction<bit<1>, bit<##REGBIT##>, bit<1> >(bitmap) set0_action = { \
        void apply(inout bit<1> reg_data, out bit<1> result) {          \
            result = reg_data;                                          \
            reg_data = 0;                                               \
        }                                                               \
    };                                                                  \
    apply {                                                             \
        if (val) {                                                      \
            res = set1_action.execute(index);                           \
        }                                                               \
        else {                                                          \
            res = set0_action.execute(index);                           \
        }                                                               \
    }                                                                   \
}
