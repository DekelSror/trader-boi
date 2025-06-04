from . import Timeseries


class MemoryTimeseries(Timeseries):
    def __init__(self):
        self.points = []

    def write_points(self, points) -> bool:
        if not isinstance(points, list):
            points = [points]
        self.points.extend(points)
        return True
    
    def range(self, start_time: int, end_time: int):
        going = False
        for p in self.points:
            if not going:
                if p[0] >= start_time:
                    going = True
                else:
                    continue
            elif p[0] <= end_time:
                yield p
            else:
                going = False
                break

    def close(self):
        pass