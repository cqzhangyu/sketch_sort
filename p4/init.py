# 10.0.0.1~9 10.0.0.20~21
port = [24, 16, 32, 40, 4, 0, 8, 20, 28, 44, 60]
MAC = [0xb8599f1d04f2, 0xb8599f0b3072, 0x98039b034650, 0xb8599f020d14, \
       0xb8599fb02d50, 0xb8599fb02bb0, 0xb8599fb02bb8, 0xb8599fb02d18, \
       0xb8599fb02d58, 0x0c42a17ab668, 0x0c42a17aca28]
for worker in range(len(port)):
    bfrt.switch_sort.pipe.Ingress.l2_forward_table.add_with_l2_forward(MAC[worker], port[worker])

bfrt.switch_sort.pipe.Egress.dcqcn.wred.add(0, 0, 125, 2500, 0.01)
# DCQCN
# 0 ~ 10KB, 0
# 10 ~ 200KB, 0 ~ 0.01
# 200KB ~, 1

for p in port:
    bfrt.port.port.add(p, 'BF_SPEED_100G', 'BF_FEC_TYP_NONE', 4, True, 'PM_AN_FORCE_DISABLE')