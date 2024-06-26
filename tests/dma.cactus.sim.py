# TEST 0: this is a functional test for the driver. It just reads and prints the
# supported matrix size from the accelerator. Runs two separate configurations
# with different accelerator sizes.
import sys; sys.path.append('./tests/')
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []

e = exp.Experiment(f'dma')

server_config = HwAccelNode()
server_config.app = MatMulApp(8)
server_config.nockp = True
server_config.disk_image = '/root/pathfinder/simbrick/simbricks-examples/custom-image/output-memcached/memcached'

server = sim.Gem5Host(server_config)
server.name = 'host'
server.cpu_type = 'X86KvmCPU'

hwaccel = HWCACTUSSim('sim', 100000)
hwaccel.name = 'accel'
hwaccel.sync = False
server.add_pcidev(hwaccel)

e.add_pcidev(hwaccel)
e.add_host(server)
server.wait = True

experiments.append(e)

