
from os import stat
from time import sleep
from typing import Any


class FileWatcher:
    def __init__(self, filename: str, callback) -> None:
        self.filename = filename
        self.callback = callback
        self.last_modified = 0.0
        self.done = False
    
    def stop(self):
        self.done = True

    def check(self):
        mod = stat(self.filename).st_mtime
        if mod > self.last_modified:
            self.last_modified = mod
            self.callback()
