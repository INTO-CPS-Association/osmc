import pickle
from fmi2 import Fmi2FMU, Fmi2Status


class Model(Fmi2FMU):
    def __init__(self, reference_to_attr=None) -> None:
        super().__init__(reference_to_attr)
        self.xpos = 0.0
        self.ypos = 0.0

        self._update_outputs()

    def serialize(self):

        bytes = pickle.dumps(
            (
                self.xpos,
                self.ypos,
            )
        )
        return Fmi2Status.ok, bytes

    def deserialize(self, bytes) -> int:
        (
            xpos,
            ypos,
        ) = pickle.loads(bytes)
        self.xpos = xpos
        self.ypos = ypos
        self._update_outputs()

        return Fmi2Status.ok

    def _update_outputs(self):
        #anomaly detector logic will be added here
        self.real_c = 2.0

    def do_step(self, current_time, step_size, no_step_prior):

        self._update_outputs()

        return Fmi2Status.ok

