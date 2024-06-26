import os
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node

class HwAccelNode(node.NodeConfig):
    def __init__(self):
        super().__init__()
        self.drivers = []
        self.memory = 2048
        self.kcmd_append = ' memmap=512M!1G'

    def prepare_pre_cp(self):
        l = [
            'mount -t proc proc /proc',
            'mount -t sysfs sysfs /sys',
            'echo 1 > /sys/module/vfio/parameters/enable_unsafe_noiommu_mode',
            'echo 9876 1234 >/sys/bus/pci/drivers/vfio-pci/new_id']
        return super().prepare_pre_cp() + l


# Actual application to run. Includes the command to run, but also makes sure
# the compiled binary gets copied into the disk image of the simulated machine.
class MatMulApp(node.AppConfig):
    def __init__(self, n = None):
        super().__init__()
        self.n = n

    def run_cmds(self, node):
        if self.n is None:
          return [f'/tmp/guest/matmul-accel']
        else:
          return [f'/tmp/guest/matmul-accel {self.n}']

    def config_files(self):
        # copy binary into host image during prep
        m = {'matmul-accel': open('app/matmul-accel', 'rb'),
             'echo.eqasm': open('resource/echo.eqasm', 'rb')}
        return {**m, **super().config_files()}


# Simulator component for our accelerator model
class HWAccelSim(sim.PCIDevSim):
    sync = True

    def __init__(self, sim, clock_period):
        super().__init__()
        self.sim = sim
        self.clock_period = clock_period

    def run_cmd(self, env):
        cmd = '%s%s %d %s %s' % \
            (os.getcwd(), f'/accel-{self.sim}/sim', self.clock_period,
             env.dev_pci_path(self), env.dev_shm_path(self))
        return cmd

# Simulator component for CACTUS
class HWCACTUSSim(sim.PCIDevSim):
    sync = True

    def __init__(self, sim, clock_period):
        super().__init__()
        self.sim = sim
        self.clock_period = clock_period

    def run_cmd(self, env):
        cmd = '%s%s %d %s %s' % \
            (os.getcwd(), f'/../CACTUS/build/bin/cactus', self.clock_period,
             env.dev_pci_path(self), env.dev_shm_path(self))
        return cmd

