import os
import mmap
import struct
from . import Timeseries

ENTRY_SIZE = 16  # 8 bytes for int64, 8 bytes for float64
DEFAULT_MAX_ENTRIES = 100000  # Adjust as needed

class ShmapTimeseries(Timeseries):
    def __init__(self, filename="/tmp/traderboi_shmmap", max_entries=DEFAULT_MAX_ENTRIES):
        self.filename = filename
        self.max_entries = max_entries
        self.size = ENTRY_SIZE * max_entries
        self._ensure_file()
        self.file = open(self.filename, "r+b")
        self.mm = mmap.mmap(self.file.fileno(), self.size)
        self.file.close()
        self.count = 0  # Number of points written

    def _ensure_file(self):
        if not os.path.exists(self.filename):
            with open(self.filename, "wb") as f:
                f.truncate(self.size)

    def write_points(self, points):
        if not isinstance(points, list):
            points = [points]
        for ts, val in points:
            if self.count >= self.max_entries:
                # self.mm.resize()
                raise RuntimeError("Shared memory full")
            offset = self.count * ENTRY_SIZE
            self.mm[offset:offset+ENTRY_SIZE] = struct.pack("q d", ts, val)
            self.count += 1
        return True

    def range(self, start_time: int, end_time: int):
        for i in range(self.count):
            offset = i * ENTRY_SIZE
            ts, val = struct.unpack("q d", self.mm[offset:offset+ENTRY_SIZE])
            if ts < start_time:
                continue
            if ts > end_time:
                break
            yield (ts, val)

    def close(self):
        self.mm.close()
        self.file.close()
